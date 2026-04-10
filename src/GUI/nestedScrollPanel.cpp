#include "nestedScrollPanel.h"

#include <GLFW/glfw3.h>

#include "debug.h"

NestedScrollPanel::NestedScrollPanel(nanogui::Widget *parent) :
    nanogui::VScrollPanel(parent) {}

bool NestedScrollPanel::mouse_motion_event(const nanogui::Vector2i &p,
                                           const nanogui::Vector2i &rel,
                                           int button,
                                           int modifiers)
{
    const bool consumed = nanogui::Widget::mouse_motion_event(p, rel, button, modifiers);

    if ((button & (1 << GLFW_MOUSE_BUTTON_1)) == 0) {
        return consumed;
    }

    if (children().empty()) {
        return consumed;
    }

    if (children()[0]->size().y() <= m_size.y()) {
        return consumed;
    }

    scroll_absolute(static_cast<float>(-rel.y()) * 3.0f);
    return true;
}

bool NestedScrollPanel::mouse_drag_event(const nanogui::Vector2i &p,
                                         const nanogui::Vector2i &rel,
                                         int button,
                                         int modifiers)
{
    if (nanogui::VScrollPanel::mouse_drag_event(p, rel, button, modifiers)) {
        return true;
    }

    if (children().empty()) {
        return false;
    }

    if (children()[0]->size().y() <= m_size.y()) {
        return false;
    }

    scroll_absolute(static_cast<float>(-rel.y()) * 3.0f);
    return true;
}

bool NestedScrollPanel::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel)
{
    if (nanogui::VScrollPanel::scroll_event(p, rel)) {
        return true;
    }

    return nanogui::Widget::scroll_event(p, rel);
}
