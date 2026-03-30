#ifndef __JARVIS_BACKGROUND_H
#define __JARVIS_BACKGROUND_H

#include <chrono>

#include <nanogui/widget.h>

class JarvisBackground : public nanogui::Widget {
public:
	explicit JarvisBackground(nanogui::Widget *parent);
	void draw(NVGcontext *ctx) override;

private:
	std::chrono::steady_clock::time_point m_startTime;
};

#endif
