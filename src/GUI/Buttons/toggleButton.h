#ifndef __TOGGLE_BUTTON_H
#define __TOGGLE_BUTTON_H

#include <functional>
#include <string>

#include "domeButton.h"

class ToggleButton : public DomeButton
{
public:
    explicit ToggleButton(nanogui::Widget *parent,
                          const std::string &onLabel,
                          const std::string &offLabel,
                          bool initialState,
                          std::function<void(bool)> onToggle = nullptr);

    void setState(bool enabled);
    bool state() const { return m_state; }

private:
    bool m_state{false};
    std::string m_onLabel;
    std::string m_offLabel;
    std::function<void(bool)> m_onToggle;

    void applyVisualState();
};

#endif