#include "overviewPage.h"

#include "metricBlock.h"

#include <nanogui/nanogui.h>

OverviewPage::OverviewPage(nanogui::Widget *parent,
                           std::function<void()> onHome) :
    DefaultPage(parent, std::move(onHome))
{
    setPageTitle("System Overview");
    contentPanel()->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    m_statusLabel = new nanogui::Label(contentPanel(), "Status: Running", "sans-bold");
    m_statusLabel->set_font_size(20);

    m_cpuLabel = createMetricBlock(contentPanel(), "CPU Usage", "collecting...", &m_cpuBar);
    m_cpuLabel->set_font_size(18);

    m_ramLabel = createMetricBlock(contentPanel(), "RAM Usage", "collecting...", &m_ramBar);
    m_ramLabel->set_font_size(18);
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
