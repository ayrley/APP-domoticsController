#include "screen.h"
#include "system.h"

#include "GUI/jarvisBackground.h"
#include "GUI/metricBlock.h"

#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include <nanogui/nanogui.h>

Screen::Screen(int width, int height) {
    m_screen = new nanogui::Screen(nanogui::Vector2i(width, height), "Domotic Controller");
    m_stopStatsThread = false;

    JarvisBackground *background = new JarvisBackground(m_screen);
    background->set_position(nanogui::Vector2i(0, 0));
    background->set_fixed_size(nanogui::Vector2i(width, height));

    nanogui::Widget *panel = new nanogui::Widget(m_screen);
    panel->set_position(nanogui::Vector2i(26, 20));
    panel->set_fixed_size(nanogui::Vector2i(470, 260));
    panel->set_layout(new nanogui::GroupLayout());

    nanogui::Label *headerLabel = new nanogui::Label(panel, "Domotics Controller", "sans-bold");
    headerLabel->set_font_size(28);

    m_statusLabel = new nanogui::Label(panel, "Status: Running", "sans-bold");
    m_statusLabel->set_font_size(20);

    m_cpuLabel = createMetricBlock(panel, "CPU Usage", "collecting...", &m_cpuBar);
    m_ramLabel = createMetricBlock(panel, "RAM Usage", "collecting...", &m_ramBar);

    m_screen->perform_layout();
    m_screen->set_visible(true);

    startStatsUpdates();
}

void Screen::updateStatus(const std::string &status) {
    m_statusLabel->set_caption("Status: " + status);
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
                m_cpuLabel->set_caption(cpuUsage);
                m_cpuBar->set_value(cpuUsageFraction);
                m_ramLabel->set_caption(ramUsage);
                m_ramBar->set_value(ramUsageFraction);
                m_screen->perform_layout();
                m_screen->redraw();
            });
        }
    });
}

void Screen::updateSystemStats() {
    std::string ramUsage = System::readRamUsage();
    float ramUsageFraction = static_cast<float>(System::readRamUsageFraction());

    m_cpuLabel->set_caption("measuring...");
    m_cpuBar->set_value(0.0f);
    m_ramLabel->set_caption(ramUsage);
    m_ramBar->set_value(ramUsageFraction);
    m_screen->perform_layout();
    m_screen->redraw();
}

nanogui::Screen *Screen::getScreen() const {
    return m_screen;
}

Screen::~Screen() {
    m_stopStatsThread = true;
    if (m_statsThread.joinable()) {
        m_statsThread.join();
    }

    delete m_screen;
}