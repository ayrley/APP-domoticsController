#ifndef __JARVIS_CORE_H
#define __JARVIS_CORE_H

#include <string>

#include <nanogui/widget.h>

class JarvisCore : public nanogui::Widget {
public:
	explicit JarvisCore(nanogui::Widget *parent, const std::string &caption = "JARVIS");
	void draw(NVGcontext *ctx) override;

private:
	std::string m_caption;
};

#endif
