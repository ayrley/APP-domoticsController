#include "badgePage.h"
#include "domeButton.h"
#include "domeConfirmDialog.h"
#include "frostPanel.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <nanogui/nanogui.h>
#include <nanogui/screen.h>

#include "json.hpp"

using json = nlohmann::json;

BadgePage::BadgePage(nanogui::Widget *parent,
                     const std::string &badgesDir,
                     std::function<void()> onHome,
                     std::function<void()> onBadgesChanged) :
    DefaultPage(parent, std::move(onHome)),
    m_badgesDir(badgesDir),
    m_onBadgesChanged(std::move(onBadgesChanged)),
    m_nanoScreen(dynamic_cast<nanogui::Screen *>(parent))
{
    setPageTitle("BADGE CONFIGURATION");
    m_addBadgeButton = new DomeButton(this, "ADD", [this]() {
        startNewBadge();
    });
    m_addBadgeButton->setHomeStyle(true);
    m_addBadgeButton->setPalette(0.10f, 0.72f, 0.24f, 0.55f, 1.0f, 0.72f);
    m_addBadgeButton->set_fixed_size(nanogui::Vector2i(128, 42));
    contentPanel()->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 0, 18));

    auto *leftColumn = new FrostPanel(contentPanel());
    leftColumn->set_fixed_width(320);
    leftColumn->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    auto *listHeader = new nanogui::Label(leftColumn, "Configured Badges", "sans-bold");
    listHeader->set_font_size(17);
    listHeader->set_color(nanogui::Color(0, 200, 220, 255));

    auto *listScroll = new nanogui::VScrollPanel(leftColumn);
    listScroll->set_fixed_size(nanogui::Vector2i(306, 420));
    leftColumn->setScrollTarget(listScroll);

    m_badgeListPanel = new nanogui::Widget(listScroll);
    m_badgeListPanel->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 8));

    auto *rightColumn = new FrostPanel(contentPanel());
    rightColumn->setOpacityPercent(80.0f);
    rightColumn->set_layout(new nanogui::GroupLayout(0, 6, 14, 0));

    auto *formHeader = new nanogui::Label(rightColumn, "Configure Badge", "sans-bold");
    formHeader->set_font_size(17);
    formHeader->set_color(nanogui::Color(0, 200, 220, 255));

    m_statusLabel = new nanogui::Label(rightColumn, " ", "sans");
    m_statusLabel->set_font_size(15);
    m_statusLabel->set_color(nanogui::Color(160, 220, 160, 255));

    auto makeField = [&](const char *labelText, const char *placeholder,
                         nanogui::TextBox **out) {
        new nanogui::Label(rightColumn, labelText, "sans");
        auto *tb = new nanogui::TextBox(rightColumn, "");
        tb->set_editable(true);
        tb->set_placeholder(placeholder);
        tb->set_fixed_size(nanogui::Vector2i(460, 30));
        tb->set_font_size(16);
        tb->set_alignment(nanogui::TextBox::Alignment::Left);
        *out = tb;
    };

    makeField("File Name (without .json):", "e.g. john_doe", &m_fileNameBox);
    makeField("First Name:", "e.g. John", &m_firstNameBox);
    makeField("Last Name:", "e.g. Doe", &m_lastNameBox);
    makeField("Badge Number:", "e.g. 12345", &m_badgeNumberBox);

    auto *buttonRow = new nanogui::Widget(rightColumn);
    buttonRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
    buttonRow->set_fixed_height(44);

    new DomeButton(buttonRow, "Save", [this]() { saveBadge(); });
    new DomeButton(buttonRow, "Clear", [this]() { clearForm(); });
    new DomeButton(buttonRow, "Remove", [this]() { removeBadge(); });

    auto *rulesHeader = new nanogui::Label(rightColumn, "Access Rules", "sans-bold");
    rulesHeader->set_font_size(16);
    rulesHeader->set_color(nanogui::Color(0, 200, 220, 255));

    auto *rulesScroll = new nanogui::VScrollPanel(rightColumn);
    rulesScroll->set_fixed_size(nanogui::Vector2i(460, 130));

    m_rulesListPanel = new nanogui::Widget(rulesScroll);
    m_rulesListPanel->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 4));

    auto *rulesButtonRow = new nanogui::Widget(rightColumn);
    rulesButtonRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 8));
    rulesButtonRow->set_fixed_height(36);

    new DomeButton(rulesButtonRow, "Add Rule", [this]() { addRule(); });

    m_ruleEditPanel = new nanogui::Widget(rightColumn);
    m_ruleEditPanel->set_layout(new nanogui::GroupLayout(0, 4, 10, 0));
    m_ruleEditPanel->set_visible(false);

    auto *ruleEditHeader = new nanogui::Label(m_ruleEditPanel, "Edit Rule", "sans-bold");
    ruleEditHeader->set_font_size(15);
    ruleEditHeader->set_color(nanogui::Color(220, 180, 80, 255));

    auto makeRuleField = [&](const char *labelText, const char *placeholder,
                              nanogui::TextBox **out) {
        new nanogui::Label(m_ruleEditPanel, labelText, "sans");
        auto *tb = new nanogui::TextBox(m_ruleEditPanel, "");
        tb->set_editable(true);
        tb->set_placeholder(placeholder);
        tb->set_fixed_size(nanogui::Vector2i(460, 28));
        tb->set_font_size(15);
        tb->set_alignment(nanogui::TextBox::Alignment::Left);
        *out = tb;
    };

    makeRuleField("Zones (comma-separated):", "e.g. house,office", &m_ruleZonesBox);
    makeRuleField("Days of week (1=Mon ... 7=Sun, comma-sep):",
                  "e.g. 1,2,3,4,5", &m_ruleDaysBox);
    makeRuleField("Time windows (HH:MM-HH:MM, semicolon-sep):",
                  "e.g. 08:00-18:00;20:00-22:00", &m_ruleTimeWindowsBox);

    auto *ruleEditButtonRow = new nanogui::Widget(m_ruleEditPanel);
    ruleEditButtonRow->set_layout(new nanogui::BoxLayout(
        nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 8));
    ruleEditButtonRow->set_fixed_height(36);

    new DomeButton(ruleEditButtonRow, "Save Rule", [this]() { applyRuleEdit(); });
    new DomeButton(ruleEditButtonRow, "Cancel", [this]() {
        m_selectedRuleIndex = -1;
        m_ruleEditPanel->set_visible(false);
        buildRulesList();
        if (m_nanoScreen)
            m_nanoScreen->perform_layout();
    });

    buildBadgeList();
}

void BadgePage::refresh()
{
    buildBadgeList();
}

void BadgePage::perform_layout(NVGcontext *ctx)
{
    DefaultPage::perform_layout(ctx);

    if (m_addBadgeButton) {
        constexpr int kChromePadding = 14;
        constexpr int kButtonGap = 10;
        constexpr int kHomeButtonHeight = 42;
        nanogui::Vector2i addSize = m_addBadgeButton->fixed_size();
        m_addBadgeButton->set_position(nanogui::Vector2i(
            m_size.x() - addSize.x() - kChromePadding,
            kChromePadding + kHomeButtonHeight + kButtonGap));
    }
}

void BadgePage::buildBadgeList()
{
    for (auto *btn : m_badgeButtons) {
        m_badgeListPanel->remove_child(btn);
    }
    m_badgeButtons.clear();

    if (!std::filesystem::exists(m_badgesDir)) {
        m_statusLabel->set_caption("Badges directory not found: " + m_badgesDir);
        return;
    }

    std::vector<std::filesystem::path> paths;
    for (const auto &entry : std::filesystem::directory_iterator(m_badgesDir)) {
        if (entry.path().extension() == ".json")
            paths.push_back(entry.path());
    }
    std::sort(paths.begin(), paths.end());

    for (const auto &path : paths) {
        std::string file = path.string();
        std::string stem = path.stem().string();
        auto *btn = new DomeButton(m_badgeListPanel, stem, [this, file]() {
            populateForm(file);
        });
        btn->set_fixed_size(nanogui::Vector2i(286, 34));
        btn->setSelected(file == m_selectedBadgeFile);
        m_badgeButtons.push_back(btn);
    }

    if (m_badgeButtons.empty())
        m_statusLabel->set_caption("No badges configured yet. Fill the form and click Save.");

    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void BadgePage::refreshBadgeSelection()
{
    for (auto *btn : m_badgeButtons) {
        if (!btn) {
            continue;
        }
        std::string badgePath = m_badgesDir + "/" + btn->label() + ".json";
        btn->setSelected(badgePath == m_selectedBadgeFile);
    }

    if (m_nanoScreen) {
        m_nanoScreen->redraw();
    }
}

void BadgePage::startNewBadge()
{
    m_selectedBadgeFile.clear();
    m_fileNameBox->set_value("");
    m_firstNameBox->set_value("");
    m_lastNameBox->set_value("");
    m_badgeNumberBox->set_value("");
    m_ruleEntries.clear();
    m_selectedRuleIndex = -1;
    m_ruleEditPanel->set_visible(false);
    buildRulesList();
    refreshBadgeSelection();
    m_statusLabel->set_caption("Creating a new badge. Fill the form and click Save.");

    if (m_fileNameBox) {
        m_fileNameBox->request_focus();
    }
}

void BadgePage::populateForm(const std::string &badgeFile)
{
    try {
        std::ifstream f(badgeFile);
        if (!f.is_open()) {
            m_statusLabel->set_caption("Error: could not open " + badgeFile);
            return;
        }
        json data = json::parse(f);
        m_selectedBadgeFile = badgeFile;

        std::string stem = std::filesystem::path(badgeFile).stem().string();
        m_fileNameBox->set_value(stem);
        m_firstNameBox->set_value(data.value("firstName", std::string{}));
        m_lastNameBox->set_value(data.value("lastName", std::string{}));
        m_badgeNumberBox->set_value(
            std::to_string(data.value("badgeNumber", static_cast<uint64_t>(0))));
        loadRulesFromJson(data);
        refreshBadgeSelection();

        m_statusLabel->set_caption("Loaded: " + stem);
    } catch (const std::exception &e) {
        m_statusLabel->set_caption(std::string("Error: ") + e.what());
    }
}

void BadgePage::clearForm()
{
    m_firstNameBox->set_value("");
    m_lastNameBox->set_value("");
    m_badgeNumberBox->set_value("");
    m_ruleEntries.clear();
    m_selectedRuleIndex = -1;
    m_ruleEditPanel->set_visible(false);
    buildRulesList();
    m_statusLabel->set_caption("Form cleared (not saved).");
}

void BadgePage::saveBadge()
{
    std::string fileName = m_fileNameBox->value();
    if (fileName.empty()) {
        m_statusLabel->set_caption("Error: file name is required.");
        return;
    }

    std::replace(fileName.begin(), fileName.end(), '/', '_');
    std::replace(fileName.begin(), fileName.end(), '\\', '_');

    const std::string &numStr = m_badgeNumberBox->value();
    uint64_t badgeNum = 0;
    if (!numStr.empty()) {
        try {
            badgeNum = std::stoull(numStr);
        } catch (...) {
            m_statusLabel->set_caption("Error: badge number must be numeric.");
            return;
        }
    }

    json data;
    data["firstName"] = m_firstNameBox->value();
    data["lastName"] = m_lastNameBox->value();
    data["badgeNumber"] = badgeNum;

    json rules = serializeRulesToJson();
    if (!rules.empty())
        data["rules"] = rules;

    std::string filePath = m_badgesDir + "/" + fileName + ".json";
    try {
        std::ofstream f(filePath);
        if (!f.is_open()) {
            m_statusLabel->set_caption("Error: could not write " + filePath);
            return;
        }
        f << data.dump(2) << "\n";
        m_statusLabel->set_caption("Saved: " + fileName + ".json");
    } catch (const std::exception &e) {
        m_statusLabel->set_caption(std::string("Save error: ") + e.what());
        return;
    }

    m_selectedBadgeFile = filePath;
    refreshBadgeSelection();
    if (m_onBadgesChanged) {
        m_onBadgesChanged();
    }
    buildBadgeList();
}

void BadgePage::removeBadge()
{
    std::string fileName = m_fileNameBox ? m_fileNameBox->value() : std::string{};
    if (fileName.empty() && !m_selectedBadgeFile.empty()) {
        fileName = std::filesystem::path(m_selectedBadgeFile).stem().string();
    }

    if (fileName.empty()) {
        m_statusLabel->set_caption("Error: select a badge first.");
        return;
    }

    std::replace(fileName.begin(), fileName.end(), '/', '_');
    std::replace(fileName.begin(), fileName.end(), '\\', '_');
    const std::string filePath = m_badgesDir + "/" + fileName + ".json";

    if (!std::filesystem::exists(filePath)) {
        m_statusLabel->set_caption("Error: badge not found: " + fileName);
        return;
    }

    nanogui::Widget *dialogParent = m_nanoScreen ? static_cast<nanogui::Widget *>(m_nanoScreen)
                                                 : static_cast<nanogui::Widget *>(this);

    new DomeConfirmDialog(
        dialogParent,
        "CONFIRM DELETION",
        "Badge: " + fileName,
        "This action permanently removes the badge.",
        "Remove",
        "Cancel",
        [this, filePath, fileName]() {
            std::error_code ec;
            bool removed = std::filesystem::remove(filePath, ec);
            if (!removed || ec) {
                m_statusLabel->set_caption("Error: could not remove " + fileName);
                return;
            }

            m_selectedBadgeFile.clear();
            m_fileNameBox->set_value("");
            m_firstNameBox->set_value("");
            m_lastNameBox->set_value("");
            m_badgeNumberBox->set_value("");
            m_statusLabel->set_caption("Removed: " + fileName + ".json");

            if (m_onBadgesChanged) {
                m_onBadgesChanged();
            }
            buildBadgeList();
        },
        [this]() {
            m_statusLabel->set_caption("Removal canceled.");
        });
}

static std::string trimStr(const std::string &s)
{
    const auto start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return {};
    const auto end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

void BadgePage::loadRulesFromJson(const json &data)
{
    m_ruleEntries.clear();
    m_selectedRuleIndex = -1;
    m_ruleEditPanel->set_visible(false);

    if (!data.contains("rules") || !data["rules"].is_array()) {
        buildRulesList();
        return;
    }

    for (const auto &ruleJson : data["rules"]) {
        if (!ruleJson.is_object())
            continue;

        RuleEntry entry;

        if (ruleJson.contains("zones") && ruleJson["zones"].is_array()) {
            bool first = true;
            for (const auto &z : ruleJson["zones"]) {
                if (!z.is_string())
                    continue;
                if (!first)
                    entry.zones += ",";
                entry.zones += z.get<std::string>();
                first = false;
            }
        }

        if (ruleJson.contains("days_of_week") && ruleJson["days_of_week"].is_array()) {
            bool first = true;
            for (const auto &d : ruleJson["days_of_week"]) {
                if (!d.is_number_integer())
                    continue;
                if (!first)
                    entry.daysOfWeek += ",";
                entry.daysOfWeek += std::to_string(d.get<int>());
                first = false;
            }
        }

        if (ruleJson.contains("time_windows") && ruleJson["time_windows"].is_array()) {
            bool first = true;
            for (const auto &tw : ruleJson["time_windows"]) {
                if (!tw.is_object() || !tw.contains("start") || !tw.contains("end"))
                    continue;
                if (!first)
                    entry.timeWindows += ";";
                entry.timeWindows += tw["start"].get<std::string>() + "-" +
                                     tw["end"].get<std::string>();
                first = false;
            }
        }

        m_ruleEntries.push_back(entry);
    }

    buildRulesList();
}

json BadgePage::serializeRulesToJson()
{
    json rules = json::array();

    for (const auto &entry : m_ruleEntries) {
        json ruleJson = json::object();

        json zonesArray = json::array();
        {
            std::istringstream ss(entry.zones);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token = trimStr(token);
                if (!token.empty())
                    zonesArray.push_back(token);
            }
        }
        if (!zonesArray.empty())
            ruleJson["zones"] = zonesArray;

        json daysArray = json::array();
        {
            std::istringstream ss(entry.daysOfWeek);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token = trimStr(token);
                if (!token.empty()) {
                    try {
                        daysArray.push_back(std::stoi(token));
                    } catch (...) {
                    }
                }
            }
        }
        if (!daysArray.empty())
            ruleJson["days_of_week"] = daysArray;

        json twArray = json::array();
        {
            std::istringstream ss(entry.timeWindows);
            std::string token;
            while (std::getline(ss, token, ';')) {
                token = trimStr(token);
                // Find dash after "HH:MM" (position >= 5)
                const auto dash = token.find('-', 5);
                if (dash != std::string::npos) {
                    json twObj;
                    twObj["start"] = token.substr(0, dash);
                    twObj["end"] = token.substr(dash + 1);
                    twArray.push_back(twObj);
                }
            }
        }
        if (!twArray.empty())
            ruleJson["time_windows"] = twArray;

        rules.push_back(ruleJson);
    }

    return rules;
}

void BadgePage::buildRulesList()
{
    for (auto *row : m_ruleRows)
        m_rulesListPanel->remove_child(row);
    m_ruleRows.clear();

    for (int i = 0; i < static_cast<int>(m_ruleEntries.size()); ++i) {
        const auto &entry = m_ruleEntries[i];

        std::string summary;
        if (!entry.zones.empty())
            summary += "Z:" + entry.zones + "  ";
        if (!entry.daysOfWeek.empty())
            summary += "D:" + entry.daysOfWeek + "  ";
        if (!entry.timeWindows.empty())
            summary += "T:" + entry.timeWindows;
        if (summary.empty())
            summary = "(empty rule)";

        auto *row = new nanogui::Widget(m_rulesListPanel);
        row->set_layout(new nanogui::BoxLayout(
            nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));
        row->set_fixed_height(30);

        auto *editBtn = new DomeButton(row, summary, [this, i]() { selectRule(i); });
        editBtn->set_fixed_size(nanogui::Vector2i(412, 26));
        editBtn->setSelected(i == m_selectedRuleIndex);

        auto *removeBtn = new DomeButton(row, "X", [this, i]() { removeRule(i); });
        removeBtn->set_fixed_size(nanogui::Vector2i(30, 26));

        m_ruleRows.push_back(row);
    }

    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void BadgePage::selectRule(int index)
{
    if (index < 0 || index >= static_cast<int>(m_ruleEntries.size()))
        return;

    m_selectedRuleIndex = index;
    const auto &entry = m_ruleEntries[index];
    m_ruleZonesBox->set_value(entry.zones);
    m_ruleDaysBox->set_value(entry.daysOfWeek);
    m_ruleTimeWindowsBox->set_value(entry.timeWindows);
    m_ruleEditPanel->set_visible(true);
    buildRulesList(); // refresh selection highlight
    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void BadgePage::addRule()
{
    m_ruleEntries.push_back(RuleEntry{});
    m_selectedRuleIndex = static_cast<int>(m_ruleEntries.size()) - 1;
    m_ruleZonesBox->set_value("");
    m_ruleDaysBox->set_value("");
    m_ruleTimeWindowsBox->set_value("");
    m_ruleEditPanel->set_visible(true);
    buildRulesList();
    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void BadgePage::applyRuleEdit()
{
    if (m_selectedRuleIndex < 0 ||
        m_selectedRuleIndex >= static_cast<int>(m_ruleEntries.size()))
        return;

    auto &entry = m_ruleEntries[m_selectedRuleIndex];
    entry.zones = m_ruleZonesBox->value();
    entry.daysOfWeek = m_ruleDaysBox->value();
    entry.timeWindows = m_ruleTimeWindowsBox->value();

    m_selectedRuleIndex = -1;
    m_ruleEditPanel->set_visible(false);
    buildRulesList();
    m_statusLabel->set_caption("Rule updated. Click Save to persist.");
    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}

void BadgePage::removeRule(int index)
{
    if (index < 0 || index >= static_cast<int>(m_ruleEntries.size()))
        return;

    m_ruleEntries.erase(m_ruleEntries.begin() + index);

    if (m_selectedRuleIndex == index) {
        m_selectedRuleIndex = -1;
        m_ruleEditPanel->set_visible(false);
    } else if (m_selectedRuleIndex > index) {
        --m_selectedRuleIndex;
    }

    buildRulesList();
    m_statusLabel->set_caption("Rule removed. Click Save to persist.");
    if (m_nanoScreen)
        m_nanoScreen->perform_layout();
}
