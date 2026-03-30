#include "defaultPage.h"

#include "homeButton.h"

DefaultPage::DefaultPage(nanogui::Widget *parent, std::function<void()> onHome)
	: nanogui::Widget(parent) {
	set_layout(nullptr);

	m_homeButton = new HomeButton(this, [onHome]() {
		if (onHome) {
			onHome();
		}
	});

	m_contentPanel = new nanogui::Widget(this);
}

void DefaultPage::perform_layout(NVGcontext *ctx) {
	if (m_homeButton) {
		nanogui::Vector2i homeSize = m_homeButton->fixed_size();
		m_homeButton->set_position(nanogui::Vector2i(m_size.x() - homeSize.x() - 14, 14));
	}

	if (m_contentPanel) {
		m_contentPanel->set_position(nanogui::Vector2i(16, 64));
		m_contentPanel->set_fixed_size(nanogui::Vector2i(m_size.x() - 32, m_size.y() - 78));
	}

	Widget::perform_layout(ctx);
}