#include "backlight.h"

#include <algorithm>
#include <iostream>
#include <debug.h>

Backlight::Backlight()
    : m_brightness(100)
{
    try {
        m_backlightFile.open(BACKLIGHT_PATH);
        if (!m_backlightFile.is_open()) {
            throw std::runtime_error("Backlight device not found");
        }

        m_backlightFile >> m_brightness;
        if (m_backlightFile.fail()) {
            throw std::runtime_error("Failed to read backlight brightness");
        }
        m_isAvailable = true;
    } catch (const std::exception &e) {
        ERR("Backlight unavailable, using mock mode");
        m_backlightFile.close();
        m_isAvailable = false;
        m_brightness = 100;
    }
}

Backlight::~Backlight()
{
    if (m_backlightFile.is_open()) {
        m_backlightFile.close();
    }
}

int Backlight::getBrightness() const
{
    return m_brightness;
}

void Backlight::setBrightness(int value)
{
    m_brightness = std::max(0, value);

    if (!m_isAvailable) {
        return;
    }

    try {
        std::ofstream backlightWriteFile(BACKLIGHT_PATH);

        if (!backlightWriteFile.is_open()) {
            throw std::runtime_error("Failed to open backlight control for writing");
        }

        backlightWriteFile << m_brightness << std::endl;

        if (backlightWriteFile.fail()) {
            throw std::runtime_error("Failed to write backlight brightness");
        }
    } catch (const std::exception &e) {
        std::cerr << "Backlight write failed: " << e.what() << std::endl;
    }
}
