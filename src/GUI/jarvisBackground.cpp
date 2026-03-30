#include "jarvisBackground.h"

#include <algorithm>
#include <cmath>

#include <nanogui/opengl.h>

namespace {

struct Rgba {
	int r;
	int g;
	int b;
	int a;
};

nanogui::Color lerpColor(const Rgba &a, const Rgba &b, float t) {
	t = std::max(0.0f, std::min(1.0f, t));

	int r = static_cast<int>(std::round(a.r + (b.r - a.r) * t));
	int g = static_cast<int>(std::round(a.g + (b.g - a.g) * t));
	int bl = static_cast<int>(std::round(a.b + (b.b - a.b) * t));
	int al = static_cast<int>(std::round(a.a + (b.a - a.a) * t));

	return nanogui::Color(r, g, bl, al);
}

} // namespace

JarvisBackground::JarvisBackground(nanogui::Widget *parent)
	: nanogui::Widget(parent),
	  m_loadFactor(0.0f),
	  m_startTime(std::chrono::steady_clock::now()) {}

void JarvisBackground::setLoadFactor(float loadFactor) {
	m_loadFactor.store(std::max(0.0f, std::min(1.0f, loadFactor)));
}

void JarvisBackground::draw(NVGcontext *ctx) {
	Widget::draw(ctx);

	auto now = std::chrono::steady_clock::now();
	double t = std::chrono::duration<double>(now - m_startTime).count();

	const float x = static_cast<float>(m_pos.x());
	const float y = static_cast<float>(m_pos.y());
	const float w = static_cast<float>(m_size.x());
	const float h = static_cast<float>(m_size.y());
	const float load = m_loadFactor.load();

	Rgba topLow{4, 11, 24, 255};
	Rgba topHigh{40, 10, 8, 255};
	Rgba bottomLow{3, 30, 54, 255};
	Rgba bottomHigh{86, 26, 12, 255};
	Rgba gridLow{70, 160, 200, 22};
	Rgba gridHigh{255, 128, 80, 36};
	Rgba ringLow{80, 220, 255, 64};
	Rgba ringHigh{255, 140, 70, 96};
	Rgba sweepLow{30, 255, 180, 130};
	Rgba sweepHigh{255, 120, 40, 170};
	Rgba coreLow{65, 255, 220, 180};
	Rgba coreHigh{255, 130, 70, 220};

	NVGpaint bg = nvgLinearGradient(
		ctx, x, y, x, y + h,
		lerpColor(topLow, topHigh, load),
		lerpColor(bottomLow, bottomHigh, load));
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
		nvgStrokeColor(ctx, lerpColor(gridLow, gridHigh, load));
		nvgStroke(ctx);
	}
	for (float gy = y - offset; gy < y + h; gy += spacing) {
		nvgBeginPath(ctx);
		nvgMoveTo(ctx, x, gy);
		nvgLineTo(ctx, x + w, gy);
		nvgStrokeWidth(ctx, 1.0f);
		nvgStrokeColor(ctx, lerpColor(gridLow, gridHigh, load * 0.85f));
		nvgStroke(ctx);
	}

	const float cx = x + w * 0.78f;
	const float cy = y + h * 0.36f;
	for (int i = 0; i < 5; ++i) {
		float radius = 38.0f + static_cast<float>(i) * 28.0f + 2.0f * std::sin(t * 1.7 + i);
		nvgBeginPath(ctx);
		nvgCircle(ctx, cx, cy, radius);
		nvgStrokeWidth(ctx, 1.7f);
		nvgStrokeColor(ctx, lerpColor(ringLow, ringHigh, load));
		nvgStroke(ctx);
	}

	float sweepStart = static_cast<float>(t * 1.1);
	float sweepEnd = sweepStart + 0.9f;
	nvgBeginPath(ctx);
	nvgArc(ctx, cx, cy, 112.0f, sweepStart, sweepEnd, NVG_CW);
	nvgStrokeWidth(ctx, 11.0f);
	nvgStrokeColor(ctx, lerpColor(sweepLow, sweepHigh, load));
	nvgStroke(ctx);

	float pulse = 0.5f + 0.5f * static_cast<float>(std::sin(t * 3.0));
	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, 10.0f + pulse * 8.0f);
	nvgFillColor(ctx, lerpColor(coreLow, coreHigh, load));
	nvgFill(ctx);
}
