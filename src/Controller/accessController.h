#ifndef __READER_H_
#define __READER_H_

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <stdint.h>

#include "json.hpp"

#include "action.h"
#include "badge.h"
#include "Readers/readerBackend.h"
#include "Readers/readerTypes.h"

using json = nlohmann::json;

// Forward declaration
class AccessZone;

enum readerIOputType {
    RDR_IN_OUT = 0,
    RDR_IN,
    RDR_OUT
};

class AccessController
{
private:
    std::string m_readerName;
    std::string m_readerLocation;

    int m_ledDuration;

    std::vector<Badge *> *m_badges;
    Action *m_grantedAction;
    Action *m_deniedAction;

    readerLocationType m_readerLocationType;

    readerProtocol m_readerProtocol;
    readerIOputType m_readerIOputType;

    AccessZone *m_zone;

    std::thread m_runner;
    std::unique_ptr<ReaderBackend> m_backend;
    std::function<void(readerLocationType, const std::string &)> m_errorHandler;

    void handle();
    void reportError(const std::string &message);

    ReaderDecision onBadgeRead(uint64_t badge);

public:
    AccessController();
    AccessController(std::vector<Badge *> *badges);
    ~AccessController();

    int initReader();
    int initReader(readerProtocol protocol);
    
    void start();
    void setBadges(std::vector<Badge *> *badges);
    void setErrorHandler(const std::function<void(readerLocationType, const std::string &)> &handler);
    void setZone(AccessZone *zone) { this->m_zone = zone; }
    void fromJson(const json &jsonObject);

    json getJson();

    std::string getName() { return this->m_readerName; }
    std::string getLocation() { return this->m_readerLocation; }

    readerLocationType getLocationType() { return this->m_readerLocationType; }

    readerProtocol getProtocol() { return this->m_readerProtocol; }

    readerIOputType getIOputType() { return this->m_readerIOputType; }
    
    AccessZone *getZone() { return this->m_zone; }
};

#endif
