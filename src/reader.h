#ifndef __READER_H_
#define __READER_H_

#include <string>
#include <mutex>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

enum readerLocationType {
	RDR_LOC_LOCAL = 0,
	RDR_LOC_IP
};

enum readerType {
	RDR_WIEGAND = 0,
	RDR_OSDP
};

class Reader
{
private:
	std::string m_readerName;
	std::string m_readerLocation;
	
	readerLocationType m_readerLocationType;
	
	readerType m_readerType;

public:
	Reader();
	~Reader();
	
	void fromJson(const json &jsonObject);
	
	std::string getName();
	std::string getLocation();
	
	readerLocationType getLocationType();
	
	readerType getType();
};

#endif
