#ifndef __SETTINGS_PAGE_H
#define __SETTINGS_PAGE_H

#include <functional>

#include "defaultPage.h"

class SettingsPage : public DefaultPage
{
public:
    explicit SettingsPage(nanogui::Widget *parent,
                          std::function<void()> onHome,
                          std::function<void()> onSave);
};

#endif
