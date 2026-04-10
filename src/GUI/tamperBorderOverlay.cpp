#include "tamperBorderOverlay.h"

#include <cmath>

#include <nanogui/common.h>
#include <nanovg.h>

TamperBorderOverlay::TamperBorderOverlay(nanogui::Widget *parent, const bool *detectedFlag)
    : nanogui::Widget(parent),
      m_detectedFlag(detectedFlag),
      m_startTime(std::chrono::steady_clock::now())
{
}

void TamperBorderOverlay::draw(NVGcontext *ctx)
{
    if (m_detectedFlag == nullptr || !(*m_detectedFlag)) {
        return;
    }

    Widget::draw(ctx);

    const auto now = std::chrono::steady_clock::now();
    const double seconds = std::chrono::duration<double>(now - m_startTime).count();
    const float pulse = 0.5f + 0.5f * static_cast<float>(std::sin(seconds * 6.0));

    const int outerAlpha = static_cast<int>(150.0f + pulse * 105.0f);
    const int innerAlpha = static_cast<int>(90.0f + pulse * 100.0f);
    const float outerWidth = 5.0f + pulse * 5.0f;

    const float x = static_cast<float>(m_pos.x());
    const float y = static_cast<float>(m_pos.y());
    const float w = static_cast<float>(m_size.x());
    const float h = static_cast<float>(m_size.y());

    nvgBeginPath(ctx);
    nvgRect(ctx, x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f);
    nvgStrokeWidth(ctx, outerWidth);
    nvgStrokeColor(ctx, nanogui::Color(245, 40, 30, outerAlpha));
    nvgStroke(ctx);

    nvgBeginPath(ctx);
    nvgRect(ctx, x + 8.0f, y + 8.0f, w - 16.0f, h - 16.0f);
    nvgStrokeWidth(ctx, 2.0f);
    nvgStrokeColor(ctx, nanogui::Color(255, 150, 150, innerAlpha));
    nvgStroke(ctx);
}