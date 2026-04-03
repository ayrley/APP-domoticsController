#ifndef __LOGGING_PAGE_H
#define __LOGGING_PAGE_H

#include <atomic>
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
    void refreshEntries();
    bool matchesFilter(const std::string &message) const;
    void setFilter(enum logFilter filter);

    nanogui::Screen *m_nanoScreen{nullptr};
    nanogui::Widget *m_listPanel{nullptr};
    nanogui::Label *m_hintLabel{nullptr};
    DomeButton *m_allButton{nullptr};
    DomeButton *m_badgeButton{nullptr};
    DomeButton *m_outputButton{nullptr};
    DomeButton *m_tamperButton{nullptr};
    DomeButton *m_clearButton{nullptr};

    enum logFilter m_activeFilter{FILTER_ALL};

    std::thread m_refreshThread;
    std::atomic<bool> m_stopRefreshThread{false};
};

#endif
