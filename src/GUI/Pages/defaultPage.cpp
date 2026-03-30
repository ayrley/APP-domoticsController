#include "defaultPage.h"

#include "homeButton.h"

#include <algorithm>

#include <nanogui/vscrollpanel.h>

DefaultPage::DefaultPage(nanogui::Widget *parent, std::function<void()> onHome)
	: nanogui::Widget(parent) {
	set_layout(nullptr);

	m_homeButton = new HomeButton(this, [onHome]() {
		if (onHome) {
			onHome();
		}
	});

	m_scrollPanel = new nanogui::VScrollPanel(this);
	m_contentPanel = new nanogui::Widget(m_scrollPanel);
}

void DefaultPage::perform_layout(NVGcontext *ctx) {
	constexpr int kOuterMargin = 16;
	constexpr int kTopOffset = 64;
	constexpr int kBottomMargin = 14;
	constexpr int kHomePadding = 14;
	constexpr int kPageHeightScrollThreshold = 600;

	if (m_homeButton) {
		nanogui::Vector2i homeSize = m_homeButton->fixed_size();
		m_homeButton->set_position(
			nanogui::Vector2i(m_size.x() - homeSize.x() - kHomePadding, kHomePadding));
	}

	if (m_scrollPanel && m_contentPanel) {
		int viewportWidth = std::max(0, m_size.x() - (2 * kOuterMargin));
		int availableHeight = std::max(0, m_size.y() - kTopOffset - kBottomMargin);

		int viewportHeight = availableHeight;
		if (m_size.y() > kPageHeightScrollThreshold) {
			viewportHeight = std::min(
				availableHeight,
				kPageHeightScrollThreshold - kTopOffset - kBottomMargin);
		}

		m_scrollPanel->set_position(nanogui::Vector2i(kOuterMargin, kTopOffset));
		m_scrollPanel->set_fixed_size(nanogui::Vector2i(viewportWidth, viewportHeight));

		int contentHeight = std::max(availableHeight, m_contentPanel->preferred_size(ctx).y());
		m_contentPanel->set_fixed_size(nanogui::Vector2i(viewportWidth, contentHeight));
	}

	Widget::perform_layout(ctx);
}