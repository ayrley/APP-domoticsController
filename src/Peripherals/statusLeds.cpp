#include "statusLeds.h"

#include <array>
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

StatusLeds::StatusLeds(const std::string &configPath)
	: m_leds {},
	  m_configPath(configPath.empty() ? defaultConfigPath() : configPath),
	  m_state(STATE_OFF),
	  m_error(ERROR_NONE)
{
	loadFromConfig();
	clear();
}

StatusLeds::~StatusLeds()
{
}

bool StatusLeds::isAvailable() const
{
	for (const auto &led : m_leds) {
		if (led) {
			return true;
		}
	}

	return false;
}

void StatusLeds::setState(enum controllerState state)
{
	m_state = state;

	if (m_error != ERROR_NONE) {
		applyPattern(errorPattern(m_error));
		return;
	}

	applyPattern(statePattern(state));
}

void StatusLeds::showError(enum errorCode error)
{
	m_error = error;
	applyPattern(errorPattern(error));
}

void StatusLeds::clearError()
{
	m_error = ERROR_NONE;
	applyPattern(statePattern(m_state));
}

void StatusLeds::clear()
{
	applyPattern({false, false, false, false});
}

void StatusLeds::loadFromConfig()
{
	std::ifstream configFile(m_configPath);
	if (!configFile.is_open()) {
		ERR("Unable to open status LED config: " << m_configPath);
		return;
	}

	json configObject;
	try {
		configObject = json::parse(configFile);
	} catch (const std::exception &e) {
		ERR("Unable to parse status LED config '" << m_configPath << "': " << e.what());
		return;
	}

	if (!configObject.contains("Status") || !configObject["Status"].is_array()) {
		ERR("Status LED config is missing array 'Status': " << m_configPath);
		return;
	}

	const json &statusArray = configObject["Status"];
	for (size_t index = 0; index < m_leds.size() && index < statusArray.size(); ++index) {
		json ledObject = statusArray[index];
		ledObject["type"] = "LOCAL";
		ledObject["direction"] = "OUT";

		std::unique_ptr<IO> led = std::make_unique<IO>();
		led->fromJson(ledObject);
		m_leds[index] = std::move(led);
	}

	if (!isAvailable()) {
		DBG("Status LEDs unavailable: no usable entries in Status array");
	}
}

void StatusLeds::applyPattern(const std::array<bool, LED_COUNT> &pattern)
{
	for (size_t index = 0; index < m_leds.size(); ++index) {
		if (!m_leds[index]) {
			continue;
		}

		m_leds[index]->set(pattern[index]);
	}
}

std::array<bool, StatusLeds::LED_COUNT> StatusLeds::statePattern(enum controllerState state) const
{
	switch (state) {
	case STATE_BOOTING:
		return {true, false, true, false};
	case STATE_IDLE:
		return {true, false, false, false};
	case STATE_READY:
		return {true, true, false, false};
	case STATE_BUSY:
		return {true, true, true, false};
	case STATE_WARNING:
		return {true, false, true, true};
	case STATE_ERROR:
		return {true, false, false, true};
	case STATE_OFF:
	default:
		return {false, false, false, false};
	}
}

std::array<bool, StatusLeds::LED_COUNT> StatusLeds::errorPattern(enum errorCode error) const
{
	switch (error) {
	case ERROR_CONFIG:
		return {true, false, false, true};
	case ERROR_NETWORK:
		return {true, true, false, true};
	case ERROR_READER:
		return {true, false, true, true};
	case ERROR_IO:
		return {true, true, true, true};
	case ERROR_SYSTEM:
		return {false, false, false, true};
	case ERROR_NONE:
	default:
		return statePattern(m_state);
	}
}
