#ifndef __DEFAULT_PAGE_H
#define __DEFAULT_PAGE_H

#include <functional>
#include <string>

#include <nanogui/widget.h>

namespace nanogui
{
class Label;
} // namespace nanogui

class HomeButton;
class NestedScrollPanel;

class DefaultPage : public nanogui::Widget
{
public:
    explicit DefaultPage(nanogui::Widget *parent, std::function<void()> onHome);
    void perform_layout(NVGcontext *ctx) override;

protected:
    void setPageTitle(const std::string &title);
    nanogui::Widget *contentPanel() const { return m_contentPanel; }

private:
    HomeButton *m_homeButton{nullptr};
    nanogui::Label *m_titleLabel{nullptr};
    NestedScrollPanel *m_scrollPanel{nullptr};
    nanogui::Widget *m_contentPanel{nullptr};
};

#endif
