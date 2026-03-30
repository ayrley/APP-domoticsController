#ifndef __HEAT_PROGRESS_BAR_H
#define __HEAT_PROGRESS_BAR_H

#include <nanogui/progressbar.h>

class HeatProgressBar : public nanogui::ProgressBar {
public:
	explicit HeatProgressBar(nanogui::Widget *parent);
	void draw(NVGcontext *ctx) override;
};

#endif
