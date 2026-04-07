#ifndef __IO_H_
#define __IO_H_

#include <memory>
#include <mutex>
#include <string>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

enum ioLocationType {
    IO_LOC_LOCAL = 0,
    IO_LOC_IP
};

enum ioDirection {
    IO_DIR_IN = 0,
    IO_DIR_AIN,
    IO_DIR_OUT
};

class IO
{
private:
    struct NetworkInputState {
        std::mutex mtx;
        int currentValue = 0;
        int pendingValue = 0;
        bool pending = false;
        bool listenerStarted = false;
    };

    std::string m_name;
    std::string m_location;
    std::string m_fullLocation;

    enum ioLocationType m_locationType;
    enum ioDirection m_direction;

    bool m_exported;
    std::shared_ptr<NetworkInputState> m_networkInputState;

    int exportIo();
    int getDigitalIo();
    int getAnalogIo();
    void startNetworkInputListener();

public:
    IO();
    ~IO();

    std::string getName() { return this->m_name; }
    std::string getLocation() { return this->m_location; }
    enum ioLocationType getLocationType() { return this->m_locationType; }
    enum ioDirection getDirection() { return this->m_direction; }

    void fromJson(const json &jsonObject);
    void clear();
    void set();
    void set(bool high);
    void start();

    json getJson();

    int get();
};

#endif
