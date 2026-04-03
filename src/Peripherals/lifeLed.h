#ifndef __LIFE_LED_H
#define __LIFE_LED_H

#include <atomic>
#include <memory>
#include <string>
#include <thread>

class IO;

class LifeLed
{
public:
	explicit LifeLed(const std::string &configPath = "");
	~LifeLed();

	bool isAvailable() const;
	void start();
	void stop();

private:
	std::unique_ptr<IO> m_lifeIo;
	std::thread m_runner;
	std::atomic<bool> m_running;
	std::string m_configPath;

	void loadFromConfig();
	void blinkLoop();
};

#endif
