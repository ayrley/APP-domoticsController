#ifndef __OVERVIEW_PAGE_H
#define __OVERVIEW_PAGE_H

#include <functional>
#include <string>

#include "defaultPage.h"
#include "../Buttons/domeButton.h"

namespace nanogui
{
class Label;
class ProgressBar;
} // namespace nanogui

class OverviewPage : public DefaultPage
{
public:
    explicit OverviewPage(nanogui::Widget *parent,
                          std::function<void()> onHome);

    void setStatus(const std::string &status);
    void setCpu(const std::string &cpuCaption, float fraction);
    void setRam(const std::string &ramCaption, float fraction);
    void setDateTime(const std::string &dateCaption, const std::string &timeCaption);
    void perform_layout(NVGcontext *ctx) override;

private:
    nanogui::Label *m_statusLabel{nullptr};
    nanogui::Label *m_cpuLabel{nullptr};
    nanogui::Label *m_ramLabel{nullptr};
    nanogui::Label *m_dateLabel{nullptr};
    nanogui::Label *m_timeLabel{nullptr};
    nanogui::ProgressBar *m_cpuBar{nullptr};
    nanogui::ProgressBar *m_ramBar{nullptr};
    DomeButton *m_rebootButton{nullptr};    
};

#endif
