#include <string>

#include "json.hpp"
#include "debug.h"
#include "serialBus.h"

using json = nlohmann::json;

SerialBus::SerialBus() : m_name("") 
{
}

SerialBus::~SerialBus()
{
}

void SerialBus::setName(const std::string &name)
{
    this->m_name = name;
}

void SerialBus::setBaudRate(int baudRate)
{
    this->m_baudRate = baudRate;
}

void SerialBus::setLocation(const std::string &location)
{
    this->m_location = location;
}

void SerialBus::setType(SerialBusType type)
{
    this->m_type = type;
}

json SerialBus::getJson()
{
    json object = {};
    object += json::object_t::value_type("name", this->m_name);
    object += json::object_t::value_type("location", this->m_location);
    object += json::object_t::value_type("baudrate", this->m_baudRate);
    std::string typeStr = "rs232";
    if (this->m_type == SERIAL_BUS_OSDP) {
        typeStr = "osdp";
    }
    object += json::object_t::value_type("type", typeStr);  

    return object;
}

void SerialBus::fromJson(const json &jsonObject)
{
    try {
        this->m_name = jsonObject["name"];
    } catch (const std::exception &e) {
        this->m_name = "";
        DBG("name could not be found in " + jsonObject.dump());
    }

    try {
        this->m_location = jsonObject["location"];
    } catch (const std::exception &e) {
        this->m_location = "";
        DBG("location could not be found in " + jsonObject.dump());
    }

    try {
        this->m_baudRate = jsonObject["baudrate"];
    } catch (const std::exception &e) {
        this->m_baudRate = 0;
        DBG("baudrate could not be found in " + jsonObject.dump());
    }
    
    try {
       std::string type = jsonObject["type"];
        if (type == "osdp") {
            this->m_type = SERIAL_BUS_OSDP;
        } else if (type == "rs232") {
            this->m_type = SERIAL_BUS_RS232;
        } else {
            this->m_type = SERIAL_BUS_RS232;
        }
    } catch (const std::exception &e) {
        this->m_type = SERIAL_BUS_RS232;
        DBG("type could not be found in " + jsonObject.dump());
    }
}


