#include "manualControlPage.h"

#include "domeButton.h"
#include "frostPanel.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include <nanogui/nanogui.h>
#include <nanogui/screen.h>

#include <nanovg.h>

#include "../Controller/io.h"

namespace
{

class StateIndicatorBar : public nanogui::Widget
{
public:
    explicit StateIndicatorBar(nanogui::Widget *parent) :
        nanogui::Widget(parent)
    {
    }

    void setState(const std::string &caption,
                  const nanogui::Color &textColor,
                  const nanogui::Color &fillTop,
                  const nanogui::Color &fillBottom,
                  const nanogui::Color &borderColor)
    {
        m_caption = caption;
        m_textColor = textColor;
        m_fillTop = fillTop;
        m_fillBottom = fillBottom;
        m_borderColor = borderColor;
    }

    void draw(NVGcontext *ctx) override
    {
        const float x = static_cast<float>(m_pos.x());
        const float y = static_cast<float>(m_pos.y());
        const float w = static_cast<float>(m_size.x());
        const float h = static_cast<float>(m_size.y());

        NVGpaint bg = nvgLinearGradient(
            ctx,
            x,
            y,
            x,
            y + h,
            m_fillTop,
            m_fillBottom);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, x, y, w, h, 8.0f);
        nvgFillPaint(ctx, bg);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, 8.0f);
        nvgStrokeColor(ctx, m_borderColor);
        nvgStrokeWidth(ctx, 1.0f);
        nvgStroke(ctx);

        nvgFontSize(ctx, 15.0f);
        nvgFontFace(ctx, "sans-bold");
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFillColor(ctx, m_textColor);
        nvgText(ctx, x + w * 0.5f, y + h * 0.5f, m_caption.c_str(), nullptr);

        Widget::draw(ctx);
    }

private:
    std::string m_caption{"UNKNOWN"};
    nanogui::Color m_textColor{220, 230, 240, 255};
    nanogui::Color m_fillTop{50, 74, 104, 255};
    nanogui::Color m_fillBottom{28, 42, 64, 255};
    nanogui::Color m_borderColor{120, 150, 190, 255};
};

StateIndicatorBar *stateIndicator(nanogui::Widget *widget)
{
    return static_cast<StateIndicatorBar *>(widget);
}

} // namespace

extern std::vector<IO *> *g_ios;

ManualControlPage::ManualControlPage(nanogui::Widget *parent, std::function<void()> onHome) :
    DefaultPage(parent, std::move(onHome)),
    m_nanoScreen(dynamic_cast<nanogui::Screen *>(parent))
{
    setPageTitle("MANUAL CONTROL");
    contentPanel()->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 0, 18));

    // Left column: IO list
    auto *leftColumn = new FrostPanel(contentPanel());
    leftColumn->set_fixed_width(320);
    leftColumn->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    auto *listHeader = new nanogui::Label(leftColumn, "Output IOs", "sans-bold");
    listHeader->set_font_size(17);
    listHeader->set_color(nanogui::Color(0, 200, 220, 255));

    auto *listScroll = new nanogui::VScrollPanel(leftColumn);
    listScroll->set_fixed_size(nanogui::Vector2i(306, 420));
    leftColumn->setScrollTarget(listScroll);

    m_ioListPanel = new nanogui::Widget(listScroll);
    m_ioListPanel->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 8));

    // Right column: IO control form
    auto *rightColumn = new FrostPanel(contentPanel());
    rightColumn->setOpacityPercent(80.0f);
    rightColumn->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    auto *formHeader = new nanogui::Label(rightColumn, "Control IO", "sans-bold");
    formHeader->set_font_size(17);
    formHeader->set_color(nanogui::Color(0, 200, 220, 255));

    m_statusLabel = new nanogui::Label(rightColumn, " ", "sans");
    m_statusLabel->set_font_size(15);
    m_statusLabel->set_color(nanogui::Color(160, 220, 160, 255));

    // Location (read-only)
    new nanogui::Label(rightColumn, "Location (Unique ID):", "sans");
    m_locationLabel = new nanogui::Label(rightColumn, "(none selected)", "sans-bold");
    m_locationLabel->set_font_size(14);
    m_locationLabel->set_color(nanogui::Color(100, 180, 200, 255));

    // Name field
    new nanogui::Label(rightColumn, "Name:", "sans");
    m_nameBox = new nanogui::TextBox(rightColumn, "");
    m_nameBox->set_editable(true);
    m_nameBox->set_placeholder("IO name");
    m_nameBox->set_fixed_size(nanogui::Vector2i(460, 30));
    m_nameBox->set_font_size(16);
    m_nameBox->set_alignment(nanogui::TextBox::Alignment::Left);

    // Duration field
    new nanogui::Label(rightColumn, "Duration (ms):", "sans");
    m_durationBox = new nanogui::TextBox(rightColumn, "");
    m_durationBox->set_editable(true);
    m_durationBox->set_placeholder("Duration in milliseconds");
    m_durationBox->set_fixed_size(nanogui::Vector2i(460, 30));
    m_durationBox->set_font_size(16);
    m_durationBox->set_alignment(nanogui::TextBox::Alignment::Left);

    // State indicator
    new nanogui::Label(rightColumn, "State:", "sans");
    m_stateIndicator = new StateIndicatorBar(rightColumn);
    m_stateIndicator->set_fixed_size(nanogui::Vector2i(460, 32));
    stateIndicator(m_stateIndicator)->setState(
        "UNKNOWN",
        nanogui::Color(210, 220, 230, 255),
        nanogui::Color(78, 88, 98, 255),
        nanogui::Color(52, 60, 70, 255),
        nanogui::Color(140, 150, 160, 255));

    // Control buttons
    auto *buttonRow = new nanogui::Widget(rightColumn);
    buttonRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
    buttonRow->set_fixed_height(44);

    m_setButton = new DomeButton(buttonRow, "Set HIGH", [this]() { triggerIO(true); });
    m_setButton->set_fixed_size(nanogui::Vector2i(150, 40));

    m_clearButton = new DomeButton(buttonRow, "Set LOW", [this]() { triggerIO(false); });
    m_clearButton->set_fixed_size(nanogui::Vector2i(150, 40));

    // Force buttons
    auto *forceRow = new nanogui::Widget(rightColumn);
    forceRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
    forceRow->set_fixed_height(44);

    auto *forceHighButton = new DomeButton(forceRow, "Force HIGH", [this]() { forceIO(true); });
    forceHighButton->set_fixed_size(nanogui::Vector2i(150, 40));

    auto *forceLowButton = new DomeButton(forceRow, "Force LOW", [this]() { forceIO(false); });
    forceLowButton->set_fixed_size(nanogui::Vector2i(150, 40));

    m_forceButton = new DomeButton(forceRow, "Release", [this]() { releaseForcedIO(); });
    m_forceButton->set_fixed_size(nanogui::Vector2i(150, 40));

    buildIOList();
}

void ManualControlPage::buildIOList()
{
    for (auto *btn : m_ioButtons) {
        m_ioListPanel->remove_child(btn);
    }
    m_ioButtons.clear();
    m_outputIOs.clear();
    m_currentIOIndex = -1;

    if (!g_ios || g_ios->empty()) {
        m_statusLabel->set_caption("No output IOs configured.");
        return;
    }

    // Filter for output IOs (IO_DIR_OUT)
    for (IO *io : *g_ios) {
        if (io && io->getDirection() == IO_DIR_OUT) {
            m_outputIOs.push_back(io);
        }
    }

    if (m_outputIOs.empty()) {
        m_statusLabel->set_caption("No output IOs available.");
        return;
    }

    for (IO *io : m_outputIOs) {
        std::string ioName = io->getName();
        if (ioName.empty())
            ioName = io->getLocation();
        if (ioName.empty())
            ioName = "(unnamed)";

        auto *btn = new DomeButton(m_ioListPanel, ioName, [this, io]() {
            populateForm(io);
        });
        btn->set_fixed_size(nanogui::Vector2i(286, 34));
        btn->setSelected(io == m_selectedIO);
        m_ioButtons.push_back(btn);
    }

    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void ManualControlPage::refreshIOSelection()
{
    for (size_t i = 0; i < m_ioButtons.size() && i < m_outputIOs.size(); ++i) {
        if (m_ioButtons[i]) {
            m_ioButtons[i]->setSelected(m_outputIOs[i] == m_selectedIO);
        }
    }

    if (m_nanoScreen) {
        m_nanoScreen->redraw();
    }
}

void ManualControlPage::populateForm(IO *io)
{
    if (!io) {
        clearForm();
        return;
    }

    m_selectedIO = io;
    
    // Find index of this IO in m_outputIOs
    m_currentIOIndex = -1;
    for (size_t i = 0; i < m_outputIOs.size(); ++i) {
        if (m_outputIOs[i] == io) {
            m_currentIOIndex = i;
            break;
        }
    }
    
    std::string location = io->getLocation();
    std::string name = io->getName();

    m_locationLabel->set_caption(location);
    m_nameBox->set_value(name);
    m_durationBox->set_value("1000"); // Default to 1 second
    stateIndicator(m_stateIndicator)->setState(
        "UNKNOWN",
        nanogui::Color(210, 220, 230, 255),
        nanogui::Color(78, 88, 98, 255),
        nanogui::Color(52, 60, 70, 255),
        nanogui::Color(140, 150, 160, 255));
    m_statusLabel->set_caption("Selected: " + (name.empty() ? location : name));
    refreshIOSelection();

    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void ManualControlPage::clearForm()
{
    m_selectedIO = nullptr;
    m_currentIOIndex = -1;
    m_locationLabel->set_caption("(none selected)");
    m_nameBox->set_value("");
    m_durationBox->set_value("");
    stateIndicator(m_stateIndicator)->setState(
        "UNKNOWN",
        nanogui::Color(210, 220, 230, 255),
        nanogui::Color(78, 88, 98, 255),
        nanogui::Color(52, 60, 70, 255),
        nanogui::Color(140, 150, 160, 255));
    m_statusLabel->set_caption("Form cleared.");
    refreshIOSelection();

    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void ManualControlPage::triggerIO(bool state)
{
    if (!m_selectedIO) {
        m_statusLabel->set_caption("Error: No IO selected.");
        return;
    }

    std::string durationStr = m_durationBox->value();
    unsigned long duration = 0;

    if (!durationStr.empty()) {
        try {
            duration = std::stoul(durationStr);
        } catch (...) {
            m_statusLabel->set_caption("Error: Invalid duration.");
            return;
        }
    }

    m_selectedIO->set(state);
    
    // Update visual indicator
    if (state) {
        stateIndicator(m_stateIndicator)->setState(
            "HIGH",
            nanogui::Color(255, 242, 242, 255),
            nanogui::Color(226, 72, 72, 255),
            nanogui::Color(158, 34, 34, 255),
            nanogui::Color(255, 160, 160, 255));
    } else {
        stateIndicator(m_stateIndicator)->setState(
            "LOW",
            nanogui::Color(55, 42, 0, 255),
            nanogui::Color(255, 219, 72, 255),
            nanogui::Color(214, 167, 18, 255),
            nanogui::Color(255, 238, 160, 255));
    }
    
    m_statusLabel->set_caption(
        std::string("Triggered: ") + (state ? "HIGH" : "LOW"));

    // If duration specified, automatically clear after delay
    if (duration > 0) {
        std::thread([this, duration]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            if (m_selectedIO) {
                m_selectedIO->clear();
                // Update indicator to LOW
                if (m_nanoScreen) {
                    nanogui::async([this]() {
                        stateIndicator(m_stateIndicator)->setState(
                            "LOW",
                            nanogui::Color(55, 42, 0, 255),
                            nanogui::Color(255, 219, 72, 255),
                            nanogui::Color(214, 167, 18, 255),
                            nanogui::Color(255, 238, 160, 255));
                        m_statusLabel->set_caption("Auto-cleared after duration.");
                        m_nanoScreen->redraw();
                    });
                }
            }
        }).detach();
    }
}

void ManualControlPage::cycleToNextIO()
{
    if (m_outputIOs.empty()) {
        m_statusLabel->set_caption("No output IOs available.");
        return;
    }

    // Move to next IO in the list
    m_currentIOIndex = (m_currentIOIndex + 1) % m_outputIOs.size();
    populateForm(m_outputIOs[m_currentIOIndex]);
}

void ManualControlPage::forceIO(bool state)
{
    if (!m_selectedIO) {
        m_statusLabel->set_caption("Error: No IO selected.");
        return;
    }

    // Release any previous forced IO
    if (m_forcedIO && m_forcedIO != m_selectedIO) {
        m_forcedIO->clear();
    }

    m_selectedIO->set(state);
    m_forcedIO = m_selectedIO;
    m_forcedState = state;
    
    // Update visual indicator
    if (state) {
        stateIndicator(m_stateIndicator)->setState(
            "FORCE HIGH",
            nanogui::Color(255, 242, 242, 255),
            nanogui::Color(235, 64, 64, 255),
            nanogui::Color(150, 26, 26, 255),
            nanogui::Color(255, 160, 160, 255));
    } else {
        stateIndicator(m_stateIndicator)->setState(
            "FORCE LOW",
            nanogui::Color(55, 42, 0, 255),
            nanogui::Color(255, 219, 72, 255),
            nanogui::Color(214, 167, 18, 255),
            nanogui::Color(255, 238, 160, 255));
    }
    
    m_statusLabel->set_caption(
        std::string("FORCED: ") + (state ? "HIGH (indefinite)" : "LOW (indefinite)"));
}

void ManualControlPage::releaseForcedIO()
{
    if (!m_forcedIO) {
        m_statusLabel->set_caption("No IO is currently forced.");
        return;
    }

    m_forcedIO->clear();
    m_forcedIO = nullptr;
    m_forcedState = false;
    
    // Update indicator
    if (m_selectedIO) {
        stateIndicator(m_stateIndicator)->setState(
            "LOW",
            nanogui::Color(55, 42, 0, 255),
            nanogui::Color(255, 219, 72, 255),
            nanogui::Color(214, 167, 18, 255),
            nanogui::Color(255, 238, 160, 255));
    } else {
        stateIndicator(m_stateIndicator)->setState(
            "UNKNOWN",
            nanogui::Color(210, 220, 230, 255),
            nanogui::Color(78, 88, 98, 255),
            nanogui::Color(52, 60, 70, 255),
            nanogui::Color(140, 150, 160, 255));
    }
    
    m_statusLabel->set_caption("Force released.");
}

