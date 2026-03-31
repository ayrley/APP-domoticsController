#ifndef __NETWORK_SETTINGS_PAGE_H
#define __NETWORK_SETTINGS_PAGE_H

#include <functional>
#include <memory>

#include <nanogui/widget.h>
#include <settings.h>

namespace nanogui
{
class CheckBox;
class TextBox;
} // namespace nanogui


class NetworkSettingsPage : public nanogui::Widget
{
public:
    explicit NetworkSettingsPage(nanogui::Widget *parent, std::function<void()> onBack);

private:
    nanogui::CheckBox *m_dhcpCheckbox{nullptr};
    nanogui::TextBox *m_ipAddressBox{nullptr};
    nanogui::TextBox *m_netmaskBox{nullptr};
    nanogui::TextBox *m_gatewayBox{nullptr};
    nanogui::TextBox *m_dns1Box{nullptr};
    nanogui::TextBox *m_dns2Box{nullptr};
    nanogui::Widget *m_staticConfigPanel{nullptr};
    std::function<void()> m_onBack;

    void updateStaticFieldsVisibility();
};

#endif