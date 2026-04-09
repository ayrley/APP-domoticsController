#ifndef __BADGE_H_
#define __BADGE_H_

#include <string>
#include <vector>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

struct BadgeTimeWindow {
    int startMinutes = -1;
    int endMinutes = -1;
};

struct BadgeRule {
    std::vector<std::string> zones;
    std::vector<int> daysOfWeek;
    std::vector<BadgeTimeWindow> timeWindows;
};

class Badge
{
private:
    json m_badge;

    std::string m_badgeFile;

    std::string m_firstName;
    std::string m_lastName;

    uint64_t m_badgeNumber;
    std::vector<BadgeRule> m_rules;

    int parse();

public:
    Badge(const std::string &badgeFile);
    ~Badge();

    uint64_t getBadgeNumber();

    std::string getFirstName();
    std::string getLastName();

    bool valid(uint64_t badgeToCheck,
               const std::string &zoneName = "",
               std::string *failureReason = nullptr);
};

#endif
