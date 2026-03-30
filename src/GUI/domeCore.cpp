#include "domeCore.h"

#include <cmath>

#include <nanogui/opengl.h>

DomeCore::DomeCore(nanogui::Widget *parent)
    : nanogui::Widget(parent) 
{
}

void DomeCore::draw(NVGcontext *ctx) {
	Widget::draw(ctx);
	const float x = static_cast<float>(m_pos.x());
	const float y = static_cast<float>(m_pos.y());
	const float w = static_cast<float>(m_size.x());
	const float h = static_cast<float>(m_size.y());
	const float cx = x + w * 0.5f;
	const float cy = y + h * 0.5f;
	const float baseR = std::min(w, h) * 0.48f;
	const float now = static_cast<float>(glfwGetTime());
	const float breathe = 0.5f + 0.5f * static_cast<float>(std::sin(now * 1.2f));
	const float r = baseR * (0.965f + breathe * 0.07f);

	// Outer halo
	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, r + 10.0f + breathe * 3.0f);
	nvgStrokeWidth(ctx, 3.0f);
	nvgStrokeColor(ctx, nvgRGBAf(0.0f, 0.9f, 1.0f, 0.18f + breathe * 0.12f));
	nvgStroke(ctx);

	// Main rings
	for (int i = 0; i < 4; ++i) {
		const float ringR = r - i * 18.0f + breathe * (3.5f - i * 0.7f);
		nvgBeginPath(ctx);
		nvgCircle(ctx, cx, cy, ringR);
		nvgStrokeWidth(ctx, i == 0 ? 2.5f : 1.4f);
		nvgStrokeColor(ctx, nvgRGBAf(0.0f, 0.88f, 1.0f, 0.22f + breathe * 0.18f - i * 0.05f));
		nvgStroke(ctx);
	}

	// Sweep arc
	const float t = static_cast<float>(std::fmod(now * 0.7f, 6.28318530718f));
	nvgBeginPath(ctx);
	nvgArc(ctx, cx, cy, r - 8.0f + breathe * 2.0f, t, t + 0.9f, NVG_CW);
	nvgStrokeWidth(ctx, 8.0f);
	nvgStrokeColor(ctx, nvgRGBAf(0.0f, 1.0f, 0.86f, 0.45f + breathe * 0.15f));
	nvgStroke(ctx);

	// Core pulse
	const float pulse = 0.5f + 0.5f * static_cast<float>(std::sin(now * 2.5f));
	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, 12.0f + pulse * 9.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 1.0f, 0.92f, 0.72f));
	nvgFill(ctx);

	nvgFontSize(ctx, 24.0f);
	nvgFontFace(ctx, "sans-bold");
	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.95f, 1.0f, 0.96f));
	nvgText(ctx, cx, cy + r * 0.58f, m_caption.c_str(), nullptr);
}
