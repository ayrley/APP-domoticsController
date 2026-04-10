#ifndef __NESTED_SCROLL_PANEL_H
#define __NESTED_SCROLL_PANEL_H

#include <nanogui/vscrollpanel.h>

class NestedScrollPanel : public nanogui::VScrollPanel
{
public:
    explicit NestedScrollPanel(nanogui::Widget *parent);

    bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                            int button, int modifiers) override;
    bool mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                          int button, int modifiers) override;
    bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
};

#endif
