#ifndef __NETWORK_SETTINGS_PAGE_H
#define __NETWORK_SETTINGS_PAGE_H

#include <functional>
#include <memory>

#include <nanogui/widget.h>
#include <settings.h>

namespace nanogui
{
class TextBox;
} // namespace nanogui

class DomeButton;
class ToggleButton;

class NetworkSettingsPage : public nanogui::Widget
{
public:
    explicit NetworkSettingsPage(nanogui::Widget *parent, std::function<void()> onBack);

private:
    ToggleButton *m_dhcpToggleButton{nullptr};
    bool m_dhcpEnabled{true};
    nanogui::TextBox *m_ipAddressBox{nullptr};
    nanogui::TextBox *m_netmaskBox{nullptr};
    nanogui::TextBox *m_gatewayBox{nullptr};
    nanogui::TextBox *m_dns1Box{nullptr};
    nanogui::TextBox *m_dns2Box{nullptr};
    nanogui::Widget *m_staticConfigPanel{nullptr};
    std::function<void()> m_onBack;

    void applyDhcpState(bool enabled);
    void updateStaticFieldsVisibility();
};

#endif