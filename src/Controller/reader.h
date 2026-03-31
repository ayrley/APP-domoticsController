#ifndef __READER_H_
#define __READER_H_

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

    void handle();
    void handleLocalReader();
    void handleWiegandReader();
    void handleOsdpReader();
    void handleNetworkReader();
    void setWiegandLed(enum ledColor color);

    int getWiegandBadge(uint64_t *badge);
    int getKeypad(uint64_t *cardNumber, int count);
    int plainTextCode(char *binaryCardString, uint64_t *cardNumber, int length);

public:
    Reader();
    Reader(std::vector<Badge *> *badges);
    ~Reader();

    void start();
    void setBadges(std::vector<Badge *> *badges);
    void fromJson(const json &jsonObject);

    json getJson();

    std::string getName();
    std::string getLocation();

    readerLocationType getLocationType();

    readerType getType();
};

#endif
