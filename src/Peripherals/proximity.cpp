#include "proximity.h"

#include <chrono>
#include <iostream>
#include <thread>

Proximity::Proximity()
{
    m_sensorFile.open(SENSOR_PATH);
    if (!m_sensorFile.is_open()) {
        throw std::runtime_error("Failed to open proximity sensor at " + std::string(SENSOR_PATH));
    }
}

Proximity::~Proximity()
{
    stop();

    if (m_sensorFile.is_open()) {
        m_sensorFile.close();
    }
}

int Proximity::readRawValue()
{
    m_sensorFile.clear();
    m_sensorFile.seekg(0);

    int value;
    m_sensorFile >> value;

    if (m_sensorFile.fail()) {
        throw std::runtime_error("Failed to read proximity sensor value");
    }

    return value;
}

bool Proximity::isObjectDetected(int threshold)
{
    return readRawValue() > threshold;
}

void Proximity::setDetectionHandler(std::function<void(bool)> handler)
{
    m_detectionHandler = std::move(handler);
}

void Proximity::run()
{
    while (m_running.load()) {
        try {
            const bool detected = isObjectDetected();
            const bool stateChanged = !m_hasLastDetectionState || detected != m_lastDetectionState;
            if (stateChanged) {
                m_lastDetectionState = detected;
                m_hasLastDetectionState = true;
                std::cout << (detected ? "Object detected!" : "No object detected.") << std::endl;
            }
            if (m_detectionHandler) {
                m_detectionHandler(detected);
            }
        } catch (const std::exception &error) {
            std::cerr << "Proximity read failed: " << error.what() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void Proximity::start()
{
    if (m_running.exchange(true)) {
        return;
    }

    this->m_runner = std::thread(&Proximity::run, this);
}

void Proximity::stop()
{
    if (!m_running.exchange(false)) {
        return;
    }

    if (m_runner.joinable()) {
        m_runner.join();
    }
}
