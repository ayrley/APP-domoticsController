#ifndef __TAMPER_SWITCH_H
#define __TAMPER_SWITCH_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

class IO;

class TamperSwitch
{
public:
	explicit TamperSwitch(const std::string &configPath = "");
	~TamperSwitch();

	bool isAvailable() const;
	bool isTampered() const;
	void setTamperHandler(std::function<void(bool)> handler);
	void start();
	void stop();

private:
	std::unique_ptr<IO> m_tamperIo;
	std::thread m_runner;
	std::atomic<bool> m_running;
	std::string m_configPath;
	bool m_activeHigh;
	int m_pollIntervalMs;
	bool m_hasLastState;
	bool m_lastTampered;
	std::function<void(bool)> m_tamperHandler;

	void loadFromConfig();
	bool readTamperState() const;
	void run();
};

#endif
