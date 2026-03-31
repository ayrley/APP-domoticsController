#ifndef __SETTINGS_PAGE_H
#define __SETTINGS_PAGE_H

#include <functional>

#include "defaultPage.h"

namespace nanogui
{
class Widget;
} // namespace nanogui

class NetworkSettingsPage;

class SettingsPage : public DefaultPage
{
public:
    explicit SettingsPage(nanogui::Widget *parent,
                          std::function<void()> onHome,
                          std::function<void()> onSave);

private:
    nanogui::Widget *m_mainSettingsPanel{nullptr};
    NetworkSettingsPage *m_networkSettingsPage{nullptr};

    void showMainSettings();
    void showNetworkSettings();
};

#endif
