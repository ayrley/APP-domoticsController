#ifndef __READER_H_
#define __READER_H_

#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>

#include "json.hpp"

#include "action.h"
#include "badge.h"
#include "io.h"

using json = nlohmann::json;

enum readerLocationType {
    RDR_LOC_LOCAL = 0,
    RDR_LOC_IP
};

enum readerType {
    RDR_WIEGAND = 0,
    RDR_OSDP
};

enum ledColor {
    LED_NONE = 0,
    LED_RED,
    LED_GREEN,
    LED_YELLOW
};

class Reader
{
private:
    std::string m_readerName;
    std::string m_readerLocation;

    char m_keypad[12];

    int m_keypadBytes;
    int m_ledDuration;
    int m_ledPassedTime;

    std::vector<Badge *> *m_badges;
    Action *m_grantedAction;
    Action *m_deniedAction;

    readerLocationType m_readerLocationType;

    readerType m_readerType;

    std::thread m_runner;
    std::function<void(readerLocationType, const std::string &)> m_errorHandler;

    void handle();
    void handleLocalReader();
    void handleWiegandReader();
    void handleOsdpReader();
    void handleNetworkReader();
    void reportError(const std::string &message);
    void setWiegandLed(enum ledColor color);

    int getWiegandBadge(uint64_t *badge);
    int getKeypad(uint64_t *cardNumber, int count);
    int plainTextCode(const char *binaryCardString, uint64_t *cardNumber, int length);

public:
    Reader();
    Reader(std::vector<Badge *> *badges);
    ~Reader();

    void start();
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
