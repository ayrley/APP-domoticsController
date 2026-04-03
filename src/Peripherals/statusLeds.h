#ifndef __STATUS_LEDS_H
#define __STATUS_LEDS_H

#include <array>
#include <memory>
#include <string>

class IO;

class StatusLeds
{
public:
	enum controllerState {
		STATE_OFF = 0,
		STATE_BOOTING,
		STATE_IDLE,
		STATE_READY,
		STATE_BUSY,
		STATE_WARNING,
		STATE_ERROR
	};

	enum errorCode {
		ERROR_NONE = 0,
		ERROR_CONFIG,
		ERROR_NETWORK,
		ERROR_READER,
		ERROR_IO,
		ERROR_SYSTEM
	};

	explicit StatusLeds(const std::string &configPath = "");
	~StatusLeds();

	bool isAvailable() const;

	void setState(enum controllerState state);
	void showError(enum errorCode error);
	void clearError();
	void clear();

private:
	enum ledSlot {
		LED_POWER = 0,
		LED_READY,
		LED_ACTIVITY,
		LED_ERROR,
		LED_COUNT
	};

	std::array<std::unique_ptr<IO>, LED_COUNT> m_leds;
	std::string m_configPath;
	enum controllerState m_state;
	enum errorCode m_error;

	void loadFromConfig();
	void applyPattern(const std::array<bool, LED_COUNT> &pattern);
	std::array<bool, LED_COUNT> statePattern(enum controllerState state) const;
	std::array<bool, LED_COUNT> errorPattern(enum errorCode error) const;
};

#endif
