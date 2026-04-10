#ifndef __SERIAL_BUS_H_
#define __SERIAL_BUS_H_

#include <string>

#include <stdint.h>

#include "json.hpp"

using json = nlohmann::json;

enum SerialBusType {
    SERIAL_BUS_OSDP,
    SERIAL_BUS_RS232
};

class SerialBus
{
private:
    std::string m_name;;
    std::string m_location;
    int m_baudRate;

    SerialBusType m_type;

public:
    SerialBus();
    ~SerialBus();

    void setName(const std::string &name);
    void setLocation(const std::string &location);
    void setBaudRate(int baudRate);
    void setType(SerialBusType type);
    void fromJson(const json &jsonObject);

    json getJson();

    std::string getName() const { return m_name; }
    std::string getLocation() const { return m_location; }

    int getBaudRate() const { return m_baudRate; }

    SerialBusType getType() const { return m_type; }
};

#endif

