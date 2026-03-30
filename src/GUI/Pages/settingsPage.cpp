#include "settingsPage.h"

#include "jarvisButton.h"

#include <nanogui/nanogui.h>

SettingsPage::SettingsPage(nanogui::Widget *parent,
                           std::function<void()> onHome,
                           std::function<void()> onSave)
    : DefaultPage(parent, std::move(onHome)) {
	contentPanel()->set_layout(new nanogui::GroupLayout(15, 6, 18, 12));

	auto *header = new nanogui::Label(contentPanel(), "System Settings", "sans-bold");
	header->set_font_size(30);
	header->set_color(nanogui::Color(0, 235, 255, 255));

	auto *desc = new nanogui::Label(
	    contentPanel(),
	    "This page is ready for runtime options like update intervals,\n"
	    "network endpoints, and device-specific behavior.",
	    "sans");
	desc->set_font_size(18);
	desc->set_color(nanogui::Color(130, 215, 225, 255));

	auto *controls = new nanogui::Widget(contentPanel());
	controls->set_layout(new nanogui::GroupLayout(10, 6, 0, 10));

	auto *saveBtn = new JarvisButton(controls, "Save Settings", [onSave]() {
		if (onSave) {
			onSave();
		}
	});
	saveBtn->set_fixed_size(nanogui::Vector2i(220, 40));
}
