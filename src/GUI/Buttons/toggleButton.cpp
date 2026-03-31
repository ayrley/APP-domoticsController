#include "toggleButton.h"

#include <utility>

ToggleButton::ToggleButton(nanogui::Widget *parent,
                           const std::string &onLabel,
                           const std::string &offLabel,
                           bool initialState,
                           std::function<void(bool)> onToggle) :
    DomeButton(parent, "", nullptr),
    m_state(initialState),
    m_onLabel(onLabel),
    m_offLabel(offLabel),
    m_onToggle(std::move(onToggle))
{
    setCallback([this]() {
        setState(!m_state);
        if (m_onToggle) {
            m_onToggle(m_state);
        }
    });
    applyVisualState();
}

void ToggleButton::setState(bool enabled)
{
    m_state = enabled;
    applyVisualState();
}

void ToggleButton::applyVisualState()
{
    if (m_state) {
        setLabel(m_onLabel);
        setPalette(0.10f, 0.55f, 0.20f, 0.20f, 0.88f, 0.35f);
    } else {
        setLabel(m_offLabel);
        setPalette(0.45f, 0.20f, 0.20f, 0.85f, 0.30f, 0.30f);
    }
}