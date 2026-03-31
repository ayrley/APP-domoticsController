#include "networkSettingsPage.h"

#include "domeButton.h"
#include "toggleButton.h"

#include <nanogui/nanogui.h>
#include <nanogui/screen.h>

#include "../../Peripherals/network.h"

extern std::vector<Settings *> *g_settings;

NetworkSettingsPage::NetworkSettingsPage(nanogui::Widget *parent,
                                         std::function<void()> onBack) :
    nanogui::Widget(parent),
    m_onBack(std::move(onBack))
{
    set_layout(new nanogui::GroupLayout(0, 6, 18, 0));

    auto *desc = new nanogui::Label(
        this,
        "Configure DHCP or enter a static IPv4 configuration.",
        "sans");
    desc->set_font_size(18);
    desc->set_color(nanogui::Color(130, 215, 225, 255));

    auto *dhcpRow = new nanogui::Widget(this);
    dhcpRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 10));
    dhcpRow->set_fixed_height(28);

    auto *dhcpLabel = new nanogui::Label(dhcpRow, "Enable DHCP", "sans-bold");
    dhcpLabel->set_font_size(16);
    dhcpLabel->set_color(nanogui::Color(150, 180, 200, 255));

    m_dhcpEnabled = g_settings->at(0)->getNetwork()->getDhcp();
    m_dhcpToggleButton = new ToggleButton(
        dhcpRow,
        "ON  [====]",
        "[====]  OFF",
        m_dhcpEnabled,
        [this](bool enabled) {
            applyDhcpState(enabled);

            if (auto *nanoScreen = screen()) {
                nanoScreen->perform_layout();
                nanoScreen->redraw();
            }
        });
    m_dhcpToggleButton->set_fixed_size(nanogui::Vector2i(130, 32));
    applyDhcpState(m_dhcpEnabled);

    m_staticConfigPanel = new nanogui::Widget(this);
    m_staticConfigPanel->set_layout(new nanogui::GroupLayout(0, 6, 0, 0));

    auto makeField = [this](const std::string &labelText,
                            const std::string &value,
                            nanogui::TextBox **out) {
        auto *label = new nanogui::Label(m_staticConfigPanel, labelText, "sans-bold");
        label->set_font_size(14);
        label->set_color(nanogui::Color(150, 180, 200, 255));

        auto *textBox = new nanogui::TextBox(m_staticConfigPanel, value);
        textBox->set_editable(true);
        textBox->set_font_size(15);
        textBox->set_fixed_size(nanogui::Vector2i(360, 34));
        textBox->set_alignment(nanogui::TextBox::Alignment::Left);
        *out = textBox;
    };

    makeField("IP Address", g_settings->at(0)->getNetwork()->getIpAddress(), &m_ipAddressBox);
    makeField("Netmask", g_settings->at(0)->getNetwork()->getNetmask(), &m_netmaskBox);
    makeField("Gateway", g_settings->at(0)->getNetwork()->getGateway(), &m_gatewayBox);
    makeField("DNS 1", g_settings->at(0)->getNetwork()->getDns1(), &m_dns1Box);
    makeField("DNS 2", g_settings->at(0)->getNetwork()->getDns2(), &m_dns2Box);

    auto *buttonRow = new nanogui::Widget(this);
    buttonRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
    buttonRow->set_fixed_height(44);

    auto *saveButton = new DomeButton(buttonRow, "Save", [this]() {
        g_settings->at(0)->getNetwork()->setIpAddress(m_ipAddressBox->value());
        g_settings->at(0)->getNetwork()->setNetmask(m_netmaskBox->value());
        g_settings->at(0)->getNetwork()->setGateway(m_gatewayBox->value());
        g_settings->at(0)->getNetwork()->setDns(m_dns1Box->value(), m_dns2Box->value());
        g_settings->at(0)->getNetwork()->save();
        g_settings->at(0)->write();
        if (m_onBack) {
            m_onBack();
        }
    });
    saveButton->set_fixed_size(nanogui::Vector2i(140, 40));

    auto *cancelButton = new DomeButton(buttonRow, "Back", [this]() {
        if (m_onBack) {
            m_onBack();
        }
    });
    cancelButton->set_fixed_size(nanogui::Vector2i(140, 40));

    updateStaticFieldsVisibility();
}

void NetworkSettingsPage::updateStaticFieldsVisibility()
{
    if (m_staticConfigPanel) {
        m_staticConfigPanel->set_visible(!m_dhcpEnabled);
    }
}

void NetworkSettingsPage::applyDhcpState(bool enabled)
{
    m_dhcpEnabled = enabled;
    g_settings->at(0)->getNetwork()->setDhcp(enabled);

    if (m_dhcpToggleButton) {
        if (m_dhcpToggleButton->state() != enabled) {
            m_dhcpToggleButton->setState(enabled);
        }
    }

    updateStaticFieldsVisibility();
}