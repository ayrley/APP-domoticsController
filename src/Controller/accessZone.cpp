#include "accessZone.h"

#include <algorithm>

AccessZone::AccessZone() : m_zoneName("default")
{
}

AccessZone::AccessZone(const std::string &zoneName) : m_zoneName(zoneName)
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

    json readersArray = json::array();
    for (const auto controller : this->m_controllers) {
        readersArray.push_back(controller->getJson());
    }
    zoneJson["readers"] = readersArray;

    return zoneJson;
}
