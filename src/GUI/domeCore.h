#ifndef __DOME_CORE_H
#define __DOME_CORE_H

#include <string>

#include <nanogui/widget.h>

class DomeCore : public nanogui::Widget {
public:
	explicit DomeCore(nanogui::Widget *parent);
	void draw(NVGcontext *ctx) override;

private:
	std::string m_caption;
};

#endif