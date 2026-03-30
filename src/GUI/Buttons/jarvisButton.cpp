#include "jarvisButton.h"

#include <cmath>

#include <GLFW/glfw3.h>
#include <nanogui/opengl.h>

JarvisButton::JarvisButton(nanogui::Widget *parent, const std::string &label,
                           std::function<void()> callback)
    : nanogui::Widget(parent), m_label(label), m_callback(std::move(callback)) {}

nanogui::Vector2i JarvisButton::preferred_size_impl(NVGcontext *ctx) const {
	nvgFontSize(ctx, 16.0f);
	nvgFontFace(ctx, "sans-bold");
	float bounds[4];
	nvgTextBounds(ctx, 0, 0, m_label.c_str(), nullptr, bounds);
	int tw = static_cast<int>(bounds[2] - bounds[0]);
	return nanogui::Vector2i(tw + 48, 36);
}

bool JarvisButton::mouse_button_event(const nanogui::Vector2i &p, int button,
                                      bool down, int modifiers) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (down) {
			m_pressed = true;
		} else if (m_pressed) {
			m_pressed = false;
			if (m_callback)
				m_callback();
		}
		return true;
	}
	return nanogui::Widget::mouse_button_event(p, button, down, modifiers);
}

bool JarvisButton::mouse_enter_event(const nanogui::Vector2i &p, bool enter) {
	m_hovered = enter;
	return nanogui::Widget::mouse_enter_event(p, enter);
}

void JarvisButton::draw(NVGcontext *ctx) {
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

	// Semi-transparent background fill
	float bgAlpha = m_pressed ? 0.55f : (m_hovered ? 0.35f : 0.18f);
	nvgBeginPath(ctx);
	if (isRound) {
		nvgCircle(ctx, cx, cy, radius - 1.0f);
	} else {
		nvgRoundedRect(ctx, x, y, w, h, 5.0f);
	}
	nvgFillColor(ctx, nvgRGBAf(baseR, baseG, baseB, bgAlpha));
	nvgFill(ctx);

	// Dim full border
	float borderAlpha = m_hovered ? 1.0f : 0.72f;
	nvgBeginPath(ctx);
	if (isRound) {
		nvgCircle(ctx, cx, cy, radius - 1.5f);
	} else {
		nvgRoundedRect(ctx, x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, 5.0f);
	}
	nvgStrokeColor(ctx, nvgRGBAf(accentR, accentG, accentB, borderAlpha * 0.55f));
	nvgStrokeWidth(ctx, m_homeStyle ? 1.7f : 1.0f);
	nvgStroke(ctx);

	// Accent lines (arc accents for round buttons)
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

	// Outer glow on hover
	if (m_hovered) {
		nvgBeginPath(ctx);
		if (isRound) {
			nvgCircle(ctx, cx, cy, radius + 2.5f);
		} else {
			nvgRoundedRect(ctx, x - 2.5f, y - 2.5f, w + 5.0f, h + 5.0f, 8.0f);
		}
		nvgStrokeColor(ctx, nvgRGBAf(accentR, accentG, accentB, m_homeStyle ? 0.38f : 0.28f));
		nvgStrokeWidth(ctx, 3.5f);
		nvgStroke(ctx);
	}

	// Label text
	nvgFontSize(ctx, 16.0f);
	nvgFontFace(ctx, "sans-bold");
	nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgFillColor(ctx, nvgRGBAf(accentR, accentG, accentB, m_pressed ? 0.76f : 1.0f));
	nvgText(ctx, x + w * 0.5f, y + h * 0.5f, m_label.c_str(), nullptr);
}
