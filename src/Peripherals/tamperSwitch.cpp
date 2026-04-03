#include "tamperSwitch.h"

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
	return std::string(DIR_SHARED) + "ios/system.json";
}
}

TamperSwitch::TamperSwitch(const std::string &configPath)
	: m_tamperIo(nullptr),
	  m_running(false),
	  m_configPath(configPath.empty() ? defaultConfigPath() : configPath),
	  m_activeHigh(true),
	  m_pollIntervalMs(100),
	  m_hasLastState(false),
	  m_lastTampered(false)
{
	loadFromConfig();
}

TamperSwitch::~TamperSwitch()
{
	stop();
}

bool TamperSwitch::isAvailable() const
{
	return m_tamperIo != nullptr;
}

bool TamperSwitch::isTampered() const
{
	if (!isAvailable()) {
		return false;
	}

	return readTamperState();
}

void TamperSwitch::setTamperHandler(std::function<void(bool)> handler)
{
	m_tamperHandler = std::move(handler);
}

void TamperSwitch::start()
{
	if (!isAvailable() || m_running.exchange(true)) {
		return;
	}

	m_runner = std::thread(&TamperSwitch::run, this);
}

void TamperSwitch::stop()
{
	if (!m_running.exchange(false)) {
		return;
	}

	if (m_runner.joinable()) {
		m_runner.join();
	}
}

void TamperSwitch::loadFromConfig()
{
	std::ifstream configFile(m_configPath);
	if (!configFile.is_open()) {
		ERR("Unable to open tamper switch config: " << m_configPath);
		return;
	}

	json configObject;
	try {
		configObject = json::parse(configFile);
	} catch (const std::exception &e) {
		ERR("Unable to parse tamper switch config '" << m_configPath << "': " << e.what());
		return;
	}

	if (!configObject.contains("tamperswitch") || !configObject["tamperswitch"].is_object()) {
		ERR("Tamper switch config is missing object 'tamperswitch': " << m_configPath);
		return;
	}

	json tamperObject = configObject["tamperswitch"];
	tamperObject["name"] = tamperObject.value("name", "tamper_switch");
	tamperObject["type"] = "LOCAL";
	tamperObject["direction"] = "IN";

	m_activeHigh = tamperObject.value("activeHigh", true);
	m_pollIntervalMs = tamperObject.value("pollIntervalMs", 100);
	if (m_pollIntervalMs < 10) {
		m_pollIntervalMs = 10;
	}

	std::unique_ptr<IO> io = std::make_unique<IO>();
	io->fromJson(tamperObject);
	m_tamperIo = std::move(io);

	DBG("Tamper switch loaded from " + m_configPath);
}

bool TamperSwitch::readTamperState() const
{
	if (!m_tamperIo) {
		return false;
	}

	const int value = m_tamperIo->get();
	if (value < 0) {
		return false;
	}

	return m_activeHigh ? value != 0 : value == 0;
}

void TamperSwitch::run()
{
	while (m_running.load()) {
		const bool tampered = readTamperState();
		const bool changed = !m_hasLastState || tampered != m_lastTampered;

		if (changed) {
			m_lastTampered = tampered;
			m_hasLastState = true;

			if (m_tamperHandler) {
				m_tamperHandler(tampered);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(m_pollIntervalMs));
	}
}
