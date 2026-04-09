#ifndef __EVENT_LOG_H
#define __EVENT_LOG_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct EventLogEntry {
    std::string timestamp;
    std::string message;
};

class EventLog
{
public:
    static void addBadgeRead(const std::string &readerName, uint64_t badge, bool granted,
                             const std::string &firstName = "", const std::string &lastName = "");
    static void addOutputToggle(const std::string &outputName, bool high, const std::string &source);
    static void addTamper(bool detected);
    static void clear();

    static std::vector<EventLogEntry> getRecent(std::size_t limit = 200);

private:
    static void append(const std::string &message);
    static std::string nowTimestamp();
    static void appendPersistent(const EventLogEntry &entry);
    static std::string currentWeekKey();
    static std::string persistentLogPath();
};

#endif
