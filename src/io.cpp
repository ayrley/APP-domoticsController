#include <string>
#include <fstream>
#include <cerrno>
#include <iostream>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "io.h"
#include "debug.h"

IO::IO()
{
	this->m_exported = false;
}

IO::~IO()
{
}

int IO::exportIo()
{
	int fptr;
	std::string gpioExportLocation = "";
	std::string exportLocation = "/sys/class/gpio/export";
	std::string gpioNumber;

	if (this->m_direction == IO_DIR_AIN) {
		this->m_exported = true;
		return 0;
	}

	gpioNumber = this->m_location.substr(this->m_location.find("gpio") + 1);

	fptr = open(exportLocation.c_str(), O_WRONLY);
	if (!fptr)
		return fptr;

	write(fptr, gpioNumber.c_str(), strlen(gpioNumber.c_str()));
	close(fptr);

	gpioExportLocation = this->m_location + "/direction";
	fptr = open(gpioExportLocation.c_str(), O_WRONLY);

	if (this->m_direction == IO_DIR_IN)
		write(fptr, "in" , 2);
	else if (this->m_direction == IO_DIR_OUT)
		write(fptr, "out" , 3);

	close(fptr);

	return 0;
}

void IO::clear()
{
	this->set(false);
}

void IO::set()
{
	this->set(true);
}

void IO::set(bool high)
{
	int fptr;
	std::string gpioValue = this->m_location + "/value";

	if (!this->m_exported)
		this->exportIo();

	fptr = open(gpioValue.c_str(), O_WRONLY);
	if (!fptr) 
		return;

	write(fptr, high ? "1" : "0", 1);
	close(fptr);
}

int IO::getAnalogIo()
{
	int fptr;
	char buffer[16] = {0};

	fptr = open(this->m_location.c_str(), O_RDONLY);
	if (!fptr) 
		return -1;

	read(fptr, buffer, 8);
	close (fptr);

	return std::stoi(buffer);
}

int IO::getDigitalIo()
{
	int fptr;
	char buffer[16] = {0};
	std::string gpioValue = this->m_location + "/value";

	fptr = open(gpioValue.c_str(), O_RDONLY);
	if (!fptr) 
		return -1;

	read(fptr, buffer, 5);
	close (fptr);

	return std::stoi(buffer);
}

int IO::get()
{
	if (!this->m_exported)
		this->exportIo();

	if (this->m_direction == IO_DIR_AIN)
		return this->getAnalogIo();
	else if (this->m_direction == IO_DIR_IN)
		return this->getDigitalIo();

	return -1;
}

void IO::fromJson(const json &jsonObject)
{
	try {
		this->m_name = jsonObject["name"];
	}
	catch (const std::exception& e) {
		this->m_name = "";
		DBG("name could not be found in " + jsonObject.dump());
	}

	try {
		this->m_location = jsonObject["location"];
	}
	catch (const std::exception& e) {
		this->m_location = "";
		DBG("location could not be found in " + jsonObject.dump());
	}

	try {
		std::string locationType = jsonObject["type"];
		if (locationType == "LOCAL")
			this->m_locationType = IO_LOC_LOCAL;
		else if (locationType == "IP")
			this->m_locationType = IO_LOC_IP;
	}
	catch (const std::exception& e) {
		this->m_locationType = IO_LOC_LOCAL;
		DBG("type could not be found in " + jsonObject.dump());
	}

	try {
		std::string direction = jsonObject["direction"];
		if (direction == "AIN")
			this->m_direction = IO_DIR_AIN;
		else if (direction == "IN")
			this->m_direction = IO_DIR_IN;
		else if (direction == "OUT")
			this->m_direction = IO_DIR_OUT;
	}
	catch (const std::exception& e) {
		this->m_direction = IO_DIR_IN;
		DBG("direction could not be found in " + jsonObject.dump());
	}
}
