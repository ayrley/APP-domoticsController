#include <string>
#include <fstream>
#include <cerrno>
#include <iostream>
#include <thread>
#include <vector>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "reader.h"
#include "badge.h"

Reader::Reader()
{
	this->m_badges = nullptr;
	this->m_ledDuration = 3000;
}

Reader::Reader(std::vector<Badge *> *badges)
{
	this->m_badges = badges;
	this->m_ledDuration = 3000;
}

Reader::~Reader()
{
}

void Reader::setBadges(std::vector<Badge *> *badges)
{
	this->m_badges = badges;
}

void Reader::start()
{
	this->m_runner = std::thread(&Reader::handle, this);
	this->m_runner.detach();
}

int Reader::plainTextCode(char * binaryCardString, uint64_t *cardNumber,
	int length)
{
	int i;
	uint64_t bit;
	uint64_t newCardNumber = 0;
	int offset = 0;
	uint64_t swappedCardNumber = 0;

	if (length > 64)
		offset = length - 64 - 1;

	for (i = offset; i < length; i++) {
		bit = ((binaryCardString[i] == '0') ||
			(binaryCardString[i] && 0x01 == 0)) ? 0x00 : 0x01;
		newCardNumber = newCardNumber << 1;
		newCardNumber = newCardNumber & 0xfffffffffffffffe;
		newCardNumber |= (bit & 0x01);
	}

	swappedCardNumber = newCardNumber;
	if (*cardNumber != swappedCardNumber && swappedCardNumber != 0)
		*cardNumber = swappedCardNumber;

	return 0;
}

int Reader::getKeypad(uint64_t *cardNumber, int count)
{
	char tmpKeypad[2] = {0};

	/* only keep last nibble for getting a number */
	if (this->m_keypadBytes >= 10) {
		*cardNumber = atoi(this->m_keypad);
	} else if (count <= 8) {
		tmpKeypad[0] = (*cardNumber & 0x0f) + 48;

		if (((*cardNumber) & 0x0f) > 9) {
			*cardNumber = atoi(this->m_keypad);
			this->m_keypadBytes = 0;
			memset(this->m_keypad, 0, 12);
		} else {
			*cardNumber = 0;
			strncat(this->m_keypad, tmpKeypad, 1);
			this->m_keypadBytes++;
		}
	}

	return 0;
}

int Reader::getWiegandBadge(uint64_t *badge)
{
	char wiegandString[128];
	char buffer[33] = {0};
	int f_count, f_wiegand;
	int count;
	int readVal;
	uint64_t cardNumber = 0;

	std::string devicePath = "/dev/" + this->m_readerLocation;
	std::string countPath = "/sys/class/idtech/" + this->m_readerLocation +
		"/device/count";

	f_count = open(countPath.c_str(), O_RDONLY);
	if (!f_count)
		return -ENOENT;

	memset(buffer, 0, 33);
	memset(wiegandString, 0, 128);

	read(f_count, buffer, 32);
	close(f_count);
	count = strtol(buffer, NULL, 10);

	f_wiegand = open(devicePath.c_str(), O_RDONLY);
	if (!f_wiegand)
		return -ENOENT;

	readVal = read(f_wiegand, wiegandString, 64);
	close(f_wiegand);

	if (readVal < 0)
		return -EINVAL;

	this->plainTextCode(wiegandString, &cardNumber, count);
	this->getKeypad(&cardNumber, count);
	*badge = cardNumber;

	return 0;
}

void Reader::setWiegandLed(enum ledColor color)
{
	int fptr;
	char buffer[16];
	std::string ledPath = "/sys/class/idtech/" + this->m_readerLocation +
		"/device/color";

	fptr = open(ledPath.c_str(), O_WRONLY);
	if (!fptr)
		return;

	sprintf(buffer, "%d", color);
	write(fptr, buffer, strlen(buffer));
	close(fptr);

	this->m_ledPassedTime = 0;

	while (this->m_ledPassedTime < this->m_ledDuration) {
		usleep(50 * 1000);
		this->m_ledPassedTime += 50;
	}

	fptr = open(ledPath.c_str(), O_WRONLY);
	if (!fptr)
		return;

	sprintf(buffer, "0");
	write(fptr, buffer, strlen(buffer));
	close(fptr);
}

void Reader::handleWiegandReader()
{
	uint64_t badge = 0;
	bool validBadge = false;

	enum ledColor color = LED_NONE;

	std::thread ledRunner;

	while (true) {
		if (this->getWiegandBadge(&badge))
			continue;

		if (!badge)
			continue;

		for (auto singleBadge : *this->m_badges) {
			validBadge = singleBadge->valid(badge);
			if (validBadge) {
				color = LED_GREEN;
				continue;
			} else {
				color = LED_RED;
			}
		}

		//singleBadge->takeAction(validBadge)

		ledRunner = std::thread(&Reader::setWiegandLed, this, color);
		ledRunner.detach();
	}
}

void Reader::handleOsdpReader()
{

}

void Reader::handleNetworkReader()
{
}

void Reader::handleLocalReader()
{
	if (this->m_readerType == RDR_WIEGAND)
		this->handleWiegandReader();
	else if (this->m_readerType == RDR_OSDP)
		this->handleOsdpReader();
}

void Reader::handle()
{
	if (this->m_readerLocationType == RDR_LOC_LOCAL)
		this->handleLocalReader();
	else if (this->m_readerLocationType == RDR_LOC_IP)
		this->handleNetworkReader();
}

void Reader::fromJson(const json &jsonObject)
{
	std::string tmpHelp;

	this->m_readerName = jsonObject["name"];
	this->m_readerLocation = jsonObject["location"];
	this->m_readerLocationType = RDR_LOC_LOCAL;
	this->m_readerType = RDR_WIEGAND;

	tmpHelp = jsonObject["location_type"];
	if (tmpHelp == "IP")
		this->m_readerLocationType = RDR_LOC_IP;

	tmpHelp = jsonObject["type"];
	if (tmpHelp == "OSDP")
		this->m_readerType = RDR_OSDP;
}
