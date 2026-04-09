#ifndef __BADGE_PAGE_H
#define __BADGE_PAGE_H

#include <functional>
#include <string>
#include <vector>

#include "defaultPage.h"
#include "json.hpp"

using json = nlohmann::json;

struct RuleEntry {
    std::string zones;       // comma-separated zone names, e.g. "house,garage"
    std::string daysOfWeek;  // comma-separated 1-7 (1=Mon), e.g. "1,2,3,4,5"
    std::string timeWindows; // semicolon-separated HH:MM-HH:MM, e.g. "08:00-18:00"
};

namespace nanogui
{
class Label;
class TextBox;
class Screen;
class Widget;
}

class DomeButton;

class BadgePage : public DefaultPage
{
public:
    explicit BadgePage(nanogui::Widget *parent,
                       const std::string &badgesDir,
                       std::function<void()> onBack,
                       std::function<void()> onBadgesChanged = nullptr);

    void refresh();
    void perform_layout(NVGcontext *ctx) override;

private:
    void buildBadgeList();
    void populateForm(const std::string &badgeFile);
    void refreshBadgeSelection();
    void startNewBadge();
    void clearForm();
    void saveBadge();
    void removeBadge();
    void loadRulesFromJson(const json &data);
    void buildRulesList();
    void selectRule(int index);
    void applyRuleEdit();
    void addRule();
    void removeRule(int index);

    json serializeRulesToJson();

    std::string m_badgesDir;
    std::string m_selectedBadgeFile;

    std::function<void()> m_onBadgesChanged;

    nanogui::Screen *m_nanoScreen{nullptr};

    nanogui::Label *m_statusLabel{nullptr};

    nanogui::Widget *m_badgeListPanel{nullptr};
    nanogui::Widget *m_rulesListPanel{nullptr};
    nanogui::Widget *m_ruleEditPanel{nullptr};
    
    std::vector<nanogui::Widget *> m_ruleRows;

    nanogui::TextBox *m_fileNameBox{nullptr};
    nanogui::TextBox *m_firstNameBox{nullptr};
    nanogui::TextBox *m_lastNameBox{nullptr};
    nanogui::TextBox *m_badgeNumberBox{nullptr};
    nanogui::TextBox *m_ruleZonesBox{nullptr};
    nanogui::TextBox *m_ruleDaysBox{nullptr};
    nanogui::TextBox *m_ruleTimeWindowsBox{nullptr};            

    DomeButton *m_addBadgeButton{nullptr};;            
;           
    std::vector<DomeButton *> m_badgeButtons;

    std::vector<RuleEntry> m_ruleEntries;
    int m_selectedRuleIndex{-1};


};

#endif
