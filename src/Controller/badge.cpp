#include <cerrno>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

#include "badge.h"
#include "debug.h"

namespace
{
int parseMinutesOfDay(const std::string &value)
{
    std::stringstream stream(value);
    int hour = 0;
    int minute = 0;
    char separator = 0;

    stream >> hour >> separator >> minute;
    if (stream.fail() || separator != ':' || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        return -1;
    }

    return (hour * 60) + minute;
}

bool timeInWindow(int currentMinutes, int startMinutes, int endMinutes)
{
    if (startMinutes < 0 || endMinutes < 0) {
        return false;
    }

    if (startMinutes <= endMinutes) {
        return currentMinutes >= startMinutes && currentMinutes <= endMinutes;
    }

    return currentMinutes >= startMinutes || currentMinutes <= endMinutes;
}

bool containsWeekday(const json &daysOfWeek, int weekday)
{
    if (!daysOfWeek.is_array()) {
        return false;
    }

    for (const auto &day : daysOfWeek) {
        if (day.is_number_integer() && day.get<int>() == weekday) {
            return true;
        }
    }

    return false;
}

bool containsWeekday(const std::vector<int> &daysOfWeek, int weekday)
{
    for (int day : daysOfWeek) {
        if (day == weekday) {
            return true;
        }
    }

    return false;
}

bool containsZone(const json &zones, const std::string &zoneName)
{
    if (zoneName.empty() || !zones.is_array()) {
        return false;
    }

    for (const auto &zone : zones) {
        if (zone.is_string() && zone.get<std::string>() == zoneName) {
            return true;
        }
    }

    return false;
}

bool containsZone(const std::vector<std::string> &zones, const std::string &zoneName)
{
    if (zoneName.empty()) {
        return false;
    }

    for (const auto &zone : zones) {
        if (zone == zoneName) {
            return true;
        }
    }

    return false;
}
} // namespace

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

    this->m_rules.clear();
    if (this->m_badge.contains("rules") && this->m_badge["rules"].is_array()) {
        for (const auto &ruleJson : this->m_badge["rules"]) {
            if (!ruleJson.is_object()) {
                continue;
            }

            BadgeRule rule;

            if (ruleJson.contains("zones") && ruleJson["zones"].is_array()) {
                for (const auto &zone : ruleJson["zones"]) {
                    if (zone.is_string()) {
                        rule.zones.push_back(zone.get<std::string>());
                    }
                }
            }

            if (ruleJson.contains("days_of_week") && ruleJson["days_of_week"].is_array()) {
                for (const auto &day : ruleJson["days_of_week"]) {
                    if (day.is_number_integer()) {
                        rule.daysOfWeek.push_back(day.get<int>());
                    }
                }
            }

            if (ruleJson.contains("time_windows") && ruleJson["time_windows"].is_array()) {
                for (const auto &timeWindowJson : ruleJson["time_windows"]) {
                    if (!timeWindowJson.is_object() || !timeWindowJson.contains("start") ||
                        !timeWindowJson.contains("end") || !timeWindowJson["start"].is_string() ||
                        !timeWindowJson["end"].is_string()) {
                        continue;
                    }

                    BadgeTimeWindow timeWindow;
                    timeWindow.startMinutes = parseMinutesOfDay(timeWindowJson["start"]);
                    timeWindow.endMinutes = parseMinutesOfDay(timeWindowJson["end"]);
                    rule.timeWindows.push_back(timeWindow);
                }
            }

            this->m_rules.push_back(rule);
        }
    }

    return 0;
}

bool Badge::valid(uint64_t badgeToCheck, const std::string &zoneName, std::string *failureReason)
{
    if (this->m_badgeNumber != badgeToCheck) {
        if (failureReason != nullptr) {
            *failureReason = "badge number mismatch";
        }
        return false;
    }

    if (this->m_rules.empty()) {
        return true;
    }

    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_r(&now, &localTime);

    const int weekday = localTime.tm_wday == 0 ? 7 : localTime.tm_wday;
    const int currentMinutes = (localTime.tm_hour * 60) + localTime.tm_min;
    bool zoneMismatch = false;
    bool weekdayMismatch = false;
    bool timeMismatch = false;

    for (const auto &rule : this->m_rules) {
        if (!rule.zones.empty() && !containsZone(rule.zones, zoneName)) {
            zoneMismatch = true;
            continue;
        }

        if (!rule.daysOfWeek.empty() && !containsWeekday(rule.daysOfWeek, weekday)) {
            weekdayMismatch = true;
            continue;
        }

        if (!rule.timeWindows.empty()) {
            bool matchesTimeWindow = false;

            for (const auto &timeWindow : rule.timeWindows) {
                if (timeInWindow(currentMinutes, timeWindow.startMinutes, timeWindow.endMinutes)) {
                    matchesTimeWindow = true;
                    break;
                }
            }

            if (!matchesTimeWindow) {
                timeMismatch = true;
                continue;
            }
        }

        return true;
    }

    if (failureReason != nullptr) {
        if (zoneMismatch) {
            *failureReason = "zone not allowed: " + zoneName;
        } else if (weekdayMismatch) {
            *failureReason = "day of week not allowed";
        } else if (timeMismatch) {
            *failureReason = "time window not allowed";
        } else {
            *failureReason = "no matching access rule";
        }
    }

    return false;
}

uint64_t Badge::getBadgeNumber()
{
    return this->m_badgeNumber;
}

std::string Badge::getFirstName()
{
    return this->m_firstName;
}

std::string Badge::getLastName()
{
    return this->m_lastName;
}
