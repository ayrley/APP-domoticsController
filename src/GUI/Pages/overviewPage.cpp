#include "overviewPage.h"

#include "metricBlock.h"
#include "../Buttons/domeButton.h"
#include "../domeConfirmDialog.h"

#include <nanogui/nanogui.h>
#include "../../Peripherals/system.h"

OverviewPage::OverviewPage(nanogui::Widget *parent,
                           std::function<void()> onHome) :
    DefaultPage(parent, std::move(onHome))
{
    setPageTitle("System Overview");

    m_rebootButton = new DomeButton(this, "REBOOT", [this]() {
        new DomeConfirmDialog(
            screen(),
            "Reboot System",
            "Are you sure?",
            "The system will restart immediately.",
            "REBOOT",
            "Cancel",
            []() { System::reboot(); },
            nullptr);
    });
    m_rebootButton->setHomeStyle(true);
    m_rebootButton->setPalette(0.72f, 0.10f, 0.10f, 1.0f, 0.28f, 0.28f);
    m_rebootButton->set_fixed_size(nanogui::Vector2i(128, 42));

    contentPanel()->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    m_statusLabel = new nanogui::Label(contentPanel(), "Status: Running", "sans-bold");
    m_statusLabel->set_font_size(20);

    m_cpuLabel = createMetricBlock(contentPanel(), "CPU Usage", "collecting...", &m_cpuBar);
    m_cpuLabel->set_font_size(18);

    m_ramLabel = createMetricBlock(contentPanel(), "RAM Usage", "collecting...", &m_ramBar);
    m_ramLabel->set_font_size(18);

    m_dateLabel = new nanogui::Label(contentPanel(), "Datum: --", "sans");
    m_dateLabel->set_font_size(18);

    m_timeLabel = new nanogui::Label(contentPanel(), "Tijd: --:--", "sans-bold");
    m_timeLabel->set_font_size(24);
}

void OverviewPage::perform_layout(NVGcontext *ctx)
{
    DefaultPage::perform_layout(ctx);
    if (m_rebootButton) {
        constexpr int kMargin = 16;
        nanogui::Vector2i btnSize = m_rebootButton->fixed_size();
        m_rebootButton->set_position(
            nanogui::Vector2i(m_size.x() - btnSize.x() - kMargin,
                              m_size.y() - btnSize.y() - kMargin));
    }
}

void OverviewPage::setStatus(const std::string &status)
{
    if (m_statusLabel) {
        m_statusLabel->set_caption("Status: " + status);
    }
}

void OverviewPage::setCpu(const std::string &cpuCaption, float fraction)
{
    if (m_cpuLabel) {
        m_cpuLabel->set_caption(cpuCaption);
    }
    if (m_cpuBar) {
        m_cpuBar->set_value(fraction);
    }
}

void OverviewPage::setRam(const std::string &ramCaption, float fraction)
{
    if (m_ramLabel) {
        m_ramLabel->set_caption(ramCaption);
    }
    if (m_ramBar) {
        m_ramBar->set_value(fraction);
    }
}

void OverviewPage::setDateTime(const std::string &dateCaption, const std::string &timeCaption)
{
    if (m_dateLabel) {
        m_dateLabel->set_caption("Datum: " + dateCaption);
    }
    if (m_timeLabel) {
        m_timeLabel->set_caption("Tijd: " + timeCaption);
    }
}
