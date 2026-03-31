#ifndef __ORBIT_BUTTON_H
#define __ORBIT_BUTTON_H

#include <functional>
#include <string>

#include "domeButton.h"

class OrbitButton : public DomeButton
{
public:
    explicit OrbitButton(nanogui::Widget *parent,
                         const std::string &label,
                         std::function<void()> callback);
};

#endif
