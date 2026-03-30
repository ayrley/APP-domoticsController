#include "domeConfirmDialog.h"

#include "domeButton.h"

#include <nanogui/nanogui.h>
#include <nanogui/screen.h>
#include <nanogui/theme.h>
#include <nanovg.h>

DomeConfirmDialog::DomeConfirmDialog(nanogui::Widget *parent,
                                     const std::string &headline,
                                     const std::string &context,
                                     const std::string &message,
                                     const std::string &confirmLabel,
                                     const std::string &cancelLabel,
                                     std::function<void()> onConfirm,
                                     std::function<void()> onCancel)
	: nanogui::Window(parent, "") {
	set_modal(true);
	set_layout(new nanogui::GroupLayout(12, 8, 16, 12));
	set_fixed_width(460);

	auto *headlineLabel = new nanogui::Label(this, headline, "sans-bold");
	headlineLabel->set_font_size(21);
	headlineLabel->set_color(nanogui::Color(0, 230, 255, 255));

	if (!context.empty()) {
		auto *contextLabel = new nanogui::Label(this, context, "sans-bold");
		contextLabel->set_font_size(18);
		contextLabel->set_color(nanogui::Color(255, 180, 70, 255));
	}

	auto *messageLabel = new nanogui::Label(this, message, "sans");
	messageLabel->set_font_size(16);
	messageLabel->set_color(nanogui::Color(255, 120, 120, 255));

	auto *buttonRow = new nanogui::Widget(this);
	buttonRow->set_layout(new nanogui::BoxLayout(
	    nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 0));

	auto *leftSpacer = new nanogui::Widget(buttonRow);
	leftSpacer->set_fixed_size(nanogui::Vector2i(48, 1));

	auto *cancelButton = new DomeButton(buttonRow, cancelLabel, [this, onCancel]() {
		dispose();
		if (onCancel) {
			onCancel();
		}
	});
	cancelButton->set_fixed_size(nanogui::Vector2i(150, 38));

	auto *middleSpacer = new nanogui::Widget(buttonRow);
	middleSpacer->set_fixed_size(nanogui::Vector2i(12, 1));

	auto *confirmButton = new DomeButton(buttonRow, confirmLabel, [this, onConfirm]() {
		dispose();
		if (onConfirm) {
			onConfirm();
		}
	});
	confirmButton->set_fixed_size(nanogui::Vector2i(170, 38));
	confirmButton->set_home_style(true);

	auto *rightSpacer = new nanogui::Widget(buttonRow);
	rightSpacer->set_fixed_size(nanogui::Vector2i(48, 1));

	if (auto *nanoScreen = screen()) {
		nanoScreen->perform_layout();
	}
	center();
}

void DomeConfirmDialog::draw(NVGcontext *ctx) {
	int ds = m_theme->m_window_drop_shadow_size;
	int cr = m_theme->m_window_corner_radius;

	nvgSave(ctx);
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);

	NVGpaint body = nvgLinearGradient(
		ctx,
		m_pos.x(),
		m_pos.y(),
		m_pos.x(),
		m_pos.y() + m_size.y(),
		nvgRGBA(2, 8, 20, 225),
		nvgRGBA(3, 18, 38, 205));
	nvgFillPaint(ctx, body);
	nvgFill(ctx);

	NVGpaint shadow_paint = nvgBoxGradient(
		ctx,
		m_pos.x(),
		m_pos.y(),
		m_size.x(),
		m_size.y(),
		cr * 2,
		ds * 2,
		m_theme->m_drop_shadow,
		m_theme->m_transparent);

	nvgSave(ctx);
	nvgResetScissor(ctx);
	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x() - ds, m_pos.y() - ds, m_size.x() + 2 * ds, m_size.y() + 2 * ds);
	nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, shadow_paint);
	nvgFill(ctx);
	nvgRestore(ctx);

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, m_pos.x() + 0.5f, m_pos.y() + 0.5f, m_size.x() - 1.0f, m_size.y() - 1.0f, cr);
	nvgStrokeColor(ctx, nvgRGBA(0, 195, 230, 80));
	nvgStrokeWidth(ctx, 1.0f);
	nvgStroke(ctx);

	nvgRestore(ctx);
	nanogui::Widget::draw(ctx);
}
