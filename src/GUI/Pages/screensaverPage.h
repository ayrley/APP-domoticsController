#ifndef __SCREENSAVER_PAGE_H
#define __SCREENSAVER_PAGE_H

#include <functional>

#include <nanogui/widget.h>

namespace nanogui {
class Label;
}

class ScreensaverPage : public nanogui::Widget {
public:
	explicit ScreensaverPage(nanogui::Widget *parent,
	                         std::function<void()> onWakeRequest = nullptr);
	bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
	                      int modifiers) override;
	void perform_layout(NVGcontext *ctx) override;

private:
	nanogui::Label *m_titleLabel{nullptr};
	nanogui::Label *m_hintLabel{nullptr};
	std::function<void()> m_onWakeRequest;
};

#endif