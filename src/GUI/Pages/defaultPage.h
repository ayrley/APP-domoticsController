#ifndef __DEFAULT_PAGE_H
#define __DEFAULT_PAGE_H

#include <functional>

#include <nanogui/widget.h>

class HomeButton;

class DefaultPage : public nanogui::Widget {
public:
	explicit DefaultPage(nanogui::Widget *parent, std::function<void()> onHome);
	void perform_layout(NVGcontext *ctx) override;

protected:
	nanogui::Widget *contentPanel() const { return m_contentPanel; }

private:
	HomeButton *m_homeButton{nullptr};
	nanogui::Widget *m_contentPanel{nullptr};
};

#endif