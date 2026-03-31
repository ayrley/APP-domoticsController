#include "heatProgressBar.h"

#include <algorithm>
#include <cmath>

#include <nanogui/opengl.h>

HeatProgressBar::HeatProgressBar(nanogui::Widget *parent) :
    nanogui::ProgressBar(parent) {}

void HeatProgressBar::draw(NVGcontext *ctx)
{
    Widget::draw(ctx);

    NVGpaint background = nvgBoxGradient(
        ctx, m_pos.x() + 1, m_pos.y() + 1,
        m_size.x() - 2, m_size.y(), 3, 4, nanogui::Color(0, 32), nanogui::Color(0, 92));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), 3);
    nvgFillPaint(ctx, background);
    nvgFill(ctx);

    float value = std::max(0.0f, std::min(1.0f, m_value));
    int barPos = static_cast<int>(std::round((m_size.x() - 2) * value));
    int red = static_cast<int>(40 + 215.0f * value);
    int green = static_cast<int>(40 + 215.0f * (1.0f - value));

    NVGpaint fill = nvgBoxGradient(
        ctx, m_pos.x(), m_pos.y(),
        barPos + 1.5f, m_size.y() - 1, 3, 4,
        nanogui::Color(red, green, 48, 220),
        nanogui::Color(std::max(0, red - 35), std::max(0, green - 35), 24, 220));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, m_pos.x() + 1, m_pos.y() + 1, barPos, m_size.y() - 2, 3);
    nvgFillPaint(ctx, fill);
    nvgFill(ctx);
}
