#include "lifeLed.h"

#include <chrono>
#include <fstream>

#include "control.h"
#include "debug.h"
#include "io.h"
#include "json.hpp"

using json = nlohmann::json;

namespace
{
std::string defaultConfigPath()
{
	return std::string(DIR_SHARED) + "ios/leds.json";
}
}

LifeLed::LifeLed(const std::string &configPath)
	: m_lifeIo(nullptr),
	  m_running(false),
	  m_configPath(configPath.empty() ? defaultConfigPath() : configPath)
{
	loadFromConfig();
}

LifeLed::~LifeLed()
{
	stop();
}

bool LifeLed::isAvailable() const
{
	return m_lifeIo != nullptr;
}

void LifeLed::start()
{
	if (!isAvailable() || m_running) {
		return;
	}

	m_running = true;
	m_runner = std::thread(&LifeLed::blinkLoop, this);
}

void LifeLed::stop()
{
	m_running = false;

	if (m_runner.joinable()) {
		m_runner.join();
	}

	if (m_lifeIo) {
		m_lifeIo->clear();
	}
}

void LifeLed::loadFromConfig()
{
	std::ifstream configFile(m_configPath);
	if (!configFile.is_open()) {
		ERR("Unable to open life LED config: " << m_configPath);
		return;
	}

	json configObject;
	try {
		configObject = json::parse(configFile);
	} catch (const std::exception &e) {
		ERR("Unable to parse life LED config '" << m_configPath << "': " << e.what());
		return;
	}

	if (!configObject.contains("life") || !configObject["life"].is_array() || configObject["life"].empty()) {
		DBG("Life LED config is missing a usable 'life' array");
		return;
	}

	json lifeObject = configObject["life"][0];
	lifeObject["type"] = "LOCAL";
	lifeObject["direction"] = "OUT";

	std::unique_ptr<IO> lifeIo = std::make_unique<IO>();
	lifeIo->fromJson(lifeObject);
	m_lifeIo = std::move(lifeIo);
}

void LifeLed::blinkLoop()
{
	while (m_running) {
		if (m_lifeIo) {
			m_lifeIo->set(true);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		if (!m_running) {
			break;
		}

		if (m_lifeIo) {
			m_lifeIo->set(false);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
