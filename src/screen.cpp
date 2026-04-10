#include "screen.h"
#include "system.h"

#include "GUI/domeBackground.h"
#include "GUI/Buttons/orbitButton.h"
#include "GUI/domeCore.h"
#include "GUI/Pages/badgePage.h"
#include "GUI/Pages/loggingPage.h"
#include "GUI/Pages/manualControlPage.h"
#include "GUI/Pages/overviewPage.h"
#include "GUI/Pages/screensaverPage.h"
#include "GUI/Pages/settingsPage.h"
#include "GUI/tamperBorderOverlay.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>

#include <nanogui/nanogui.h>
#include <nanogui/opengl.h>

namespace {
constexpr int PAGE_LANDING = 0;
constexpr int PAGE_BADGES = 1;
constexpr int PAGE_OVERVIEW = 2;
constexpr int PAGE_SETTINGS = 3;
constexpr int PAGE_MANUAL = 4;
constexpr int PAGE_LOGGING = 5;
constexpr int PAGE_SCREENSAVER = 6;
constexpr float PI = 3.14159265359f;
constexpr float ORBIT_BASE_OVERVIEW = -PI * 0.5f;
constexpr float ORBIT_BASE_BADGES = -PI * 0.1f;
constexpr float ORBIT_BASE_MANUAL = PI * 0.3f;
constexpr float ORBIT_BASE_SETTINGS = PI * 0.7f;
constexpr float ORBIT_BASE_LOGGING = PI * 1.1f;
}

std::string Screen::formatDutchDate(const std::tm &localTime)
{
    static const char *kWeekdays[] = {
        "zondag", "maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag"
    };
    static const char *kMonths[] = {
        "januari", "februari", "maart", "april", "mei", "juni",
        "juli", "augustus", "september", "oktober", "november", "december"
    };

    char buffer[96] = {0};
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%s %d %s %d",
                  kWeekdays[localTime.tm_wday],
                  localTime.tm_mday,
                  kMonths[localTime.tm_mon],
                  localTime.tm_year + 1900);
    return std::string(buffer);
}

std::string Screen::formatDutchTime(const std::tm &localTime)
{
    char buffer[16] = {0};
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%02d:%02d",
                  localTime.tm_hour,
                  localTime.tm_min);
    return std::string(buffer);
}

Screen::Screen(int width,
               int height,
               const std::string &badgesDir,
                             std::function<void()> onBadgesChanged,
                             std::function<void()> onScreensaverWakeRequest)
        : m_onBadgesChanged(std::move(onBadgesChanged)),
            m_onScreensaverWakeRequest(std::move(onScreensaverWakeRequest)),
            m_tamperOverlay(nullptr),
            m_tamperDetected(false) {
    m_screen = new nanogui::Screen(nanogui::Vector2i(width, height), "Domotic Controller");
    m_stopStatsThread = false;
    const nanogui::Vector2i screenSize = m_screen->size();

    m_background = new DomeBackground(m_screen);
    m_background->set_position(nanogui::Vector2i(0, 0));
    m_background->set_fixed_size(nanogui::Vector2i(width, height));

    m_landingPanel = nullptr;
    m_overviewPage = nullptr;
    m_settingsPage = nullptr;
    m_manualControlPage = nullptr;
    m_badgePage = nullptr;
    m_loggingPage = nullptr;
    m_screensaverPage = nullptr;
    m_overviewOrbitButton = nullptr;
    m_badgesOrbitButton = nullptr;
    m_settingsOrbitButton = nullptr;
    m_manualOrbitButton = nullptr;
    m_loggingOrbitButton = nullptr;
    m_landingCenterX = width / 2;
    m_landingCenterY = height / 2;
    m_landingOrbitRadius = 200.0f;

    buildLandingPage();
    buildOverviewPage();
    buildSettingsPage();
    buildManualControlPage();
    buildLoggingPage();
    buildScreensaverPage();

    m_badgePage = new BadgePage(
        m_screen,
        badgesDir,
        [this]() { switchPage(0); },
        [this]() {
            if (m_onBadgesChanged) {
                m_onBadgesChanged();
            }
        });
    m_badgePage->set_position(nanogui::Vector2i(0, 0));
    m_badgePage->set_fixed_size(screenSize);
    m_badgePage->set_visible(false);
    m_badgePage->setActivityCallback([this]() {
        if (m_onScreensaverWakeRequest) m_onScreensaverWakeRequest();
    });

    switchPage(PAGE_LANDING);

    m_tamperOverlay = new TamperBorderOverlay(m_screen, &m_tamperDetected);
    m_tamperOverlay->set_position(nanogui::Vector2i(0, 0));
    m_tamperOverlay->set_fixed_size(screenSize);

    m_screen->perform_layout();
    m_screen->set_visible(true);

    startStatsUpdates();
}

void Screen::buildLandingPage() {
    m_landingPanel = new nanogui::Widget(m_screen);
    const nanogui::Vector2i screenSize = m_screen->size();
    const int sw = screenSize.x();
    const int sh = screenSize.y();
    const nanogui::Vector2i center(sw / 2, sh / 2);
    m_landingCenterX = center.x();
    m_landingCenterY = center.y();

    m_landingPanel->set_position(nanogui::Vector2i(0, 0));
    m_landingPanel->set_fixed_size(screenSize);

    auto *core = new DomeCore(m_landingPanel);
    core->set_fixed_size(nanogui::Vector2i(240, 240));
    core->set_position(nanogui::Vector2i(center.x() - 120, center.y() - 120));

    m_overviewOrbitButton = new OrbitButton(m_landingPanel, "Overview", [this]() { switchPage(PAGE_OVERVIEW); });
    m_badgesOrbitButton = new OrbitButton(m_landingPanel, "Badges", [this]() { switchPage(PAGE_BADGES); });
    m_settingsOrbitButton = new OrbitButton(m_landingPanel, "Settings", [this]() { switchPage(PAGE_SETTINGS); });
    m_manualOrbitButton = new OrbitButton(m_landingPanel, "Manual", [this]() { switchPage(PAGE_MANUAL); });
    m_loggingOrbitButton = new OrbitButton(m_landingPanel, "Logs", [this]() { switchPage(PAGE_LOGGING); });

    // Keep orbit spacing balanced around the core while respecting small-screen margins.
    const float coreRadius = 120.0f;
    const float buttonRadius = static_cast<float>(m_overviewOrbitButton->fixed_size().x()) * 0.5f;
    const float desiredGap = 32.0f;
    const float edgePadding = 48.0f;
    const float desiredRadius = coreRadius + buttonRadius + desiredGap;
    const float maxRadiusX = std::min(
        static_cast<float>(center.x()) - edgePadding - buttonRadius,
        static_cast<float>(sw - center.x()) - edgePadding - buttonRadius);
    const float maxRadiusY = std::min(
        static_cast<float>(center.y()) - edgePadding - buttonRadius,
        static_cast<float>(sh - center.y()) - edgePadding - buttonRadius);
    m_landingOrbitRadius = std::max(coreRadius + buttonRadius + 12.0f,
                                    std::min(desiredRadius, std::min(maxRadiusX, maxRadiusY)));

    updateLandingOrbit(0.0f);
}

void Screen::updateLandingOrbit(float phase) {
    if (!m_overviewOrbitButton || !m_badgesOrbitButton || !m_settingsOrbitButton ||
        !m_manualOrbitButton || !m_loggingOrbitButton) {
        return;
    }

    auto placeButton = [this](OrbitButton *button, float baseAngle) {
        const float angle = baseAngle;
        const nanogui::Vector2i size = button->fixed_size();
        const float bx = static_cast<float>(m_landingCenterX) + std::cos(angle) * m_landingOrbitRadius - size.x() * 0.5f;
        const float by = static_cast<float>(m_landingCenterY) + std::sin(angle) * m_landingOrbitRadius - size.y() * 0.5f;
        button->set_position(nanogui::Vector2i(static_cast<int>(bx), static_cast<int>(by)));
    };

    placeButton(m_overviewOrbitButton, ORBIT_BASE_OVERVIEW + phase);
    placeButton(m_badgesOrbitButton, ORBIT_BASE_BADGES + phase);
    placeButton(m_manualOrbitButton, ORBIT_BASE_MANUAL + phase);
    placeButton(m_settingsOrbitButton, ORBIT_BASE_SETTINGS + phase);
    placeButton(m_loggingOrbitButton, ORBIT_BASE_LOGGING + phase);
}

void Screen::buildOverviewPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_overviewPage = new OverviewPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); });
    m_overviewPage->set_position(nanogui::Vector2i(0, 0));
    m_overviewPage->set_fixed_size(screenSize);
    m_overviewPage->setActivityCallback([this]() {
        if (m_onScreensaverWakeRequest) m_onScreensaverWakeRequest();
    });
}

void Screen::buildSettingsPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_settingsPage = new SettingsPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); },
        [this]() { updateStatus("Settings saved"); });
    m_settingsPage->set_position(nanogui::Vector2i(0, 0));
    m_settingsPage->set_fixed_size(screenSize);
    m_settingsPage->setActivityCallback([this]() {
        if (m_onScreensaverWakeRequest) m_onScreensaverWakeRequest();
    });
}

void Screen::buildManualControlPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_manualControlPage = new ManualControlPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); });
    m_manualControlPage->set_position(nanogui::Vector2i(0, 0));
    m_manualControlPage->set_fixed_size(screenSize);
    m_manualControlPage->setActivityCallback([this]() {
        if (m_onScreensaverWakeRequest) m_onScreensaverWakeRequest();
    });
}

void Screen::buildLoggingPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_loggingPage = new LoggingPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); });
    m_loggingPage->set_position(nanogui::Vector2i(0, 0));
    m_loggingPage->set_fixed_size(screenSize);
    m_loggingPage->set_visible(false);
    m_loggingPage->setActivityCallback([this]() {
        if (m_onScreensaverWakeRequest) m_onScreensaverWakeRequest();
    });
}

void Screen::buildScreensaverPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_screensaverPage = new ScreensaverPage(m_screen, [this]() {
        if (m_onScreensaverWakeRequest) {
            m_onScreensaverWakeRequest();
        }
        setPresenceDetected(true);
    });
    m_screensaverPage->set_position(nanogui::Vector2i(0, 0));
    m_screensaverPage->set_fixed_size(screenSize);
}

void Screen::updateStatus(const std::string &status) {
    if (m_overviewPage) {
        m_overviewPage->setStatus(status);
    }
    m_screen->redraw();
}

void Screen::render() {
    m_screen->draw_all();
}

void Screen::startStatsUpdates() {
    m_statsThread = std::thread([this]() {
        System::CpuSample previousSample = System::readCpuSample();

        updateSystemStats();

        while (!m_stopStatsThread.load()) {
            for (int tick = 0; tick < 50 && !m_stopStatsThread.load(); ++tick) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (m_stopStatsThread.load()) {
                break;
            }

            System::CpuSample currentSample = System::readCpuSample();

            std::string cpuUsage = System::formatCpuUsage(previousSample, currentSample);
            float cpuUsageFraction = static_cast<float>(System::cpuUsageFraction(previousSample, currentSample));
            std::string ramUsage = System::readRamUsage();
            float ramUsageFraction = static_cast<float>(System::readRamUsageFraction());
            const std::time_t nowTime = std::time(nullptr);
            std::tm localTime {};
            localtime_r(&nowTime, &localTime);
            std::string dateCaption = formatDutchDate(localTime);
            std::string timeCaption = formatDutchTime(localTime);
            previousSample = currentSample;

            nanogui::async([this, cpuUsage, cpuUsageFraction, ramUsage, ramUsageFraction, dateCaption, timeCaption]() {
                float loadFactor = (cpuUsageFraction + ramUsageFraction) * 0.5f;
                if (m_overviewPage) {
                    m_overviewPage->setCpu(cpuUsage, cpuUsageFraction);
                    m_overviewPage->setRam(ramUsage, ramUsageFraction);
                    m_overviewPage->setDateTime(dateCaption, timeCaption);
                }
                m_background->setLoadFactor(loadFactor);
                m_screen->perform_layout();
                m_screen->redraw();
            });
        }
    });
}

void Screen::updateSystemStats() {
    std::string ramUsage = System::readRamUsage();
    float ramUsageFraction = static_cast<float>(System::readRamUsageFraction());
    const std::time_t nowTime = std::time(nullptr);
    std::tm localTime {};
    localtime_r(&nowTime, &localTime);
    std::string dateCaption = formatDutchDate(localTime);
    std::string timeCaption = formatDutchTime(localTime);

    if (m_overviewPage) {
        m_overviewPage->setCpu("measuring...", 0.0f);
        m_overviewPage->setRam(ramUsage, ramUsageFraction);
        m_overviewPage->setDateTime(dateCaption, timeCaption);
    }
    m_background->setLoadFactor(ramUsageFraction * 0.5f);
    m_screen->perform_layout();
    m_screen->redraw();
}

nanogui::Screen *Screen::getScreen() const {
    return m_screen;
}

void Screen::setPresenceDetected(bool detected) {
    if (m_presenceDetected == detected) {
        return;
    }

    m_presenceDetected = detected;
    switchPage(detected ? PAGE_LANDING : PAGE_SCREENSAVER);
}

void Screen::setTamperDetected(bool detected)
{
    if (m_tamperDetected == detected) {
        return;
    }

    m_tamperDetected = detected;

    m_screen->redraw();
}

void Screen::switchPage(int page) {
    m_currentPage = page;
    m_landingPanel->set_visible(page == PAGE_LANDING);
    m_badgePage->set_visible(page == PAGE_BADGES);
    m_overviewPage->set_visible(page == PAGE_OVERVIEW);
    m_settingsPage->set_visible(page == PAGE_SETTINGS);
    m_manualControlPage->set_visible(page == PAGE_MANUAL);
    if (m_loggingPage) {
        m_loggingPage->set_visible(page == PAGE_LOGGING);
    }
    if (m_screensaverPage) {
        m_screensaverPage->set_visible(page == PAGE_SCREENSAVER);
    }
    if (page == PAGE_BADGES)
        m_badgePage->refresh();
    m_screen->perform_layout();
    m_screen->redraw();
}

Screen::~Screen() {
    m_stopStatsThread = true;
    if (m_statsThread.joinable()) {
        m_statsThread.join();
    }

    delete m_screen;
}