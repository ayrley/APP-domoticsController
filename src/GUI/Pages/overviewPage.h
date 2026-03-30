#ifndef __OVERVIEW_PAGE_H
#define __OVERVIEW_PAGE_H

#include <functional>
#include <string>

#include "defaultPage.h"

namespace nanogui {
class Label;
class ProgressBar;
}

class OverviewPage : public DefaultPage {
public:
	explicit OverviewPage(nanogui::Widget *parent,
	                      std::function<void()> onHome);

	void setStatus(const std::string &status);
	void setCpu(const std::string &cpuCaption, float fraction);
	void setRam(const std::string &ramCaption, float fraction);

private:
	nanogui::Label *m_statusLabel{nullptr};
	nanogui::Label *m_cpuLabel{nullptr};
	nanogui::Label *m_ramLabel{nullptr};
	nanogui::ProgressBar *m_cpuBar{nullptr};
	nanogui::ProgressBar *m_ramBar{nullptr};
};

#endif
