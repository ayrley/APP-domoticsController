#ifndef __READER_H_
#define __READER_H_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>

#include "json.hpp"

#include "action.h"
#include "badge.h"
#include "io.h"
#include "Readers/readerBackend.h"
#include "readerTypes.h"

using json = nlohmann::json;

class Reader
{
private:
    std::string m_readerName;
    std::string m_readerLocation;
    int m_ledDuration;

    std::vector<Badge *> *m_badges;
    Action *m_grantedAction;
    Action *m_deniedAction;

    readerLocationType m_readerLocationType;

    readerType m_readerType;

    std::thread m_runner;
    std::unique_ptr<ReaderBackend> m_backend;
    std::function<void(readerLocationType, const std::string &)> m_errorHandler;

    void handle();
    ReaderDecision onBadgeRead(uint64_t badge);
    void reportError(const std::string &message);

public:
    Reader();
    Reader(std::vector<Badge *> *badges);
    ~Reader();

    void start();
    int init_reader();
    int init_reader(readerType type);
    void setBadges(std::vector<Badge *> *badges);
    void setErrorHandler(const std::function<void(readerLocationType, const std::string &)> &handler);
    void fromJson(const json &jsonObject);

    json getJson();

    std::string getName() { return this->m_readerName; }
    std::string getLocation() { return this->m_readerLocation; }
    readerLocationType getLocationType() { return this->m_readerLocationType; }
    readerType getType() { return this->m_readerType; }
};

#endif
