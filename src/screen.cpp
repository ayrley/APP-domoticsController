#include "screen.h"
#include "system.h"

#include "GUI/jarvisBackground.h"
#include "GUI/Buttons/jarvisButton.h"
#include "GUI/jarvisCore.h"
#include "GUI/Pages/badgePage.h"
#include "GUI/Pages/manualControlPage.h"
#include "GUI/Pages/overviewPage.h"
#include "GUI/Pages/settingsPage.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <thread>

#include <nanogui/nanogui.h>

namespace {
constexpr int PAGE_LANDING = 0;
constexpr int PAGE_BADGES = 1;
constexpr int PAGE_OVERVIEW = 2;
constexpr int PAGE_SETTINGS = 3;
constexpr int PAGE_MANUAL = 4;
constexpr float PI = 3.14159265359f;
constexpr float ORBIT_BASE_OVERVIEW = -PI * 0.5f;
constexpr float ORBIT_BASE_BADGES = 0.0f;
constexpr float ORBIT_BASE_MANUAL = PI * 0.5f;
constexpr float ORBIT_BASE_SETTINGS = PI;
}

Screen::Screen(int width,
               int height,
               const std::string &badgesDir,
               std::function<void()> onBadgesChanged)
    : m_onBadgesChanged(std::move(onBadgesChanged)) {
    m_screen = new nanogui::Screen(nanogui::Vector2i(width, height), "Domotic Controller");
    m_stopStatsThread = false;
    const nanogui::Vector2i screenSize = m_screen->size();

    m_background = new JarvisBackground(m_screen);
    m_background->set_position(nanogui::Vector2i(0, 0));
    m_background->set_fixed_size(nanogui::Vector2i(width, height));

    m_landingPanel = nullptr;
    m_overviewPage = nullptr;
    m_settingsPage = nullptr;
    m_manualControlPage = nullptr;
    m_badgePage = nullptr;
    m_overviewOrbitButton = nullptr;
    m_badgesOrbitButton = nullptr;
    m_settingsOrbitButton = nullptr;
    m_manualOrbitButton = nullptr;
    m_landingCenterX = width / 2;
    m_landingCenterY = height / 2;
    m_landingOrbitRadius = 245.0f;

    buildLandingPage();
    buildOverviewPage();
    buildSettingsPage();
    buildManualControlPage();

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

    switchPage(PAGE_LANDING);
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

    auto *title = new nanogui::Label(m_landingPanel, "DOMOTICS CONTROL", "sans-bold");
    title->set_font_size(34);
    title->set_color(nanogui::Color(0, 235, 255, 255));
    title->set_position(nanogui::Vector2i(center.x() - 170, 34));

    auto *core = new JarvisCore(m_landingPanel, "JARVIS");
    core->set_fixed_size(nanogui::Vector2i(240, 240));
    core->set_position(nanogui::Vector2i(center.x() - 120, center.y() - 120));

    m_overviewOrbitButton = new JarvisButton(m_landingPanel, "Overview", [this]() { switchPage(PAGE_OVERVIEW); });
    m_badgesOrbitButton = new JarvisButton(m_landingPanel, "Badges", [this]() { switchPage(PAGE_BADGES); });
    m_settingsOrbitButton = new JarvisButton(m_landingPanel, "Settings", [this]() { switchPage(PAGE_SETTINGS); });
    m_manualOrbitButton = new JarvisButton(m_landingPanel, "Manual", [this]() { switchPage(PAGE_MANUAL); });
    m_overviewOrbitButton->set_fixed_size(nanogui::Vector2i(118, 118));
    m_badgesOrbitButton->set_fixed_size(nanogui::Vector2i(118, 118));
    m_settingsOrbitButton->set_fixed_size(nanogui::Vector2i(118, 118));
    m_manualOrbitButton->set_fixed_size(nanogui::Vector2i(118, 118));

    updateLandingOrbit(0.0f);
}

void Screen::updateLandingOrbit(float phase) {
    if (!m_overviewOrbitButton || !m_badgesOrbitButton || !m_settingsOrbitButton || !m_manualOrbitButton) {
        return;
    }

    auto placeButton = [this](JarvisButton *button, float baseAngle) {
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
}

void Screen::buildOverviewPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_overviewPage = new OverviewPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); });
    m_overviewPage->set_position(nanogui::Vector2i(0, 0));
    m_overviewPage->set_fixed_size(screenSize);
}

void Screen::buildSettingsPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_settingsPage = new SettingsPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); },
        [this]() { updateStatus("Settings saved"); });
    m_settingsPage->set_position(nanogui::Vector2i(0, 0));
    m_settingsPage->set_fixed_size(screenSize);
}

void Screen::buildManualControlPage() {
    const nanogui::Vector2i screenSize = m_screen->size();
    m_manualControlPage = new ManualControlPage(
        m_screen,
        [this]() { switchPage(PAGE_LANDING); });
    m_manualControlPage->set_position(nanogui::Vector2i(0, 0));
    m_manualControlPage->set_fixed_size(screenSize);
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
            previousSample = currentSample;

            nanogui::async([this, cpuUsage, cpuUsageFraction, ramUsage, ramUsageFraction]() {
                float loadFactor = (cpuUsageFraction + ramUsageFraction) * 0.5f;
                if (m_overviewPage) {
                    m_overviewPage->setCpu(cpuUsage, cpuUsageFraction);
                    m_overviewPage->setRam(ramUsage, ramUsageFraction);
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

    if (m_overviewPage) {
        m_overviewPage->setCpu("measuring...", 0.0f);
        m_overviewPage->setRam(ramUsage, ramUsageFraction);
    }
    m_background->setLoadFactor(ramUsageFraction * 0.5f);
    m_screen->perform_layout();
    m_screen->redraw();
}

nanogui::Screen *Screen::getScreen() const {
    return m_screen;
}

void Screen::switchPage(int page) {
    m_currentPage = page;
    m_landingPanel->set_visible(page == PAGE_LANDING);
    m_badgePage->set_visible(page == PAGE_BADGES);
    m_overviewPage->set_visible(page == PAGE_OVERVIEW);
    m_settingsPage->set_visible(page == PAGE_SETTINGS);
    m_manualControlPage->set_visible(page == PAGE_MANUAL);
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