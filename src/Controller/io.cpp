#include <cerrno>
#include <cstdlib>
#include <string>

#include <stdint.h>

#include <system/File.hpp>

#include "debug.h"
#include "io.h"

IO::IO()
{
    this->m_exported = false;
}

IO::~IO()
{
}

int IO::exportIo()
{
    std::string gpioExportLocation = "";
    std::string exportLocation = "/sys/class/gpio/export";
    std::string gpioNumber;

    if (this->m_direction == IO_DIR_AIN) {
        this->m_exported = true;
        return 0;
    }

    gpioNumber = this->m_location.substr(this->m_location.find("gpio") + 1);

    File::writeFile(exportLocation, gpioNumber);
    File::writeFile(this->m_location + "/direction", this->m_direction == IO_DIR_IN ? "in" : "out");

    return 0;
}

void IO::clear()
{
    this->set(false);
}

void IO::set()
{
    this->set(true);
}

void IO::set(bool high)
{
    std::string gpioValue = this->m_location + "/value";

    if (!this->m_exported)
        this->exportIo();

    File::writeFile(gpioValue, high ? "1" : "0");
}

int IO::getAnalogIo()
{
    std::string buffer;

    if (!File::exists(this->m_location)) {
        DBG("Analog IO location does not exist: " + this->m_location);
        return 0;
    }

    File::catFile(this->m_location, buffer);

    int parsedValue = stoi(buffer);
    if (parsedValue < 0) {
        DBG("Analog IO parse failed for location: " + this->m_location +
            ", raw='" + buffer + "'");
    }

    return parsedValue;
}

int IO::getDigitalIo()
{
    std::string buffer;

    std::string gpioValue = this->m_location + "/value";

     if (!File::exists(this->m_location)) {
        DBG("Digital IO location does not exist: " + this->m_location);
        return 0;
    }

    File::catFile(gpioValue, buffer);

    int parsedValue = stoi(buffer);
    if (parsedValue < 0) {
        DBG("Digital IO parse failed for location: " + gpioValue +
            ", raw='" + buffer + "'");
    }

    return parsedValue;
}

int IO::get()
{
    if (!this->m_exported)
        this->exportIo();

    if (this->m_direction == IO_DIR_AIN)
        return this->getAnalogIo();
    else if (this->m_direction == IO_DIR_IN)
        return this->getDigitalIo();

    return -1;
}

json IO::getJson()
{
    json object = {};
    object += json::object_t::value_type("name", this->m_name);
    object += json::object_t::value_type("location", this->m_location);

    std::string locationTypeStr;
    if (this->m_locationType == IO_LOC_LOCAL)
        locationTypeStr = "LOCAL";
    else if (this->m_locationType == IO_LOC_IP)
        locationTypeStr = "IP";
    object += json::object_t::value_type("type", locationTypeStr);

    std::string directionStr;
    if (this->m_direction == IO_DIR_IN)
        directionStr = "IN";
    else if (this->m_direction == IO_DIR_AIN)
        directionStr = "AIN";
    else if (this->m_direction == IO_DIR_OUT)
        directionStr = "OUT";
    object += json::object_t::value_type("direction", directionStr);

    return object;
}

void IO::fromJson(const json &jsonObject)
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
        std::string locationType = jsonObject["type"];
        if (locationType == "LOCAL")
            this->m_locationType = IO_LOC_LOCAL;
        else if (locationType == "IP")
            this->m_locationType = IO_LOC_IP;
    } catch (const std::exception &e) {
        this->m_locationType = IO_LOC_LOCAL;
        DBG("type could not be found in " + jsonObject.dump());
    }

    try {
        std::string direction = jsonObject["direction"];
        if (direction == "AIN")
            this->m_direction = IO_DIR_AIN;
        else if (direction == "IN")
            this->m_direction = IO_DIR_IN;
        else if (direction == "OUT")
            this->m_direction = IO_DIR_OUT;
    } catch (const std::exception &e) {
        this->m_direction = IO_DIR_IN;
        DBG("direction could not be found in " + jsonObject.dump());
    }
}
