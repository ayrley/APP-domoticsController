#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <string>
#include <thread>
#include <vector>

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <stdint.h>

#include "backlight.h"
#include "badge.h"
#include "control.h"
#include "debug.h"
#include "eventLog.h"
#include "io.h"
#include "lifeLed.h"
#include "proximity.h"
#include "accessController.h"
#include "screen.h"
#include "signalHandlers.h"
#include "settings.h"
#include "statusLeds.h"
#include "tamperSwitch.h"
#include "serialBus.h"

namespace {
constexpr int SCREENSAVER_BACKLIGHT_BRIGHTNESS = 12;
}

Settings *g_userSettings = nullptr;
Settings *g_factorySettings = nullptr;
StatusLeds *g_statusLeds = nullptr;
std::vector<Badge *> *g_badges = new std::vector<Badge *>();
std::vector<IO *> *g_ios = new std::vector<IO *>();
std::vector<SerialBus *> *g_serialBusses = new std::vector<SerialBus *>();
std::vector<Action *> *g_actions = new std::vector<Action *>();

bool hasNetworkReadersConfigured()
{
    if (g_userSettings == nullptr || g_userSettings->getAccessControllers() == nullptr) {
        return false;
    }

    for (AccessController *accessCtlr : *g_userSettings->getAccessControllers()) {
        if (accessCtlr != nullptr && accessCtlr->getLocationType() == RDR_LOC_IP) {
            return true;
        }
    }

    return false;
}

void clearBadgesCache()
{
    for (Badge *badge : *g_badges) {
        delete badge;
    }
    g_badges->clear();
}

void reloadBadgesCache()
{
    clearBadgesCache();
    loadBadges();
}

int loadSettings()
{
    std::stringstream userSettingsStream;
    std::stringstream factorySettingsStream;
    std::string userSettingsFile;
    std::string factorySettingsFile;

    userSettingsStream << DIR_SHARED << "user.settings.domotics";
    userSettingsStream >> userSettingsFile;

    factorySettingsStream << DIR_ETC << "factory.settings.domotics";
    factorySettingsStream >> factorySettingsFile;

    if (!std::filesystem::exists(factorySettingsFile)) {
        ERR("Factory settings file not found: " + factorySettingsFile);
        return -ENOENT;
    }

    Settings *factory = new Settings(factorySettingsFile);
    if (factory->read() != 0) {
        delete factory;
        ERR("Failed to read factory settings: " + factorySettingsFile);
        return -ENOENT;
    }
    factory->parse();

    bool settingsExist = std::filesystem::exists(userSettingsFile);

    if (!settingsExist) {
        std::error_code ec;
        const auto userSettingsParent = std::filesystem::path(userSettingsFile).parent_path();
        if (!userSettingsParent.empty()) {
            std::filesystem::create_directories(userSettingsParent, ec);
            if (ec) {
                delete factory;
                ERR("Failed to create settings directory: " + ec.message());
                return -ENOENT;
            }
        }

        std::filesystem::copy_file(factorySettingsFile,
                                   userSettingsFile,
                                   std::filesystem::copy_options::overwrite_existing,
                                   ec);
        if (ec) {
            delete factory;
            ERR("Failed to create user settings from factory template: " + ec.message());
            return -ENOENT;
        }
    }

    Settings *user = new Settings(userSettingsFile);
    if (user->read() != 0) {
        delete user;
        delete factory;
        ERR("Failed to read user settings: " + userSettingsFile);
        return -ENOENT;
    }
    user->parse();

    if (user->getType() != SET_USER) {
        user->setType(SET_USER);
        user->write();
    }
    g_userSettings = user;
    g_factorySettings = factory;

    return 0;
}

int loadBadges()
{
    int ret = 0;
    std::stringstream badgesStream;
    std::string badgesDir;

    badgesStream << DIR_SHARED << "badges";
    badgesStream >> badgesDir;

    for (const auto &entry : std::filesystem::directory_iterator(badgesDir)) {
        std::string badgePath = entry.path();
        Badge *badge = new Badge(badgePath);
        g_badges->push_back(badge);
    }

    return ret;
}

int loadIos()
{
    int ret = 0;
    std::stringstream iosStream;
    std::string iosDir;

    iosStream << DIR_SHARED << "ios";
    iosStream >> iosDir;

    for (const auto &entry : std::filesystem::directory_iterator(iosDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        json ioFileObject;
        std::ifstream jsonFile(entry.path());

        if (!jsonFile.is_open()) {
            ERR("Unable to open IO config: " + entry.path().string());
            continue;
        }

        if (jsonFile.peek() == std::ifstream::traits_type::eof()) {
            LOG("Skipping empty IO config: " + entry.path().string());
            continue;
        }

        try {
            ioFileObject = json::parse(jsonFile);
        } catch (const std::exception &e) {
            ERR("Skipping invalid IO config '" + entry.path().string() + "': " + e.what());
            continue;
        }

        if (!ioFileObject.contains("IOs") || !ioFileObject["IOs"].is_array()) {
            LOG("Skipping IO config without array 'IOs': " + entry.path().string());
            continue;
        }

        for (auto &singleIo : ioFileObject["IOs"].items()) {
            IO *io = new IO();
            io->fromJson(singleIo.value());
            g_ios->push_back(io);
        }
    }

    return ret;
}

int loadBusses()
{
    int ret = 0;
    std::stringstream bussesStream;
    std::string bussesDir;

    bussesStream << DIR_SHARED << "ios";
    bussesStream >> bussesDir;

    for (const auto &entry : std::filesystem::directory_iterator(bussesDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        json busFileObject;
        std::ifstream jsonFile(entry.path());

        if (!jsonFile.is_open()) {
            ERR("Unable to open bus config: " + entry.path().string());
            continue;
        }

        if (jsonFile.peek() == std::ifstream::traits_type::eof()) {
            LOG("Skipping empty bus config: " + entry.path().string());
            continue;
        }

        try {
            busFileObject = json::parse(jsonFile);
        } catch (const std::exception &e) {
            ERR("Skipping invalid bus config '" + entry.path().string() + "': " + e.what());
            continue;
        }

        if (!busFileObject.contains("SerialBusses") || !busFileObject["SerialBusses"].is_array()) {
            LOG("Skipping bus config without array 'SerialBusses': " + entry.path().string());
            continue;
        }

        for (auto &singleBus : busFileObject["SerialBusses"].items()) {
            SerialBus *bus = new SerialBus();
            bus->fromJson(singleBus.value());
            g_serialBusses->push_back(bus);
        }
    }

    return ret;
}

int startReaders(StatusLeds &statusLeds)
{
    int ret = 0;

    for (auto singleAccessController : *g_userSettings->getAccessControllers()) {
        singleAccessController->setBadges(g_badges);
        singleAccessController->setErrorHandler([&statusLeds](readerLocationType locationType, const std::string &message) {
            ERR("Reader failure: " << message);
            statusLeds.showError(locationType == RDR_LOC_IP
                                     ? StatusLeds::ERROR_NETWORK
                                     : StatusLeds::ERROR_READER);
        });
        singleAccessController->start();
    }

    return ret;
}

int startActions()
{
    int ret = 0;

    for (auto singleAction : *g_userSettings->getActions()) {
        singleAction->start();
    }

    return ret;
}

bool hasDisplayServer()
{
    const char *display = std::getenv("DISPLAY");
    const char *waylandDisplay = std::getenv("WAYLAND_DISPLAY");

    return (display != nullptr && display[0] != '\0') || (waylandDisplay != nullptr && waylandDisplay[0] != '\0');
}

void ensureGuiRuntimeEnvironment()
{
    const char *xdgRuntime = std::getenv("XDG_RUNTIME_DIR");
    if (xdgRuntime == nullptr || xdgRuntime[0] == '\0') {
        const char *fallbackRuntime = "/tmp/xdg-runtime-control";
        mkdir(fallbackRuntime, 0700);
        chmod(fallbackRuntime, 0700);
        setenv("XDG_RUNTIME_DIR", fallbackRuntime, 1);
        LOG("[GUI] XDG_RUNTIME_DIR was missing; using " << fallbackRuntime);
        xdgRuntime = std::getenv("XDG_RUNTIME_DIR");
    }

    const char *glfwPlatform = std::getenv("GLFW_PLATFORM");
    if (glfwPlatform == nullptr || glfwPlatform[0] == '\0') {
        const char *waylandDisplay = std::getenv("WAYLAND_DISPLAY");
        const char *x11Display = std::getenv("DISPLAY");

        // Neither display server available — attempt to self-start weston.
        if ((waylandDisplay == nullptr || waylandDisplay[0] == '\0') &&
            (x11Display    == nullptr || x11Display[0]    == '\0')) {

            if (std::system("test -x /usr/bin/weston || command -v weston >/dev/null 2>&1") == 0) {
                LOG("[GUI] No display server found; attempting to start weston...");

                // Check if weston is already running before spawning a new one.
                if (std::system("pgrep -x weston >/dev/null 2>&1") != 0) {
                    std::system("/usr/bin/weston --socket=wayland-0 "
                                "--idle-time=0 --continue-without-input --use-pixman >>/tmp/weston.log 2>&1 &");
                }

                // Wait for the Wayland socket to appear (up to 10 s).
                const std::string socketPath = std::string(xdgRuntime) + "/wayland-0";
                for (int i = 0; i < 10; ++i) {
                    struct stat st{};
                    if (stat(socketPath.c_str(), &st) == 0 && S_ISSOCK(st.st_mode)) {
                        setenv("WAYLAND_DISPLAY", "wayland-0", 1);
                        LOG("[GUI] weston socket ready; WAYLAND_DISPLAY=wayland-0");
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                if (std::getenv("WAYLAND_DISPLAY") == nullptr) {
                    ERR("[GUI] weston socket not ready after 10 s; will try without Wayland");
                }
            }

            waylandDisplay = std::getenv("WAYLAND_DISPLAY");
        }

        if (waylandDisplay != nullptr && waylandDisplay[0] != '\0') {
            setenv("GLFW_PLATFORM", "wayland", 0);
        } else if (x11Display != nullptr && x11Display[0] != '\0') {
            setenv("GLFW_PLATFORM", "x11", 0);
        }

    }
    // QEMU/embedded: force Mesa software renderer when no DRI render node is available.
    // This covers virtio-gpu without virgl and any target where /dev/dri/renderD128
    // does not exist.  Wayland EGL still works via weston on top of pixman/scanout.
    if (std::getenv("LIBGL_ALWAYS_SOFTWARE") == nullptr) {
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        setenv("MESA_LOADER_DRIVER_OVERRIDE", "swrast", 1);
        LOG("[GUI] No DRI render node detected; using Mesa software renderer");
    }
}


int runGui(Proximity &proximitySensor, StatusLeds &statusLeds, TamperSwitch &tamperSwitch)
{
    int ret = 0;
    std::unique_ptr<Backlight> backlight;
    int wakeBrightness = SCREENSAVER_BACKLIGHT_BRIGHTNESS;
    bool lastBacklightPresence = false;
    bool hasLastBacklightPresence = false;

    try {
        backlight = std::make_unique<Backlight>();
        wakeBrightness = backlight->getBrightness();
        LOG("Backlight control available");
    } catch (const std::exception &e) {
        ERR("Backlight control unavailable: " << e.what());
    }

    LOG("[GUI] Startup mode: "
        << (hasDisplayServer() ? "desktop window (X11/Wayland)" : "KMS/DRM framebuffer"));

    ensureGuiRuntimeEnvironment();

    try {
        try {
            nanogui::init();
        } catch (const std::runtime_error &e) {
            // NFD (native file dialog) requires a display server (GTK). When running in
            // framebuffer mode without one, nanogui::init() throws after GLFW has already
            // been initialised successfully. File dialogs will be unavailable, but that
            // is acceptable in headless/framebuffer mode.
            if (std::string(e.what()) != "Could not initialize NFD!") {
                throw;
            }
            ERR("NFD unavailable (no display server); file dialogs will be disabled.");
        }
        statusLeds.setState(StatusLeds::STATE_READY);
        Screen screen(1024, 600, DIR_SHARED "badges", []() {
			reloadBadgesCache();
			LOG("Badges cache reloaded"); }, [&backlight, &proximitySensor, wakeBrightness]() {
			if (backlight) {
				backlight->setBrightness(wakeBrightness);
			}
			proximitySensor.resetAbsenceGrace(); });
        tamperSwitch.setTamperHandler([&screen, &statusLeds](bool detected) {
            nanogui::async([&screen, detected]() {
                screen.setTamperDetected(detected);
            });

            EventLog::addTamper(detected);

            if (detected) {
                statusLeds.showError(StatusLeds::ERROR_SYSTEM);
            } else {
                statusLeds.clearError();
                statusLeds.setState(StatusLeds::STATE_READY);
            }
        });

        tamperSwitch.start();
        proximitySensor.setDetectionHandler([&screen, &backlight, &lastBacklightPresence, &hasLastBacklightPresence, wakeBrightness](bool detected) {
            if (backlight && (!hasLastBacklightPresence || lastBacklightPresence != detected)) {
                backlight->setBrightness(detected ? wakeBrightness : SCREENSAVER_BACKLIGHT_BRIGHTNESS);
                lastBacklightPresence = detected;
                hasLastBacklightPresence = true;
            }
            nanogui::async([&screen, detected]() {
                screen.setPresenceDetected(detected);
            });
        });
        proximitySensor.start();
        screen.render();
        nanogui::run();
        proximitySensor.stop();
        tamperSwitch.stop();
        nanogui::shutdown();
    } catch (const std::exception &e) {
        ERR("GUI startup failed: " << e.what());
        ERR("Running in headless mode — access control active, no display");
        statusLeds.setState(StatusLeds::STATE_READY);
        proximitySensor.start();
        tamperSwitch.start();
        // Park here until the process is terminated by a signal.
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGTERM);
        sigdelset(&mask, SIGINT);
        sigsuspend(&mask);
        proximitySensor.stop();
        tamperSwitch.stop();
    }

    return ret;
}

int main(void)
{

    Proximity proximitySensor;
    StatusLeds statusLeds;
    LifeLed lifeLed;
    TamperSwitch tamperSwitch;
    g_statusLeds = &statusLeds;
    SignalHandlers &signalHandlers = SignalHandlers::instance();

    statusLeds.setState(StatusLeds::STATE_BOOTING);
    EventLog::addStarting();

    if (signalHandlers.installTerminationHandlers() != 0) {
        ERR("Failed to install termination signal handlers");
    }

    try {
        loadIos();
        LOG("IOs loaded");
    } catch (const std::exception &e) {
        ERR("IO startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_IO);
        lifeLed.stop();
        return 1;
    }

    try {
        loadBusses();
        LOG("Busses loaded");
    } catch (const std::exception &e) {
        ERR("Bus startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_IO);
        lifeLed.stop();
        return 1;
    }

    try {
        loadBadges();
        LOG("Badges loaded");
    } catch (const std::exception &e) {
        ERR("Badge startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_SYSTEM);
        lifeLed.stop();
        return 1;
    }

    try {
        if (loadSettings() != 0) {
            throw std::runtime_error("Unable to load settings");
        }
        LOG("Settings loaded");
    } catch (const std::exception &e) {
        ERR("Settings startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_CONFIG);
        lifeLed.stop();
        return 1;
    }

    try {
        startReaders(statusLeds);
        LOG("Readers started");
    } catch (const std::exception &e) {
        ERR("Reader startup failed: " << e.what());
        statusLeds.showError(hasNetworkReadersConfigured()
                                 ? StatusLeds::ERROR_NETWORK
                                 : StatusLeds::ERROR_READER);
        lifeLed.stop();
        return 1;
    }

    try {
        startActions();
        LOG("Actions started");
    } catch (const std::exception &e) {
        ERR("Action startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_SYSTEM);
        lifeLed.stop();
        return 1;
    }

    lifeLed.start();
    statusLeds.setState(StatusLeds::STATE_READY);

    const int ret = runGui(proximitySensor, statusLeds, tamperSwitch);
    if (ret != 0) {
        statusLeds.showError(StatusLeds::ERROR_SYSTEM);
    }

    lifeLed.stop();

    EventLog::addStopping();

    return ret;
}
