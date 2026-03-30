#include "manualControlPage.h"

#include "domeButton.h"

#include <nanogui/nanogui.h>

ManualControlPage::ManualControlPage(nanogui::Widget *parent, std::function<void()> onHome)
	: DefaultPage(parent, std::move(onHome)) {
	contentPanel()->set_layout(new nanogui::GroupLayout(15, 6, 18, 12));

	auto *header = new nanogui::Label(contentPanel(), "Manual Control", "sans-bold");
	header->set_font_size(30);
	header->set_color(nanogui::Color(0, 235, 255, 255));

	auto *desc = new nanogui::Label(
		contentPanel(),
		"Directly trigger configured actions for testing and maintenance.",
		"sans");
	desc->set_font_size(18);
	desc->set_color(nanogui::Color(130, 215, 225, 255));

	m_statusLabel = new nanogui::Label(contentPanel(), "Status: Idle", "sans-bold");
	m_statusLabel->set_font_size(18);

	auto *actionsRow = new nanogui::Widget(contentPanel());
	actionsRow->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
	actionsRow->set_fixed_height(44);

	auto *unlockButton = new DomeButton(actionsRow, "Unlock Door", [this]() {
		m_statusLabel->set_caption("Status: Unlock Door triggered");
	});
	unlockButton->set_fixed_size(nanogui::Vector2i(180, 40));

	auto *lockButton = new DomeButton(actionsRow, "Lock Door", [this]() {
		m_statusLabel->set_caption("Status: Lock Door triggered");
	});
	lockButton->set_fixed_size(nanogui::Vector2i(160, 40));

	auto *lightButton = new DomeButton(actionsRow, "Toggle Light", [this]() {
		m_statusLabel->set_caption("Status: Toggle Light triggered");
	});
	lightButton->set_fixed_size(nanogui::Vector2i(180, 40));
}
