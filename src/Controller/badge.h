#ifndef __BADGE_H_
#define __BADGE_H_

#include <string>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

class Badge
{
private:
    json m_badge;

    std::string m_badgeFile;

    std::string m_firstName;
    std::string m_lastName;

    uint64_t m_badgeNumber;

    int parse();

public:
    Badge(const std::string &badgeFile);
    ~Badge();

    uint64_t getBadgeNumber();

    std::string getFirstName();
    std::string getLastName();

    bool valid(uint64_t badgeToCheck);
};

#endif
