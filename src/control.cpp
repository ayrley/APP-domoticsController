#include <algorithm>
#include <cstdlib>
#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
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
#include "io.h"
#include "lifeLed.h"
#include "proximity.h"
#include "reader.h"
#include "screen.h"
#include "settings.h"
#include "statusLeds.h"
#include "tamperSwitch.h"

namespace
{
constexpr int SCREENSAVER_BACKLIGHT_BRIGHTNESS = 12;
}

Settings *g_userSettings = nullptr;
Settings *g_factorySettings = nullptr;
StatusLeds *g_statusLeds = nullptr;
std::vector<Badge *> *g_badges = new std::vector<Badge *>();
std::vector<IO *> *g_ios = new std::vector<IO *>();
std::vector<Action *> *g_actions = new std::vector<Action *>();

namespace
{
bool hasNetworkReadersConfigured()
{
    if (g_userSettings == nullptr || g_userSettings->getReaders() == nullptr) {
        return false;
    }

    for (Reader *reader : *g_userSettings->getReaders()) {
        if (reader != nullptr && reader->getLocationType() == RDR_LOC_IP) {
            return true;
        }
    }

    return false;
}
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
    int ret = 0;
    std::stringstream userSettingsStream;
    std::stringstream factorySettingsStream;
    std::string userSettingsFile;
    std::string factorySettingsFile;

    userSettingsStream << DIR_SHARED << "user.settings.domotics";
    userSettingsStream >> userSettingsFile;

    factorySettingsStream << DIR_ETC << "factory.settings.domotics";
    factorySettingsStream >> factorySettingsFile;

    Settings *factory = new Settings(factorySettingsFile);
    factory->parse();

    bool settingsExist = std::filesystem::exists(userSettingsFile);

    if (!settingsExist)
        std::filesystem::copy(factorySettingsFile, userSettingsFile);

    Settings *user = new Settings(userSettingsFile);
    user->parse();

    if (user->getType() != SET_USER) {
        user->setType(SET_USER);
        user->write();
    }
    g_userSettings = user;
    g_factorySettings = factory;

    return ret;
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
        json ioFileObject;
        std::ifstream jsonFile(entry.path());

        if (!jsonFile.is_open()) {
            ERR("Unable to open IO config: " + entry.path().string());
            continue;
        }

        if (jsonFile.peek() == std::ifstream::traits_type::eof()) {
            ERR("Skipping empty IO config: " + entry.path().string());
            continue;
        }

        try {
            ioFileObject = json::parse(jsonFile);
        } catch (const std::exception &e) {
            ERR("Skipping invalid IO config '" + entry.path().string() + "': " + e.what());
            continue;
        }

        if (!ioFileObject.contains("IOs") || !ioFileObject["IOs"].is_array()) {
            ERR("Skipping IO config without array 'IOs': " + entry.path().string());
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

int startReaders(StatusLeds &statusLeds)
{
    int ret = 0;

    for (auto singleReader : *g_userSettings->getReaders()) {
        singleReader->setBadges(g_badges);
        singleReader->setErrorHandler([&statusLeds](readerLocationType locationType, const std::string &message) {
            ERR("Reader failure: " << message);
            statusLeds.showError(locationType == RDR_LOC_IP
                                     ? StatusLeds::ERROR_NETWORK
                                     : StatusLeds::ERROR_READER);
        });
        singleReader->start();
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

bool shouldUseFramebufferGui()
{
    return !hasDisplayServer();
}

bool enableNullPlatformIfSupported()
{
    constexpr int glfwPlatformHint = 0x00050003;
    constexpr int glfwPlatformNull = 0x00060005;
    using GlfwPlatformSupportedFn = int (*)(int);

    void *symbol = dlsym(RTLD_DEFAULT, "glfwPlatformSupported");
    if (symbol == nullptr) {
        ERR("GLFW runtime does not expose platform capability probing; attempting default desktop backend");

        return false;
    }

    auto glfwPlatformSupportedFn = reinterpret_cast<GlfwPlatformSupportedFn>(symbol);
    if (glfwPlatformSupportedFn(glfwPlatformNull) == GLFW_FALSE) {
        ERR("GLFW null platform is unavailable; attempting default desktop backend");

        return false;
    }

    glfwInitHint(glfwPlatformHint, glfwPlatformNull);

    return true;
}

int runGui(Proximity &proximitySensor, StatusLeds &statusLeds, TamperSwitch &tamperSwitch)
{
    int ret = 0;
    bool useFramebufferGui = shouldUseFramebufferGui();
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
        << (useFramebufferGui ? "framebuffer/null platform" : "desktop window (X11/Wayland)"));

    try {
        if (useFramebufferGui) {
            LOG("No display server detected; GUI will run in framebuffer/null-platform mode.");
            enableNullPlatformIfSupported();
        }

        nanogui::init();
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
        statusLeds.showError(StatusLeds::ERROR_SYSTEM);
        proximitySensor.stop();
        tamperSwitch.stop();
        ret = 1;
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

    statusLeds.setState(StatusLeds::STATE_BOOTING);

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
        loadBadges();
        LOG("Badges loaded");
    } catch (const std::exception &e) {
        ERR("Badge startup failed: " << e.what());
        statusLeds.showError(StatusLeds::ERROR_SYSTEM);
        lifeLed.stop();
        return 1;
    }

    try {
        loadSettings();
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

    return ret;
}
