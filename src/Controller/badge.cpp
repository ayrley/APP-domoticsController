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
}

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

    if (!this->m_badge.contains("rules") || !this->m_badge["rules"].is_array() || this->m_badge["rules"].empty()) {
        return true;
    }

    const std::time_t now = std::time(nullptr);
    std::tm localTime {};
    localtime_r(&now, &localTime);

    const int weekday = localTime.tm_wday == 0 ? 7 : localTime.tm_wday;
    const int currentMinutes = (localTime.tm_hour * 60) + localTime.tm_min;
    bool zoneMismatch = false;
    bool weekdayMismatch = false;
    bool timeMismatch = false;

    for (const auto &rule : this->m_badge["rules"]) {
        if (!rule.is_object()) {
            continue;
        }

        if (rule.contains("zones") && !containsZone(rule["zones"], zoneName)) {
            zoneMismatch = true;
            continue;
        }

        if (rule.contains("days_of_week") && !containsWeekday(rule["days_of_week"], weekday)) {
            weekdayMismatch = true;
            continue;
        }

        if (rule.contains("time_windows") && rule["time_windows"].is_array()) {
            bool matchesTimeWindow = false;

            for (const auto &timeWindow : rule["time_windows"]) {
                if (!timeWindow.is_object() || !timeWindow.contains("start") || !timeWindow.contains("end") ||
                    !timeWindow["start"].is_string() || !timeWindow["end"].is_string()) {
                    continue;
                }

                const int startMinutes = parseMinutesOfDay(timeWindow["start"]);
                const int endMinutes = parseMinutesOfDay(timeWindow["end"]);
                if (timeInWindow(currentMinutes, startMinutes, endMinutes)) {
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
