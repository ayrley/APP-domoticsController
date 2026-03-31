#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <mutex>
#include <string>
#include <vector>

#include <stdint.h>

#include "json.hpp"

#include "network.h"
#include "reader.h"

using json = nlohmann::json;

enum settingsType {
    SET_NONE = 0,
    SET_FACTORY,
    SET_USER
};

std::string getValueString(json prop);

class Settings
{
private:
    std::string m_settingsFile;
    json m_settings;

    std::mutex m_mtx;

    enum settingsType settingsType;

    int parseSettingsType();
    int parseReaders();
    int parseActions();

    Network *m_network;

    std::vector<Reader *> *m_readers;
    std::vector<Action *> *m_actions;

public:
    Settings();
    Settings(std::string settingsFile);
    ~Settings();

    int setSettingsFile(std::string settingsFile);

    int read();
    int read(std::string settingsFile);
    int write();
    int write(std::string settingsFile);

    int version();

    Network *getNetwork();
    std::vector<Reader *> *getReaders();
    std::vector<Action *> *getActions();

    enum settingsType getType();

    void setType(enum settingsType);
    void parse();

    json getFromSettings(const std::string &key);
};

#endif
