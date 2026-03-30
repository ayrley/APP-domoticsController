#include "badgePage.h"
#include "domeConfirmDialog.h"
#include "domeButton.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include <nanogui/nanogui.h>
#include <nanogui/screen.h>

#include "json.hpp"

using json = nlohmann::json;

BadgePage::BadgePage(nanogui::Widget     *parent,
                     const std::string   &badgesDir,
										 std::function<void()> onHome,
										 std::function<void()> onBadgesChanged)
    : DefaultPage(parent, std::move(onHome)),
      m_badgesDir(badgesDir),
			m_onBadgesChanged(std::move(onBadgesChanged)),
      m_nanoScreen(dynamic_cast<nanogui::Screen *>(parent)) {
	contentPanel()->set_layout(new nanogui::GroupLayout(15, 6, 14, 10));

	auto *titleLabel = new nanogui::Label(contentPanel(), "BADGE CONFIGURATION", "sans-bold");
	titleLabel->set_font_size(26);
	titleLabel->set_color(nanogui::Color(0, 230, 255, 255));

	m_statusLabel = new nanogui::Label(contentPanel(), " ", "sans");
	m_statusLabel->set_font_size(15);
	m_statusLabel->set_color(nanogui::Color(160, 220, 160, 255));

	auto *listHeader = new nanogui::Label(contentPanel(), "Configured Badges", "sans-bold");
	listHeader->set_font_size(17);
	listHeader->set_color(nanogui::Color(0, 200, 220, 255));

	m_badgeListPanel = new nanogui::Widget(contentPanel());
	m_badgeListPanel->set_layout(new nanogui::BoxLayout(
	    nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 8));
	m_badgeListPanel->set_fixed_height(44);

	auto *formHeader = new nanogui::Label(contentPanel(), "Configure Badge", "sans-bold");
	formHeader->set_font_size(17);
	formHeader->set_color(nanogui::Color(0, 200, 220, 255));

	auto makeField = [&](const char *labelText, const char *placeholder,
	                     nanogui::TextBox **out) {
		new nanogui::Label(contentPanel(), labelText, "sans");
		auto *tb = new nanogui::TextBox(contentPanel(), "");
		tb->set_editable(true);
		tb->set_placeholder(placeholder);
		tb->set_fixed_size(nanogui::Vector2i(460, 30));
		tb->set_font_size(16);
		tb->set_alignment(nanogui::TextBox::Alignment::Left);
		*out = tb;
	};

	makeField("File Name (without .json):", "e.g. john_doe", &m_fileNameBox);
	makeField("First Name:",                "e.g. John",     &m_firstNameBox);
	makeField("Last Name:",                 "e.g. Doe",      &m_lastNameBox);
	makeField("Badge Number:",              "e.g. 12345",    &m_badgeNumberBox);

	auto *buttonRow = new nanogui::Widget(contentPanel());
	buttonRow->set_layout(new nanogui::BoxLayout(
	    nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 12));
	buttonRow->set_fixed_height(44);

	new DomeButton(buttonRow, "Save",  [this]() { saveBadge(); });
	new DomeButton(buttonRow, "Clear", [this]() { clearForm(); });
	new DomeButton(buttonRow, "Remove", [this]() { removeBadge(); });

	buildBadgeList();
}

void BadgePage::refresh() {
	buildBadgeList();
}

void BadgePage::buildBadgeList() {
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
		btn->set_fixed_height(34);
		m_badgeButtons.push_back(btn);
	}

	if (m_badgeButtons.empty())
		m_statusLabel->set_caption("No badges configured yet. Fill the form and click Save.");

	if (m_nanoScreen)
		m_nanoScreen->perform_layout();
}

void BadgePage::populateForm(const std::string &badgeFile) {
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
		m_lastNameBox->set_value(data.value("lastName",  std::string{}));
		m_badgeNumberBox->set_value(
		    std::to_string(data.value("badgeNumber", static_cast<uint64_t>(0))));

		m_statusLabel->set_caption("Loaded: " + stem);
	} catch (const std::exception &e) {
		m_statusLabel->set_caption(std::string("Error: ") + e.what());
	}
}

void BadgePage::clearForm() {
	// Keep selected badge/file name context; only field values are cleared until Save is clicked.
	m_firstNameBox->set_value("");
	m_lastNameBox->set_value("");
	m_badgeNumberBox->set_value("");
	m_statusLabel->set_caption("Form cleared (not saved).");
}

void BadgePage::saveBadge() {
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
	data["firstName"]   = m_firstNameBox->value();
	data["lastName"]    = m_lastNameBox->value();
	data["badgeNumber"] = badgeNum;

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
	if (m_onBadgesChanged) {
		m_onBadgesChanged();
	}
	buildBadgeList();
}

void BadgePage::removeBadge() {
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
