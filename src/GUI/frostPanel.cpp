#include "frostPanel.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>
#include <nanogui/vscrollpanel.h>
#include <nanovg.h>

namespace
{

int scaledAlpha(int baseAlpha, float opacityPercent)
{
    float clampedPercent = std::max(0.0f, std::min(100.0f, opacityPercent));
    float scale = clampedPercent / 100.0f;
    int alpha = static_cast<int>(std::lround(static_cast<float>(baseAlpha) * scale));
    return std::max(0, std::min(255, alpha));
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

FrostPanel::FrostPanel(nanogui::Widget *parent) :
    nanogui::Widget(parent) {}

void FrostPanel::setOpacityPercent(float percent)
{
    m_opacityPercent = std::max(0.0f, std::min(100.0f, percent));
}

bool FrostPanel::mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
                                    int modifiers)
{
    if (Widget::mouse_button_event(p, button, down, modifiers)) {
        return true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && m_scrollTarget) {
        m_dragScrollActive = down;
        return true;
    }

    return false;
}

bool FrostPanel::mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                                  int button, int modifiers)
{
    if (m_dragScrollActive && m_scrollTarget && (button & (1 << GLFW_MOUSE_BUTTON_LEFT)) != 0) {
        nanogui::Vector2i mapped = mapPointToParentSpace(this, m_scrollTarget, p);
        return m_scrollTarget->mouse_drag_event(mapped, rel, button, modifiers);
    }

    return Widget::mouse_drag_event(p, rel, button, modifiers);
}

bool FrostPanel::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel)
{
    if (Widget::scroll_event(p, rel)) {
        return true;
    }

    if (m_scrollTarget) {
        nanogui::Vector2i mapped = mapPointToParentSpace(this, m_scrollTarget, p);
        return m_scrollTarget->scroll_event(mapped, rel);
    }

    return false;
}

void FrostPanel::perform_layout(NVGcontext *ctx)
{
    const nanogui::Vector2i margin(m_contentMargin, m_contentMargin);
    const nanogui::Vector2i originalSize = m_size;
    m_size = nanogui::Vector2i(
        std::max(0, m_size.x() - 2 * m_contentMargin),
        std::max(0, m_size.y() - 2 * m_contentMargin));

    Widget::perform_layout(ctx);
    m_size = originalSize;

    for (auto *child : children()) {
        child->set_position(child->position() + margin);
    }
}

void FrostPanel::draw(NVGcontext *ctx)
{
    const float x = static_cast<float>(m_pos.x());
    const float y = static_cast<float>(m_pos.y());
    const float w = static_cast<float>(m_size.x());
    const float h = static_cast<float>(m_size.y());

    const int topAlpha = scaledAlpha(112, m_opacityPercent);
    const int bottomAlpha = scaledAlpha(86, m_opacityPercent);
    const int borderAlpha = scaledAlpha(108, m_opacityPercent);

    NVGpaint bg = nvgLinearGradient(
        ctx,
        x,
        y,
        x,
        y + h,
        nvgRGBA(42, 78, 132, topAlpha),
        nvgRGBA(16, 40, 82, bottomAlpha));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, x, y, w, h, 10.0f);
    nvgFillPaint(ctx, bg);
    nvgFill(ctx);

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, 10.0f);
    nvgStrokeColor(ctx, nvgRGBA(170, 220, 255, borderAlpha));
    nvgStrokeWidth(ctx, 1.0f);
    nvgStroke(ctx);

    Widget::draw(ctx);
}
