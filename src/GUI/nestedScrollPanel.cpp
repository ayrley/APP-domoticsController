#include "nestedScrollPanel.h"

NestedScrollPanel::NestedScrollPanel(nanogui::Widget *parent) :
    nanogui::VScrollPanel(parent) {}

bool NestedScrollPanel::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel)
{
    if (nanogui::Widget::scroll_event(p, rel)) {
        return true;
    }

    return nanogui::VScrollPanel::scroll_event(p, rel);
}
