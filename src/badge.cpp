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

void Badge::takeAction(bool accessGranted)
{

}

int Badge::parse()
{
	std::ifstream jsonFile(this->m_badgeFile);
	this->m_badge = json::parse(jsonFile);

	if (this->m_badge.is_discarded())
		return -ENOENT;

	try {
		this->m_firstName = this->m_badge["firstName"];
	}
	catch (const std::exception& e) {
		this->m_firstName = "";
		std::cout << "firstName could not be found in " << this->m_badgeFile
			<< std::endl;
	}

	try {
		this->m_lastName = this->m_badge["lastName"];
	}
	catch (const std::exception& e) {
		this->m_lastName = "";
		std::cout << "lastName could not be found in " << this->m_badgeFile
			<< std::endl;
	}

	try {
		this->m_badgeNumber = this->m_badge["badgeNumber"];
	}
	catch (const std::exception& e) {
		this->m_badgeNumber = 0;
		std::cout << "badgeNumber could not be found in " << this->m_badgeFile
			<< std::endl;
	}



	return 0;
}

bool Badge::valid(uint64_t badgeToCheck)
{
	return false;
}

uint64_t Badge::getBadgeNumber()
{
	return this->m_badgeNumber;
}
