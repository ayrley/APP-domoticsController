#ifndef __BADGE_PAGE_H
#define __BADGE_PAGE_H

#include <functional>
#include <string>
#include <vector>

#include "defaultPage.h"

namespace nanogui {
class Label;
class TextBox;
class Screen;
} // namespace nanogui

class JarvisButton;

class BadgePage : public DefaultPage {
public:
	explicit BadgePage(nanogui::Widget     *parent,
	                   const std::string   &badgesDir,
	                   std::function<void()> onBack,
	                   std::function<void()> onBadgesChanged = nullptr);

	/// Rescan the badges directory and rebuild the list.
	void refresh();

private:
	void buildBadgeList();
	void populateForm(const std::string &badgeFile);
	void clearForm();
	void saveBadge();

	std::string           m_badgesDir;
	std::string           m_selectedBadgeFile;
	std::function<void()> m_onBadgesChanged;
	nanogui::Screen      *m_nanoScreen{nullptr};

	nanogui::Label   *m_statusLabel{nullptr};
	nanogui::Widget  *m_badgeListPanel{nullptr};
	nanogui::TextBox *m_fileNameBox{nullptr};
	nanogui::TextBox *m_firstNameBox{nullptr};
	nanogui::TextBox *m_lastNameBox{nullptr};
	nanogui::TextBox *m_badgeNumberBox{nullptr};

	std::vector<JarvisButton *> m_badgeButtons;
};

#endif
