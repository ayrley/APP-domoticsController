#include "screensaverPage.h"

#include <GLFW/glfw3.h>
#include <nanogui/label.h>

ScreensaverPage::ScreensaverPage(nanogui::Widget *parent,
	                             std::function<void()> onWakeRequest)
	: nanogui::Widget(parent),
	  m_onWakeRequest(std::move(onWakeRequest)) {
	set_layout(nullptr);

	m_titleLabel = new nanogui::Label(this, "DOMOTICS CONTROLLER", "sans-bold");
	m_titleLabel->set_font_size(38);
	m_titleLabel->set_color(nanogui::Color(0, 235, 255, 255));

	m_hintLabel = new nanogui::Label(this, "Approach the panel to wake the interface", "sans");
	m_hintLabel->set_font_size(22);
	m_hintLabel->set_color(nanogui::Color(130, 215, 225, 255));
	set_visible(false);
}

bool ScreensaverPage::mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
	                                     int modifiers) {
	if (Widget::mouse_button_event(p, button, down, modifiers)) {
		return true;
	}

	if (down && button == GLFW_MOUSE_BUTTON_LEFT) {
		if (m_onWakeRequest) {
			m_onWakeRequest();
		}
		return true;
	}

	return false;
}

void ScreensaverPage::perform_layout(NVGcontext *ctx) {
	Widget::perform_layout(ctx);

	if (!m_titleLabel || !m_hintLabel) {
		return;
	}

	const nanogui::Vector2i titleSize = m_titleLabel->preferred_size(ctx);
	const nanogui::Vector2i hintSize = m_hintLabel->preferred_size(ctx);
	const int centerX = m_size.x() / 2;
	const int centerY = m_size.y() / 2;
	const int gap = 18;

	m_titleLabel->set_position(nanogui::Vector2i(
		centerX - titleSize.x() / 2,
		centerY - titleSize.y() - gap / 2));
	m_hintLabel->set_position(nanogui::Vector2i(
		centerX - hintSize.x() / 2,
		centerY + gap / 2));
}