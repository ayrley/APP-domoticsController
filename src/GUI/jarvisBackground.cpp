#include "jarvisBackground.h"

#include <cmath>

#include <nanogui/opengl.h>

JarvisBackground::JarvisBackground(nanogui::Widget *parent)
	: nanogui::Widget(parent), m_startTime(std::chrono::steady_clock::now()) {}

void JarvisBackground::draw(NVGcontext *ctx) {
	Widget::draw(ctx);

	auto now = std::chrono::steady_clock::now();
	double t = std::chrono::duration<double>(now - m_startTime).count();

	const float x = static_cast<float>(m_pos.x());
	const float y = static_cast<float>(m_pos.y());
	const float w = static_cast<float>(m_size.x());
	const float h = static_cast<float>(m_size.y());

	NVGpaint bg = nvgLinearGradient(
		ctx, x, y, x, y + h,
		nanogui::Color(4, 11, 24, 255),
		nanogui::Color(3, 30, 54, 255));
	nvgBeginPath(ctx);
	nvgRect(ctx, x, y, w, h);
	nvgFillPaint(ctx, bg);
	nvgFill(ctx);

	const float spacing = 34.0f;
	const float offset = static_cast<float>(std::fmod(t * 26.0, spacing));
	for (float gx = x - offset; gx < x + w; gx += spacing) {
		nvgBeginPath(ctx);
		nvgMoveTo(ctx, gx, y);
		nvgLineTo(ctx, gx, y + h);
		nvgStrokeWidth(ctx, 1.0f);
		nvgStrokeColor(ctx, nanogui::Color(70, 160, 200, 22));
		nvgStroke(ctx);
	}
	for (float gy = y - offset; gy < y + h; gy += spacing) {
		nvgBeginPath(ctx);
		nvgMoveTo(ctx, x, gy);
		nvgLineTo(ctx, x + w, gy);
		nvgStrokeWidth(ctx, 1.0f);
		nvgStrokeColor(ctx, nanogui::Color(70, 160, 200, 18));
		nvgStroke(ctx);
	}

	const float cx = x + w * 0.78f;
	const float cy = y + h * 0.36f;
	for (int i = 0; i < 5; ++i) {
		float radius = 38.0f + static_cast<float>(i) * 28.0f + 2.0f * std::sin(t * 1.7 + i);
		nvgBeginPath(ctx);
		nvgCircle(ctx, cx, cy, radius);
		nvgStrokeWidth(ctx, 1.7f);
		nvgStrokeColor(ctx, nanogui::Color(80, 220, 255, 30 + i * 14));
		nvgStroke(ctx);
	}

	float sweepStart = static_cast<float>(t * 1.1);
	float sweepEnd = sweepStart + 0.9f;
	nvgBeginPath(ctx);
	nvgArc(ctx, cx, cy, 112.0f, sweepStart, sweepEnd, NVG_CW);
	nvgStrokeWidth(ctx, 11.0f);
	nvgStrokeColor(ctx, nanogui::Color(30, 255, 180, 130));
	nvgStroke(ctx);

	float pulse = 0.5f + 0.5f * static_cast<float>(std::sin(t * 3.0));
	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, 10.0f + pulse * 8.0f);
	nvgFillColor(ctx, nanogui::Color(65, 255, 220, static_cast<uint8_t>(90 + 90 * pulse)));
	nvgFill(ctx);
}
