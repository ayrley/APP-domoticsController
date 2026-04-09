#include "accessZone.h"

#include <algorithm>
#include <chrono>

AccessZone::AccessZone() : m_zoneName("default"), m_antipassbackEnabled(false)
{
}

AccessZone::AccessZone(const std::string &zoneName) : m_zoneName(zoneName), m_antipassbackEnabled(false)
{
}

AccessZone::~AccessZone()
{
    for (auto controller : this->m_controllers) {
        delete controller;
    }
    this->m_controllers.clear();
}

void AccessZone::addController(AccessController *controller)
{
    if (controller != nullptr) {
        controller->setZone(this);
        this->m_controllers.push_back(controller);
    }
}

void AccessZone::removeController(AccessController *controller)
{
    auto it = std::find(this->m_controllers.begin(), this->m_controllers.end(), controller);
    if (it != this->m_controllers.end()) {
        this->m_controllers.erase(it);
    }
}

std::vector<AccessController *> *AccessZone::getControllers()
{
    return &this->m_controllers;
}

void AccessZone::fromJson(const json &jsonObject)
{
    if (jsonObject.contains("name")) {
        this->m_zoneName = jsonObject["name"];
    }

    if (jsonObject.contains("antipassback")) {
        this->m_antipassbackEnabled = jsonObject["antipassback"];
    }

    if (jsonObject.contains("readers") && jsonObject["readers"].is_array()) {
        for (const auto &readerJson : jsonObject["readers"]) {
            AccessController *controller = new AccessController();
            controller->fromJson(readerJson);
            this->addController(controller);
        }
    }
}

json AccessZone::getJson()
{
    json zoneJson = {};
    zoneJson["name"] = this->m_zoneName;
    zoneJson["antipassback"] = this->m_antipassbackEnabled;

    json readersArray = json::array();
    for (const auto controller : this->m_controllers) {
        readersArray.push_back(controller->getJson());
    }
    zoneJson["readers"] = readersArray;

    return zoneJson;
}

bool AccessZone::recordEntry(uint64_t badge, readerIOputType readerType)
{
    if (readerType != RDR_IN && readerType != RDR_IN_OUT) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->m_antipassbackMutex);
    
    int64_t now = std::chrono::system_clock::now().time_since_epoch().count();
    this->m_enteredBadges[badge] = now;
    
    return true;
}

bool AccessZone::validateAndRecordExit(uint64_t badge, readerIOputType readerType)
{
    if (readerType != RDR_OUT && readerType != RDR_IN_OUT) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->m_antipassbackMutex);
    
    // Check if badge is in the entered list
    auto it = this->m_enteredBadges.find(badge);
    if (it == this->m_enteredBadges.end()) {
        // Badge was never entered - antipassback violation
        return false;
    }

    // Remove badge from entered list
    this->m_enteredBadges.erase(it);
    return true;
}

void AccessZone::clearBadge(uint64_t badge)
{
    std::lock_guard<std::mutex> lock(this->m_antipassbackMutex);
    this->m_enteredBadges.erase(badge);
}
