#include <string>
#include <fstream>
#include <cerrno>
#include <iostream>

#include <stdint.h>

#include "reader.h"

Reader::Reader()
{
}

Reader::~Reader()
{
}

void Reader::fromJson(const json &jsonObject)
{
	std::string tmpHelp;

	this->m_readerName = jsonObject.value("name", "");
	this->m_readerLocation = jsonObject.value("location", "");
	this->m_readerLocationType = RDR_LOC_LOCAL;
	this->m_readerType = RDR_WIEGAND;
	
	tmpHelp = jsonObject.value("location_type", "");
	if (tmpHelp == "IP")
		this->m_readerLocationType = RDR_LOC_IP;

	tmpHelp = jsonObject.value("type", "");
	if (tmpHelp == "OSDP")
		this->m_readerType = RDR_OSDP;

}
