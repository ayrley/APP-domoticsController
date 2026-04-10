#include "defaultPage.h"

#include "homeButton.h"
#include "nestedScrollPanel.h"

#include <algorithm>

#include <nanogui/label.h>

#include "debug.h"

DefaultPage::DefaultPage(nanogui::Widget *parent, std::function<void()> onHome) :
    nanogui::Widget(parent)
{
    set_layout(nullptr);

    m_homeButton = new HomeButton(this, [onHome]() {
        if (onHome) {
            onHome();
        }
    });

    m_titleLabel = new nanogui::Label(this, "", "sans-bold");
    m_titleLabel->set_font_size(30);
    m_titleLabel->set_color(nanogui::Color(0, 235, 255, 255));

    m_scrollPanel = new NestedScrollPanel(this);
    m_contentPanel = new nanogui::Widget(m_scrollPanel);
}

void DefaultPage::setPageTitle(const std::string &title)
{
    if (m_titleLabel) {
        m_titleLabel->set_caption(title);
    }
}

void DefaultPage::setActivityCallback(std::function<void()> onActivity)
{
    m_onActivity = std::move(onActivity);
}

bool DefaultPage::mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
                                     int modifiers)
{
    if (down && m_onActivity) {
        m_onActivity();
    }
    return Widget::mouse_button_event(p, button, down, modifiers);
}

bool DefaultPage::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel)
{
    if (m_scrollPanel && m_scrollPanel->contains(p) && m_scrollPanel->scroll_event(p, rel)) {
        if (m_onActivity) {
            m_onActivity();
        }
        return true;
    }

    return Widget::scroll_event(p, rel);
}

bool DefaultPage::mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                                   int button, int modifiers)
{
    if (Widget::mouse_drag_event(p, rel, button, modifiers)) {
        return true;
    }

    if (!m_scrollPanel || !m_scrollPanel->contains(p)) {
        return false;
    }

    if (m_contentPanel->size().y() <= m_scrollPanel->size().y()) {
        return false;
    }

    // Touch-like drag scrolling: dragging down reveals earlier content.
    m_scrollPanel->scroll_absolute(static_cast<float>(-rel.y()) * 3.0f);
    return true;
}

void DefaultPage::perform_layout(NVGcontext *ctx)
{
    constexpr int kOuterMargin = 16;
    constexpr int kTitleY = 16;
    constexpr int kTopOffset = 58;
    constexpr int kBottomMargin = 14;
    constexpr int kHomePadding = 14;
    constexpr int kPageHeightScrollThreshold = 600;

    if (m_homeButton) {
        nanogui::Vector2i homeSize = m_homeButton->fixed_size();
        m_homeButton->set_position(
            nanogui::Vector2i(m_size.x() - homeSize.x() - kHomePadding, kHomePadding));
    }

    if (m_titleLabel) {
        m_titleLabel->set_position(nanogui::Vector2i(kOuterMargin, kTitleY));
    }

    if (m_scrollPanel && m_contentPanel) {
        int viewportWidth = std::max(0, m_size.x() - (2 * kOuterMargin));
        int availableHeight = std::max(0, m_size.y() - kTopOffset - kBottomMargin);

        int viewportHeight = availableHeight;
        if (m_size.y() > kPageHeightScrollThreshold) {
            viewportHeight = std::min(
                availableHeight,
                kPageHeightScrollThreshold - kTopOffset - kBottomMargin);
        }

        m_scrollPanel->set_position(nanogui::Vector2i(kOuterMargin, kTopOffset));
        m_scrollPanel->set_fixed_size(nanogui::Vector2i(viewportWidth, viewportHeight));

        int contentHeight = std::max(availableHeight, m_contentPanel->preferred_size(ctx).y());
        m_contentPanel->set_fixed_size(nanogui::Vector2i(viewportWidth, contentHeight));
    }

    Widget::perform_layout(ctx);
}
