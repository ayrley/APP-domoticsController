#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>

#include <stdint.h>

#include "badge.h"
#include "debug.h"

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

    if (!jsonFile.is_open()) {
        DBG("Unable to open badge file: " + this->m_badgeFile);
        return -ENOENT;
    }

    if (jsonFile.peek() == std::ifstream::traits_type::eof()) {
        DBG("Badge file is empty, skipping: " + this->m_badgeFile);
        return -ENOENT;
    }

    try {
        this->m_badge = json::parse(jsonFile);
    } catch (const std::exception &e) {
        DBG("Badge parse failed for file '" + this->m_badgeFile + "': " + e.what());
        return -ENOENT;
    }

    if (this->m_badge.is_discarded())
        return -ENOENT;

    try {
        this->m_firstName = this->m_badge["firstName"];
    } catch (const std::exception &e) {
        this->m_firstName = "";
        DBG("firstName could not be found in " + this->m_badgeFile);
    }

    try {
        this->m_lastName = this->m_badge["lastName"];
    } catch (const std::exception &e) {
        this->m_lastName = "";
        DBG("lastName could not be found in " + this->m_badgeFile);
    }

    try {
        this->m_badgeNumber = this->m_badge["badgeNumber"];
    } catch (const std::exception &e) {
        this->m_badgeNumber = 0;
        DBG("badgeNumber could not be found in " + this->m_badgeFile);
    }

    return 0;
}

bool Badge::valid(uint64_t badgeToCheck)
{
    return (this->m_badgeNumber == badgeToCheck);
}

uint64_t Badge::getBadgeNumber()
{
    return this->m_badgeNumber;
}
