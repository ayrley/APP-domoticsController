#include "eventLog.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

#include "control.h"

namespace
{
std::mutex g_eventLogMutex;
std::deque<EventLogEntry> g_eventLog;
constexpr std::size_t kMaxEntries = 500;
}

void EventLog::addBadgeRead(const std::string &readerName, uint64_t badge, bool granted,
                             const std::string &firstName, const std::string &lastName)
{
    std::stringstream message;
    message << "badge=" << badge << " reader=" << readerName
            << " result=" << (granted ? "GRANTED" : "DENIED");
    if (!firstName.empty() || !lastName.empty()) {
        message << " name=" << firstName;
        if (!lastName.empty()) {
            message << " " << lastName;
        }
    }
    append("[BADGE] " + message.str());
}

void EventLog::addStarting()
{
    append("[SYSTEM] Starting up");
}

void EventLog::addStopping()
{
    append("[SYSTEM] Shutting down");
}

void EventLog::addOutputToggle(const std::string &outputName, bool high, const std::string &source)
{
    std::stringstream message;
    message << "output=" << outputName
            << " state=" << (high ? "HIGH" : "LOW")
            << " source=" << source;
    append("[OUTPUT] " + message.str());
}

void EventLog::addTamper(bool detected)
{
    append(std::string("[TAMPER] ") + (detected ? "TRIGGERED" : "CLEARED"));
}

void EventLog::clear()
{
    std::lock_guard<std::mutex> lock(g_eventLogMutex);
    g_eventLog.clear();
}

std::vector<EventLogEntry> EventLog::getRecent(std::size_t limit)
{
    std::lock_guard<std::mutex> lock(g_eventLogMutex);

    std::vector<EventLogEntry> entries;
    if (limit == 0 || g_eventLog.empty()) {
        return entries;
    }

    const std::size_t count = std::min(limit, g_eventLog.size());
    entries.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t index = g_eventLog.size() - 1 - i;
        entries.push_back(g_eventLog[index]);
    }

    return entries;
}

void EventLog::append(const std::string &message)
{
    std::lock_guard<std::mutex> lock(g_eventLogMutex);

    EventLogEntry entry {nowTimestamp(), message};

    g_eventLog.push_back(entry);
    if (g_eventLog.size() > kMaxEntries) {
        g_eventLog.pop_front();
    }

    appendPersistent(entry);
}

std::string EventLog::nowTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime {};
    localtime_r(&nowTime, &localTime);

    char buffer[64] = {0};
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%04d-%02d-%02d %02d:%02d:%02d",
                  localTime.tm_year + 1900,
                  localTime.tm_mon + 1,
                  localTime.tm_mday,
                  localTime.tm_hour,
                  localTime.tm_min,
                  localTime.tm_sec);

    return std::string(buffer);
}

void EventLog::appendPersistent(const EventLogEntry &entry)
{
    const std::string logPath = persistentLogPath();
    if (logPath.empty()) {
        return;
    }

    std::ofstream logFile(logPath, std::ios::app);
    if (!logFile.is_open()) {
        return;
    }

    logFile << entry.timestamp << " " << entry.message << std::endl;
}

std::string EventLog::currentWeekKey()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime {};
    localtime_r(&nowTime, &localTime);

    const int weekOfYear = (localTime.tm_yday / 7) + 1;

    char buffer[32] = {0};
    std::snprintf(buffer,
                  sizeof(buffer),
                  "%04d-W%02d",
                  localTime.tm_year + 1900,
                  weekOfYear);

    return std::string(buffer);
}

std::string EventLog::persistentLogPath()
{
    const std::string baseDir = std::string(DIR_SHARED) + "logs";

    std::error_code ec;
    std::filesystem::create_directories(baseDir, ec);
    if (ec) {
        return "";
    }

    return baseDir + "/eventlog-" + currentWeekKey() + ".log";
}
