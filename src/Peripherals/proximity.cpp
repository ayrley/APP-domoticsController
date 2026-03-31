#include "proximity.h"

#include <iostream>
#include <chrono>
#include <thread>

Proximity::Proximity() {
    m_sensorFile.open(SENSOR_PATH);
    if (!m_sensorFile.is_open()) {
        throw std::runtime_error("Failed to open proximity sensor at " + std::string(SENSOR_PATH));
    }
}

Proximity::~Proximity() {
    if (m_sensorFile.is_open()) {
        m_sensorFile.close();
    }
}

int Proximity::readRawValue() {
    m_sensorFile.clear();
    m_sensorFile.seekg(0);
    
    int value;
    m_sensorFile >> value;
    
    if (m_sensorFile.fail()) {
        throw std::runtime_error("Failed to read proximity sensor value");
    }
    
    return value;
}

bool Proximity::isObjectDetected(int threshold) {
    return readRawValue() > threshold;
}


void Proximity::run()
{
    while (1)
    {
        if (this->isObjectDetected()) {
            std::cout << "Object detected!" << std::endl;
        } else {
            std::cout << "No object detected." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}
void Proximity::start()
{
	this->m_runner = std::thread(&Proximity::run, this);
	this->m_runner.detach();
}
