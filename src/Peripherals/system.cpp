#include "system.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/reboot.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <system/Command.hpp>

System::CpuSample System::readCpuSample()
{
    std::ifstream statFile("/proc/stat");
    std::string cpu;
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

    statFile >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    CpuSample sample;
    sample.idle = idle + iowait;
    sample.total = user + nice + system + idle + iowait + irq + softirq + steal;

    return sample;
}

std::string System::formatCpuUsage(const CpuSample &previousSample, const CpuSample &currentSample)
{
    double usageFraction = cpuUsageFraction(previousSample, currentSample);

    return formatPercent(usageFraction * 100.0);
}

double System::cpuUsageFraction(const CpuSample &previousSample, const CpuSample &currentSample)
{
    unsigned long long totalDelta = currentSample.total - previousSample.total;
    unsigned long long idleDelta = currentSample.idle - previousSample.idle;
    double usageFraction = 0.0;

    if (totalDelta != 0) {
        usageFraction = static_cast<double>(totalDelta - idleDelta) / static_cast<double>(totalDelta);
    }

    if (usageFraction < 0.0) {
        return 0.0;
    }
    if (usageFraction > 1.0) {
        return 1.0;
    }

    return usageFraction;
}

std::string System::readRamUsage()
{
    std::ifstream meminfoFile("/proc/meminfo");
    std::string key;
    std::string unit;
    unsigned long long value = 0;
    unsigned long long totalKb = 0;
    unsigned long long availableKb = 0;

    while (meminfoFile >> key >> value >> unit) {
        if (key == "MemTotal:") {
            totalKb = value;
        } else if (key == "MemAvailable:") {
            availableKb = value;
        }

        if (totalKb != 0 && availableKb != 0) {
            break;
        }
    }

    unsigned long long usedKb = totalKb > availableKb ? totalKb - availableKb : 0;
    double usage = readRamUsageFraction() * 100.0;

    std::ostringstream stream;
    stream << formatPercent(usage) << " (" << formatGiB(usedKb * 1024ULL) << " / " << formatGiB(totalKb * 1024ULL) << ')';

    return stream.str();
}

double System::readRamUsageFraction()
{
    std::ifstream meminfoFile("/proc/meminfo");
    std::string key;
    std::string unit;
    unsigned long long value = 0;
    unsigned long long totalKb = 0;
    unsigned long long availableKb = 0;

    while (meminfoFile >> key >> value >> unit) {
        if (key == "MemTotal:") {
            totalKb = value;
        } else if (key == "MemAvailable:") {
            availableKb = value;
        }

        if (totalKb != 0 && availableKb != 0) {
            break;
        }
    }

    if (totalKb == 0) {
        return 0.0;
    }

    unsigned long long usedKb = totalKb > availableKb ? totalKb - availableKb : 0;
    double usageFraction = static_cast<double>(usedKb) / static_cast<double>(totalKb);

    if (usageFraction < 0.0) {
        return 0.0;
    }
    if (usageFraction > 1.0) {
        return 1.0;
    }

    return usageFraction;
}

std::string System::readDiskUsage()
{
    struct statvfs statBuffer;
    if (statvfs("/", &statBuffer) != 0 || statBuffer.f_blocks == 0) {
        return "unavailable";
    }

    unsigned long long totalBytes = static_cast<unsigned long long>(statBuffer.f_blocks) * statBuffer.f_frsize;
    unsigned long long availableBytes = static_cast<unsigned long long>(statBuffer.f_bavail) * statBuffer.f_frsize;
    unsigned long long usedBytes = totalBytes > availableBytes ? totalBytes - availableBytes : 0;
    double usage = readDiskUsageFraction() * 100.0;

    std::ostringstream stream;
    stream << formatPercent(usage) << " (" << formatGiB(usedBytes) << " / " << formatGiB(totalBytes) << ')';

    return stream.str();
}

double System::readDiskUsageFraction()
{
    struct statvfs statBuffer;
    if (statvfs("/", &statBuffer) != 0 || statBuffer.f_blocks == 0) {
        return 0.0;
    }

    unsigned long long totalBytes = static_cast<unsigned long long>(statBuffer.f_blocks) * statBuffer.f_frsize;
    unsigned long long availableBytes = static_cast<unsigned long long>(statBuffer.f_bavail) * statBuffer.f_frsize;
    unsigned long long usedBytes = totalBytes > availableBytes ? totalBytes - availableBytes : 0;
    double usageFraction = static_cast<double>(usedBytes) / static_cast<double>(totalBytes);

    if (usageFraction < 0.0) {
        return 0.0;
    }
    if (usageFraction > 1.0) {
        return 1.0;
    }

    return usageFraction;
}

std::string System::formatPercent(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value << '%';

    return stream.str();
}

std::string System::formatGiB(unsigned long long bytes)
{
    constexpr double gib = 1024.0 * 1024.0 * 1024.0;

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / gib) << " GiB";

    return stream.str();
}

void System::reboot()
{
	sync();

	if (::reboot(RB_AUTOBOOT) != 0) {
        funcMod::Command::exec("reboot");
    }
}
