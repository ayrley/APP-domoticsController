#ifndef __ACCESS_ZONE_H_
#define __ACCESS_ZONE_H_

#include <string>
#include <vector>

#include "json.hpp"
#include "accessController.h"

using json = nlohmann::json;

class AccessZone
{
private:
    std::string m_zoneName;
    std::vector<AccessController *> m_controllers;

public:
    AccessZone();
    AccessZone(const std::string &zoneName);
    ~AccessZone();

    void addController(AccessController *controller);
    void removeController(AccessController *controller);

    std::vector<AccessController *> *getControllers();

    std::string getName() const { return this->m_zoneName; }
    void setName(const std::string &name) { this->m_zoneName = name; }

    std::size_t getControllerCount() const { return this->m_controllers.size(); }

    void fromJson(const json &jsonObject);
    json getJson();
};

#endif
