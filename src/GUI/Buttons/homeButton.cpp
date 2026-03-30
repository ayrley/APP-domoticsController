#include "homeButton.h"

HomeButton::HomeButton(nanogui::Widget *parent, std::function<void()> callback)
	: DomeButton(parent, "HOME", std::move(callback)) {
	set_home_style(true);
	set_fixed_size(nanogui::Vector2i(128, 42));
}