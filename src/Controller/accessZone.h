#ifndef __ACCESS_ZONE_H_
#define __ACCESS_ZONE_H_

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "json.hpp"
#include "accessController.h"

using json = nlohmann::json;

class AccessZone
{
private:
    std::string m_zoneName;
    bool m_antipassbackEnabled;
    std::vector<AccessController *> m_controllers;
    
    // Antipassback tracking: badge -> timestamp when entered
    std::map<uint64_t, int64_t> m_enteredBadges;
    mutable std::mutex m_antipassbackMutex;

public:
    AccessZone();
    AccessZone(const std::string &zoneName);
    ~AccessZone();

    void addController(AccessController *controller);
    void removeController(AccessController *controller);

    std::vector<AccessController *> *getControllers();

    std::string getName() const { return this->m_zoneName; }
    void setName(const std::string &name) { this->m_zoneName = name; }

    bool isAntipassbackEnabled() const { return this->m_antipassbackEnabled; }
    void setAntipassbackEnabled(bool enabled) { this->m_antipassbackEnabled = enabled; }

    std::size_t getControllerCount() const { return this->m_controllers.size(); }

    // Antipassback methods
    // Returns true if badge entry was successfully recorded
    bool recordEntry(uint64_t badge, readerIOputType readerType);
    
    // Checks if badge can exit and removes it from tracking if allowed
    // Returns true if exit is allowed
    bool validateAndRecordExit(uint64_t badge, readerIOputType readerType);
    
    // Clear a badge from the entered list
    void clearBadge(uint64_t badge);

    void fromJson(const json &jsonObject);
    json getJson();
};

#endif
