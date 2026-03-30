#include "frostPanel.h"

#include <algorithm>
#include <cmath>

#include <nanovg.h>

namespace {

int scaled_alpha(int baseAlpha, float opacityPercent) {
	float clampedPercent = std::max(0.0f, std::min(100.0f, opacityPercent));
	float scale = clampedPercent / 100.0f;
	int alpha = static_cast<int>(std::lround(static_cast<float>(baseAlpha) * scale));
	return std::max(0, std::min(255, alpha));
}

} // namespace

FrostPanel::FrostPanel(nanogui::Widget *parent)
    : nanogui::Widget(parent) {}

void FrostPanel::set_opacity_percent(float percent) 
{
	m_opacityPercent = std::max(0.0f, std::min(100.0f, percent));
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

	const int topAlpha = scaled_alpha(112, m_opacityPercent);
	const int bottomAlpha = scaled_alpha(86, m_opacityPercent);
	const int borderAlpha = scaled_alpha(108, m_opacityPercent);

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
