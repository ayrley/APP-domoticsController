#ifndef __BACKLIGHT_H
#define __BACKLIGHT_H

#include <fstream>
#include <stdexcept>
#include <string>

#define BACKLIGHT_PATH "/sys/class/backlight/backlight/brightness"

class Backlight
{
private:
    std::ifstream m_backlightFile;
    int m_brightness{0};
    bool m_isAvailable{false};

public:
    Backlight();
    ~Backlight();

    int getBrightness() const;
    void setBrightness(int value);
    bool isAvailable() const { return m_isAvailable; }
};

#endif // __BACKLIGHT_H
