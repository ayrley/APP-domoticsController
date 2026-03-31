#ifndef __FROST_PANEL_H
#define __FROST_PANEL_H

#include <nanogui/widget.h>

namespace nanogui
{
class VScrollPanel;
}

class FrostPanel : public nanogui::Widget
{
public:
    explicit FrostPanel(nanogui::Widget *parent);
    void setOpacityPercent(float percent);
    void setScrollTarget(nanogui::VScrollPanel *scrollTarget) { m_scrollTarget = scrollTarget; }
    float opacityPercent() const { return m_opacityPercent; }
    bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
                            int modifiers) override;
    bool mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                          int button, int modifiers) override;
    bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
    void perform_layout(NVGcontext *ctx) override;
    void draw(NVGcontext *ctx) override;

private:
    float m_opacityPercent{0.0f};
    int m_contentMargin{16};
    nanogui::VScrollPanel *m_scrollTarget{nullptr};
    bool m_dragScrollActive{false};
};

#endif
