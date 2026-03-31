#ifndef __DOME_BUTTON_H
#define __DOME_BUTTON_H

#include <functional>
#include <string>

#include <nanogui/widget.h>

class DomeButton : public nanogui::Widget
{
public:
    explicit DomeButton(nanogui::Widget *parent, const std::string &label,
                        std::function<void()> callback = nullptr);

    nanogui::Vector2i preferred_size_impl(NVGcontext *ctx) const override;
    bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
                            int modifiers) override;
    bool mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                          int button, int modifiers) override;
    bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
    bool mouse_enter_event(const nanogui::Vector2i &p, bool enter) override;
    void draw(NVGcontext *ctx) override;

    void setLabel(const std::string &label) { m_label = label; }
    const std::string &label() const { return m_label; }
    void setCallback(std::function<void()> callback) { m_callback = std::move(callback); }
    void setHomeStyle(bool enabled) { m_homeStyle = enabled; }
    void setPalette(float baseR, float baseG, float baseB,
                    float accentR, float accentG, float accentB);
    void setSelected(bool selected) { m_selected = selected; }
    bool selected() const { return m_selected; }

private:
    std::string m_label;
    std::function<void()> m_callback;
    bool m_hovered{false};
    bool m_pressed{false};
    bool m_dragged{false};
    int m_dragMotion{0};
    bool m_homeStyle{false};
    bool m_selected{false};
    bool m_customPalette{false};
    float m_baseR{0.0f};
    float m_baseG{0.65f};
    float m_baseB{0.9f};
    float m_accentR{0.0f};
    float m_accentG{0.85f};
    float m_accentB{1.0f};
};

#endif
