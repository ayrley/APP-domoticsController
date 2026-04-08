#ifndef __LOGGING_PAGE_H
#define __LOGGING_PAGE_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

#include "defaultPage.h"

namespace nanogui
{
class Label;
class Screen;
class Widget;
} // namespace nanogui

class DomeButton;

class LoggingPage : public DefaultPage
{
public:
    explicit LoggingPage(nanogui::Widget *parent,
                         std::function<void()> onHome);
    ~LoggingPage();

private:
    enum logFilter {
        FILTER_ALL = 0,
        FILTER_BADGE,
        FILTER_OUTPUT,
        FILTER_TAMPER
    };

    void startRefreshLoop();
    void stopRefreshLoop();
    void refreshEntries(bool force = false);
    bool matchesFilter(const std::string &message) const;
    void setFilter(enum logFilter filter);
    void maybeReportRefreshStats();

    nanogui::Screen *m_nanoScreen{nullptr};
    nanogui::Widget *m_listPanel{nullptr};
    nanogui::Label *m_hintLabel{nullptr};
    DomeButton *m_allButton{nullptr};
    DomeButton *m_badgeButton{nullptr};
    DomeButton *m_outputButton{nullptr};
    DomeButton *m_tamperButton{nullptr};
    DomeButton *m_clearButton{nullptr};

    enum logFilter m_activeFilter{FILTER_ALL};
    std::uint64_t m_lastModelHash{0};
    std::uint64_t m_refreshEvaluated{0};
    std::uint64_t m_refreshSkippedHidden{0};
    std::uint64_t m_refreshSkippedUnchanged{0};
    std::uint64_t m_refreshRendered{0};

    std::thread m_refreshThread;
    std::atomic<bool> m_stopRefreshThread{false};
};

#endif
