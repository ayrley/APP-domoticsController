#ifndef __HOME_BUTTON_H
#define __HOME_BUTTON_H

#include <functional>

#include "jarvisButton.h"

class HomeButton : public JarvisButton {
public:
	explicit HomeButton(nanogui::Widget *parent, std::function<void()> callback);
};

#endif