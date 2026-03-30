#ifndef __MANUAL_CONTROL_PAGE_H
#define __MANUAL_CONTROL_PAGE_H

#include <functional>

#include "defaultPage.h"

namespace nanogui {
class Label;
}

class ManualControlPage : public DefaultPage {
public:
	explicit ManualControlPage(nanogui::Widget *parent, std::function<void()> onHome);

private:
	nanogui::Label *m_statusLabel{nullptr};
};

#endif
