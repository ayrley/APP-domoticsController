#ifndef __PROXIMITY_H
#define __PROXIMITY_H

#include <atomic>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#define SENSOR_PATH "/sys/class/iio/device1/in_proximity_raw"

class Proximity
{
private:
    std::ifstream m_sensorFile;
    std::thread m_runner;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_graceResetRequested{false};
    std::function<void(bool)> m_detectionHandler;
    bool m_lastDetectionState{false};
    bool m_hasLastDetectionState{false};
    bool m_isAvailable{false};

    void run();

public:
    Proximity();
    ~Proximity();

    int readRawValue();

    bool isObjectDetected(int threshold = 100);
    bool isAvailable() const { return m_isAvailable; }
    void setDetectionHandler(std::function<void(bool)> handler);
    void resetAbsenceGrace();
    void start();
    void stop();
};

#endif // __PROXIMITY_H
