#ifndef __HOME_BUTTON_H
#define __HOME_BUTTON_H

#include <functional>

#include "domeButton.h"

class HomeButton : public DomeButton
{
public:
    explicit HomeButton(nanogui::Widget *parent, std::function<void()> callback);
};

#endif
