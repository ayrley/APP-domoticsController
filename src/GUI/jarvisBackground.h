#ifndef __JARVIS_BACKGROUND_H
#define __JARVIS_BACKGROUND_H

#include <atomic>
#include <chrono>

#include <nanogui/widget.h>

class JarvisBackground : public nanogui::Widget {
public:
	explicit JarvisBackground(nanogui::Widget *parent);
	void setLoadFactor(float loadFactor);
	void draw(NVGcontext *ctx) override;

private:
	std::atomic<float> m_loadFactor;
	std::chrono::steady_clock::time_point m_startTime;
};

#endif
