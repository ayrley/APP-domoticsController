#ifndef __PROXIMITY_H
#define __PROXIMITY_H

#include <fstream>
#include <string>
#include <stdexcept>
#include <thread>

#define SENSOR_PATH "/sys/class/iio/device1/in_proximity_raw"

class Proximity {
private:
    std::ifstream m_sensorFile;
    std::thread m_runner;

    void run();

public:
    Proximity();
    ~Proximity();

    int readRawValue();

    bool isObjectDetected(int threshold = 100);
    void start();

};

#endif // __PROXIMITY_H