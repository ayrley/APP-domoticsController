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

public:
    Backlight();
    ~Backlight();

    int getBrightness();
    void setBrightness(int value);
};

#endif // __BACKLIGHT_H
