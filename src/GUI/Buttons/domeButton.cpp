#include "domeButton.h"

#include <cmath>

#include <GLFW/glfw3.h>
#include <nanogui/opengl.h>
#include <nanogui/vscrollpanel.h>

namespace
{

constexpr int kDragThresholdPixels = 6;

nanogui::VScrollPanel *findParentScrollPanel(nanogui::Widget *widget)
{
    nanogui::Widget *current = widget ? widget->parent() : nullptr;
    while (current) {
        auto *scroll = dynamic_cast<nanogui::VScrollPanel *>(current);
        if (scroll) {
            return scroll;
        }
        current = current->parent();
    }
    return nullptr;
}

nanogui::Vector2i mapPointToParentSpace(const nanogui::Widget *source,
                                        const nanogui::Widget *target,
                                        const nanogui::Vector2i &pointInSourceParent)
{
    if (!source || !target) {
        return pointInSourceParent;
    }

    nanogui::Vector2i pointInSourceLocal = pointInSourceParent - source->position();
    nanogui::Vector2i pointInAbsolute = source->absolute_position() + pointInSourceLocal;

    if (auto *targetParent = target->parent()) {
        return pointInAbsolute - targetParent->absolute_position();
    }
    return pointInAbsolute;
}

} // namespace

DomeButton::DomeButton(nanogui::Widget *parent, const std::string &label,
                       std::function<void()> callback) :
    nanogui::Widget(parent), m_label(label), m_callback(std::move(callback)) {}

void DomeButton::setPalette(float baseR, float baseG, float baseB,
                            float accentR, float accentG, float accentB)
{
    m_customPalette = true;
    m_baseR = baseR;
    m_baseG = baseG;
    m_baseB = baseB;
    m_accentR = accentR;
    m_accentG = accentG;
    m_accentB = accentB;
}

nanogui::Vector2i DomeButton::preferred_size_impl(NVGcontext *ctx) const
{
    nvgFontSize(ctx, 16.0f);
    nvgFontFace(ctx, "sans-bold");
    float bounds[4];
    nvgTextBounds(ctx, 0, 0, m_label.c_str(), nullptr, bounds);
    int tw = static_cast<int>(bounds[2] - bounds[0]);
    return nanogui::Vector2i(tw + 48, 36);
}

bool DomeButton::mouse_button_event(const nanogui::Vector2i &p, int button,
                                    bool down, int modifiers)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (down) {
            m_pressed = true;
            m_dragged = false;
            m_dragMotion = 0;
        } else if (m_pressed) {
            m_pressed = false;
            if (!m_dragged && m_callback)
                m_callback();
            m_dragged = false;
            m_dragMotion = 0;
        }
        return true;
    }
    return nanogui::Widget::mouse_button_event(p, button, down, modifiers);
}

bool DomeButton::mouse_drag_event(const nanogui::Vector2i &p,
                                  const nanogui::Vector2i &rel,
                                  int button,
                                  int modifiers)
{
    if (m_pressed && (button & (1 << GLFW_MOUSE_BUTTON_LEFT)) != 0) {
        m_dragMotion += std::abs(rel.x()) + std::abs(rel.y());
        if (m_dragMotion >= kDragThresholdPixels) {
            m_dragged = true;
        }

        if (m_dragged) {
            if (auto *scrollPanel = findParentScrollPanel(this)) {
                nanogui::Vector2i mapped = mapPointToParentSpace(this, scrollPanel, p);
                return scrollPanel->mouse_drag_event(mapped, rel, button, modifiers);
            }
        }

        return true;
    }

    return nanogui::Widget::mouse_drag_event(p, rel, button, modifiers);
}

bool DomeButton::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel)
{
    if (auto *scrollPanel = findParentScrollPanel(this)) {
        nanogui::Vector2i mapped = mapPointToParentSpace(this, scrollPanel, p);
        return scrollPanel->scroll_event(mapped, rel);
    }
    return nanogui::Widget::scroll_event(p, rel);
}

bool DomeButton::mouse_enter_event(const nanogui::Vector2i &p, bool enter)
{
    m_hovered = enter;
    return nanogui::Widget::mouse_enter_event(p, enter);
}

void DomeButton::draw(NVGcontext *ctx)
{
    Widget::draw(ctx);

    float x = static_cast<float>(m_pos.x());
    float y = static_cast<float>(m_pos.y());
    float w = static_cast<float>(m_size.x());
    float h = static_cast<float>(m_size.y());
    bool isRound = std::fabs(w - h) < 0.5f;
    float radius = std::min(w, h) * 0.5f;
    float cx = x + w * 0.5f;
    float cy = y + h * 0.5f;
    float baseR = m_homeStyle ? 0.96f : 0.0f;
    float baseG = m_homeStyle ? 0.36f : 0.65f;
    float baseB = m_homeStyle ? 0.08f : 0.9f;
    float accentR = m_homeStyle ? 1.0f : 0.0f;
    float accentG = m_homeStyle ? 0.65f : 0.85f;
    float accentB = m_homeStyle ? 0.12f : 1.0f;
    if (m_customPalette) {
        baseR = m_baseR;
        baseG = m_baseG;
        baseB = m_baseB;
        accentR = m_accentR;
        accentG = m_accentG;
        accentB = m_accentB;
    }

    float bgAlpha = m_selected ? 0.44f : (m_pressed ? 0.55f : (m_hovered ? 0.35f : 0.18f));
    nvgBeginPath(ctx);
    if (isRound) {
        nvgCircle(ctx, cx, cy, radius - 1.0f);
    } else {
        nvgRoundedRect(ctx, x, y, w, h, 5.0f);
    }
    nvgFillColor(ctx, nvgRGBAf(baseR, baseG, baseB, bgAlpha));
    nvgFill(ctx);

    float borderAlpha = m_selected ? 1.0f : (m_hovered ? 1.0f : 0.72f);
    nvgBeginPath(ctx);
    if (isRound) {
        nvgCircle(ctx, cx, cy, radius - 1.5f);
    } else {
        nvgRoundedRect(ctx, x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, 5.0f);
    }
    nvgStrokeColor(ctx, nvgRGBAf(accentR, accentG, accentB, borderAlpha * 0.55f));
    nvgStrokeWidth(ctx, m_homeStyle ? 1.7f : 1.0f);
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    if (isRound) {
        nvgArc(ctx, cx, cy, radius - 2.5f, -2.6f, -1.8f, NVG_CW);
        nvgArc(ctx, cx, cy, radius - 2.5f, 0.55f, 1.35f, NVG_CW);
    } else {
        float ac = 11.0f;
        nvgMoveTo(ctx, x + ac, y);
        nvgLineTo(ctx, x, y);
        nvgLineTo(ctx, x, y + ac);
        nvgMoveTo(ctx, x + w - ac, y + h);
        nvgLineTo(ctx, x + w, y + h);
        nvgLineTo(ctx, x + w, y + h - ac);
    }
    nvgStrokeColor(ctx, nvgRGBAf(accentR, accentG, accentB, borderAlpha));
    nvgStrokeWidth(ctx, m_homeStyle ? 2.3f : 1.8f);
    nvgStroke(ctx);

    if (m_hovered || m_selected) {
        nvgBeginPath(ctx);
        if (isRound) {
            nvgCircle(ctx, cx, cy, radius + 2.5f);
        } else {
            nvgRoundedRect(ctx, x - 2.5f, y - 2.5f, w + 5.0f, h + 5.0f, 8.0f);
        }
        float glowAlpha = m_selected ? 0.44f : (m_homeStyle ? 0.38f : 0.28f);
        nvgStrokeColor(ctx, nvgRGBAf(accentR, accentG, accentB, glowAlpha));
        nvgStrokeWidth(ctx, m_selected ? 4.2f : 3.5f);
        nvgStroke(ctx);
    }

    // Label text
    nvgFontSize(ctx, 16.0f);
    nvgFontFace(ctx, "sans-bold");
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    float textAlpha = m_pressed ? 0.76f : 1.0f;
    nvgFillColor(ctx, nvgRGBAf(accentR, accentG, accentB, textAlpha));
    nvgText(ctx, x + w * 0.5f, y + h * 0.5f, m_label.c_str(), nullptr);
}
