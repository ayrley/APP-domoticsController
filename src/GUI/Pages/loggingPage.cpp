#include "loggingPage.h"

#include "domeButton.h"
#include "eventLog.h"

#include <chrono>
#include <cstdio>
#include <cstdint>

#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/screen.h>
#include <nanogui/vscrollpanel.h>

namespace
{
std::uint64_t hashEventEntries(const std::vector<EventLogEntry> &entries,
                               const int activeFilter)
{
    constexpr std::uint64_t fnvOffsetBasis = 1469598103934665603ULL;
    constexpr std::uint64_t fnvPrime = 1099511628211ULL;

    std::uint64_t hash = fnvOffsetBasis;

    auto hashString = [&](const std::string &value) {
        for (const unsigned char ch : value) {
            hash ^= static_cast<std::uint64_t>(ch);
            hash *= fnvPrime;
        }
        hash ^= 0xFF;
        hash *= fnvPrime;
    };

    hash ^= static_cast<std::uint64_t>(activeFilter);
    hash *= fnvPrime;

    for (const auto &entry : entries) {
        hashString(entry.timestamp);
        hashString(entry.message);
    }

    return hash;
}
}

LoggingPage::LoggingPage(nanogui::Widget *parent,
                         std::function<void()> onHome) :
    DefaultPage(parent, std::move(onHome)),
    m_nanoScreen(dynamic_cast<nanogui::Screen *>(parent))
{
    setPageTitle("Event Log");

    contentPanel()->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    m_hintLabel = new nanogui::Label(
        contentPanel(),
        "Recent events: badge reads, output toggles, and tamper state.",
        "sans");
    m_hintLabel->set_font_size(16);
    m_hintLabel->set_color(nanogui::Color(150, 200, 220, 255));

    auto *toolbar = new nanogui::Widget(contentPanel());
    toolbar->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 8));
    toolbar->set_fixed_height(42);

    m_allButton = new DomeButton(toolbar, "All", [this]() { setFilter(FILTER_ALL); });
    m_badgeButton = new DomeButton(toolbar, "Badge", [this]() { setFilter(FILTER_BADGE); });
    m_outputButton = new DomeButton(toolbar, "Output", [this]() { setFilter(FILTER_OUTPUT); });
    m_tamperButton = new DomeButton(toolbar, "Tamper", [this]() { setFilter(FILTER_TAMPER); });
    m_clearButton = new DomeButton(toolbar, "Clear", [this]() {
        EventLog::clear();
        refreshEntries(true);
    });

    m_allButton->set_fixed_size(nanogui::Vector2i(96, 34));
    m_badgeButton->set_fixed_size(nanogui::Vector2i(96, 34));
    m_outputButton->set_fixed_size(nanogui::Vector2i(96, 34));
    m_tamperButton->set_fixed_size(nanogui::Vector2i(96, 34));
    m_clearButton->set_fixed_size(nanogui::Vector2i(120, 34));
    m_clearButton->setPalette(0.72f, 0.10f, 0.10f, 1.0f, 0.28f, 0.28f);

    setFilter(FILTER_ALL);

    auto *scrollPanel = new nanogui::VScrollPanel(contentPanel());
    scrollPanel->set_fixed_size(nanogui::Vector2i(980, 460));

    m_listPanel = new nanogui::Widget(scrollPanel);
    m_listPanel->set_layout(new nanogui::GroupLayout(0, 4, 8, 0));

    refreshEntries(true);
    startRefreshLoop();
}

LoggingPage::~LoggingPage()
{
    stopRefreshLoop();
}

void LoggingPage::startRefreshLoop()
{
    m_stopRefreshThread = false;

    m_refreshThread = std::thread([this]() {
        while (!m_stopRefreshThread.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(750));

            if (m_stopRefreshThread.load()) {
                break;
            }

            if (m_nanoScreen) {
                nanogui::async([this]() {
                    refreshEntries(false);
                });
            }
        }
    });
}

void LoggingPage::stopRefreshLoop()
{
    m_stopRefreshThread = true;

    if (m_refreshThread.joinable()) {
        m_refreshThread.join();
    }
}

void LoggingPage::refreshEntries(bool force)
{
    ++m_refreshEvaluated;

    if (!m_listPanel) {
        return;
    }

    if (!force && !visible()) {
        ++m_refreshSkippedHidden;
        maybeReportRefreshStats();
        return;
    }

    const auto entries = EventLog::getRecent(300);
    const std::uint64_t modelHash = hashEventEntries(entries, static_cast<int>(m_activeFilter));
    if (!force && modelHash == m_lastModelHash) {
        ++m_refreshSkippedUnchanged;
        maybeReportRefreshStats();
        return;
    }
    m_lastModelHash = modelHash;
    ++m_refreshRendered;

    while (!m_listPanel->children().empty()) {
        auto *child = m_listPanel->children().back();
        m_listPanel->remove_child(child);
    }

    std::size_t shown = 0;

    if (entries.empty()) {
        auto *emptyLabel = new nanogui::Label(m_listPanel, "No events yet.", "sans");
        emptyLabel->set_font_size(15);
        emptyLabel->set_color(nanogui::Color(150, 180, 200, 255));
    } else {
        for (const auto &entry : entries) {
            if (!matchesFilter(entry.message)) {
                continue;
            }

            auto *line = new nanogui::Label(
                m_listPanel,
                entry.timestamp + "  " + entry.message,
                "sans");
            line->set_font_size(15);
            line->set_color(nanogui::Color(220, 232, 240, 255));
            ++shown;
        }

        if (shown == 0) {
            auto *emptyFilterLabel = new nanogui::Label(m_listPanel, "No events for this filter.", "sans");
            emptyFilterLabel->set_font_size(15);
            emptyFilterLabel->set_color(nanogui::Color(150, 180, 200, 255));
        }
    }

    if (m_nanoScreen) {
        m_nanoScreen->perform_layout();
        m_nanoScreen->redraw();
    }

    maybeReportRefreshStats();
}

bool LoggingPage::matchesFilter(const std::string &message) const
{
    switch (m_activeFilter) {
    case FILTER_BADGE:
        return message.find("[BADGE]") == 0;
    case FILTER_OUTPUT:
        return message.find("[OUTPUT]") == 0;
    case FILTER_TAMPER:
        return message.find("[TAMPER]") == 0;
    case FILTER_ALL:
    default:
        return true;
    }
}

void LoggingPage::setFilter(enum logFilter filter)
{
    m_activeFilter = filter;

    if (m_allButton) {
        m_allButton->setSelected(filter == FILTER_ALL);
    }
    if (m_badgeButton) {
        m_badgeButton->setSelected(filter == FILTER_BADGE);
    }
    if (m_outputButton) {
        m_outputButton->setSelected(filter == FILTER_OUTPUT);
    }
    if (m_tamperButton) {
        m_tamperButton->setSelected(filter == FILTER_TAMPER);
    }

    refreshEntries(true);
}

void LoggingPage::maybeReportRefreshStats()
{
#if defined(DEBUG)
    constexpr std::uint64_t kReportEvery = 40;
    if (m_refreshEvaluated == 0 || (m_refreshEvaluated % kReportEvery) != 0) {
        return;
    }

    std::fprintf(
        stderr,
        "[GUI][LoggingPage] refresh stats: eval=%llu rendered=%llu skipped_hidden=%llu skipped_unchanged=%llu\n",
        static_cast<unsigned long long>(m_refreshEvaluated),
        static_cast<unsigned long long>(m_refreshRendered),
        static_cast<unsigned long long>(m_refreshSkippedHidden),
        static_cast<unsigned long long>(m_refreshSkippedUnchanged));
#endif
}
