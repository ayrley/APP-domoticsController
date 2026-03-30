#ifndef __SCREEN_H
#define __SCREEN_H

#include <string>
#include <thread>
#include <atomic>

namespace nanogui {
class Screen;
class Label;
class ProgressBar;
}

class JarvisBackground;

class Screen {
private:
	nanogui::Screen *m_screen;
	JarvisBackground *m_background;

	nanogui::Label *m_statusLabel;
	nanogui::Label *m_cpuLabel;
	nanogui::Label *m_ramLabel;
	nanogui::ProgressBar *m_cpuBar;
	nanogui::ProgressBar *m_ramBar;

	std::thread m_statsThread;
	std::atomic<bool> m_stopStatsThread;

	void startStatsUpdates();
	void updateSystemStats();

public:
	Screen(int width = 1024, int height = 768);
	~Screen();

	void updateStatus(const std::string &status);
	void render();

	nanogui::Screen *getScreen() const;
};

#endif