#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <string>

class System
{
public:
    struct CpuSample {
        unsigned long long idle = 0;
        unsigned long long total = 0;
    };

    static CpuSample readCpuSample();

    static std::string formatCpuUsage(const CpuSample &previousSample, const CpuSample &currentSample);
    static double cpuUsageFraction(const CpuSample &previousSample, const CpuSample &currentSample);
    static std::string readRamUsage();
    static double readRamUsageFraction();
    static std::string readDiskUsage();
    static double readDiskUsageFraction();

    static void reboot();

private:
    static std::string formatPercent(double value);
    static std::string formatGiB(unsigned long long bytes);
};

#endif
