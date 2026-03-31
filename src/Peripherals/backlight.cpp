#include "backlight.h"

#include <algorithm>

Backlight::Backlight()
{
    m_backlightFile.open(BACKLIGHT_PATH);
    if (!m_backlightFile.is_open()) {
        throw std::runtime_error("Failed to open backlight control at " + std::string(BACKLIGHT_PATH));
    }

    m_backlightFile >> m_brightness;
    if (m_backlightFile.fail()) {
        throw std::runtime_error("Failed to read backlight brightness");
    }
}

Backlight::~Backlight()
{
    if (m_backlightFile.is_open()) {
        m_backlightFile.close();
    }
}

int Backlight::getBrightness()
{
    return m_brightness;
}

void Backlight::setBrightness(int value)
{
    m_brightness = std::max(0, value);

    std::ofstream backlightWriteFile(BACKLIGHT_PATH);

    if (!backlightWriteFile.is_open()) {
        throw std::runtime_error("Failed to open backlight control for writing at " + std::string(BACKLIGHT_PATH));
    }

    backlightWriteFile << m_brightness << std::endl;

    if (backlightWriteFile.fail()) {
        throw std::runtime_error("Failed to write backlight brightness");
    }
}
