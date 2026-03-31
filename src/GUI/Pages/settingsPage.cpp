#include "settingsPage.h"

#include "domeButton.h"
#include "networkSettingsPage.h"

#include <nanogui/nanogui.h>

SettingsPage::SettingsPage(nanogui::Widget *parent,
                           std::function<void()> onHome,
                           std::function<void()> onSave) :
    DefaultPage(parent, std::move(onHome))
{
    setPageTitle("System Settings");
    contentPanel()->set_layout(new nanogui::GroupLayout(0, 6, 18, 0));

    m_mainSettingsPanel = new nanogui::Widget(contentPanel());
    m_mainSettingsPanel->set_layout(new nanogui::GroupLayout(0, 6, 18, 0));

    auto *desc = new nanogui::Label(
        m_mainSettingsPanel,
        "This page is ready for runtime options like update intervals,\n"
        "network endpoints, and device-specific behavior.",
        "sans");
    desc->set_font_size(18);
    desc->set_color(nanogui::Color(130, 215, 225, 255));

    auto *controls = new nanogui::Widget(m_mainSettingsPanel);
    controls->set_layout(new nanogui::GroupLayout(10, 6, 0, 10));

    auto *networkBtn = new DomeButton(controls, "Network Settings", [this]() {
        showNetworkSettings();
    });
    networkBtn->set_fixed_size(nanogui::Vector2i(220, 40));

    m_networkSettingsPage = new NetworkSettingsPage(contentPanel(), [this]() {
        showMainSettings();
    });
    m_networkSettingsPage->set_visible(false);

    showMainSettings();
}

void SettingsPage::showMainSettings()
{
    setPageTitle("System Settings");
    if (m_mainSettingsPanel) {
        m_mainSettingsPanel->set_visible(true);
    }
    if (m_networkSettingsPage) {
        m_networkSettingsPage->set_visible(false);
    }
}

void SettingsPage::showNetworkSettings()
{
    setPageTitle("Network Settings");
    if (m_mainSettingsPanel) {
        m_mainSettingsPanel->set_visible(false);
    }
    if (m_networkSettingsPage) {
        m_networkSettingsPage->set_visible(true);
    }
}
