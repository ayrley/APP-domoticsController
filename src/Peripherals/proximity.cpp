#include "proximity.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <debug.h>

Proximity::Proximity()
    : m_lastDetectionState(true),
      m_hasLastDetectionState(true)
{
    try {
        m_sensorFile.open(SENSOR_PATH);
        if (!m_sensorFile.is_open()) {
            throw std::runtime_error("Proximity sensor device not found");
        }
        m_isAvailable = true;
    } catch (const std::exception &e) {
        ERR("Proximity sensor unavailable, using mock mode (always detected)");
        m_isAvailable = false;
        m_lastDetectionState = true;
        m_hasLastDetectionState = true;
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
    if (!m_isAvailable) {
        return 50;
    }

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

void Proximity::resetAbsenceGrace()
{
    m_graceResetRequested = true;
}

void Proximity::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr auto kAbsenceGracePeriod = std::chrono::seconds(5);

    bool absenceGracePending = false;
    Clock::time_point absenceStart;

    while (m_running.load()) {
        try {
            if (m_graceResetRequested.exchange(false) && absenceGracePending) {
                absenceStart = Clock::now();
            }

            const bool detected = isObjectDetected();
            const bool stateChanged = !m_hasLastDetectionState || detected != m_lastDetectionState;
            if (stateChanged) {
                m_lastDetectionState = detected;
                m_hasLastDetectionState = true;
                LOG((detected ? "Object detected!" : "No object detected."));
            }

            if (detected) {
                absenceGracePending = false;
                if (m_detectionHandler) {
                    m_detectionHandler(true);
                }
            } else {
                if (!absenceGracePending) {
                    absenceGracePending = true;
                    absenceStart = Clock::now();
                } else if (Clock::now() - absenceStart >= kAbsenceGracePeriod) {
                    if (m_detectionHandler) {
                        m_detectionHandler(false);
                    }
                }
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
