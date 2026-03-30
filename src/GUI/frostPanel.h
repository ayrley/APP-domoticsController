#ifndef __FROST_PANEL_H
#define __FROST_PANEL_H

#include <nanogui/widget.h>

class FrostPanel : public nanogui::Widget {
public:
	explicit FrostPanel(nanogui::Widget *parent);
	void set_opacity_percent(float percent);
	float opacity_percent() const { return m_opacityPercent; }
	void perform_layout(NVGcontext *ctx) override;
	void draw(NVGcontext *ctx) override;

private:
	float m_opacityPercent{0.0f};
	int m_contentMargin{16};
};

#endif
