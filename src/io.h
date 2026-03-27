#ifndef __IO_H_
#define __IO_H_

#include <string>
#include <mutex>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

enum ioLocationType {
	IO_LOC_LOCAL = 0,
	IO_LOC_IP
};

enum ioDirection {
	IO_DIR_IN = 0,
	IO_DIR_AIN,
	IO_DIR_OUT
};

class IO
{
private:
	std::string m_name;
	std::string m_location;
	std::string m_fullLocation;

	enum ioLocationType m_locationType;
	enum ioDirection m_direction;

	bool m_exported;
	int m_duration;

	int exportIo();
	int getDigitalIo();
	int getAnalogIo();

public:
	IO();
	~IO();

	std::string getName() { return this->m_name; }

	void fromJson(const json &jsonObject);
	void clear();
	void set();
	void set(bool high);

	int get();
	int getDuration() { return this->m_duration; }
};

#endif
