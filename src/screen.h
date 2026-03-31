#ifndef __SCREEN_H
#define __SCREEN_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>

namespace nanogui {
class Screen;
class Widget;
}

class DomeBackground;
class BadgePage;
class OverviewPage;
class SettingsPage;
class ManualControlPage;
class OrbitButton;

class Screen {
private:
	nanogui::Screen *m_screen;
	DomeBackground *m_background;

	nanogui::Widget *m_landingPanel;
	OverviewPage    *m_overviewPage;
	SettingsPage    *m_settingsPage;
	ManualControlPage *m_manualControlPage;
	BadgePage       *m_badgePage;
	OrbitButton     *m_overviewOrbitButton;
	OrbitButton     *m_badgesOrbitButton;
	OrbitButton     *m_settingsOrbitButton;
	OrbitButton     *m_manualOrbitButton;
	int              m_landingCenterX;
	int              m_landingCenterY;
	float            m_landingOrbitRadius;
	int              m_currentPage{0};
	std::function<void()> m_onBadgesChanged;

	std::thread m_statsThread;
	std::atomic<bool> m_stopStatsThread;

	void startStatsUpdates();
	void updateSystemStats();
	void switchPage(int page);
	void buildLandingPage();
	void buildOverviewPage();
	void buildSettingsPage();
	void buildManualControlPage();
	void updateLandingOrbit(float phase);

public:
	Screen(int width = 1024,
	       int height = 600,
	       const std::string &badgesDir = "badges",
	       std::function<void()> onBadgesChanged = nullptr);
	~Screen();

	void updateStatus(const std::string &status);
	void render();

	nanogui::Screen *getScreen() const;
};

#endif