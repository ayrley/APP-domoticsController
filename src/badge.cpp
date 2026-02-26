#include <string>
#include <fstream>
#include <cerrno>
#include <iostream>

#include <stdint.h>

#include "badge.h"

Badge::Badge(const std::string &badgeFile)
{
	this->m_badgeFile = badgeFile;
	this->parse();
}

Badge::~Badge()
{
}

int Badge::parse()
{
	std::ifstream jsonFile(this->m_badgeFile);
	this->m_badge = json::parse(jsonFile);
	
	if (this->m_badge.is_discarded())
		return -ENOENT;
		
	std::cout << this->m_badge << std::endl;
		
	this->m_firstName = this->m_badge["firstName"];
	this->m_lastName = this->m_badge["lastName"];
	this->m_badgeNumber = this->m_badge["badgeNumber"];

	return 0;
}

uint64_t Badge::getBadgeNumber()
{
	return this->m_badgeNumber;
}
