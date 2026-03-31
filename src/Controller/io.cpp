#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include "debug.h"
#include "io.h"

namespace
{
int parseIntOrMinusOne(const char *buffer)
{
    if (!buffer || buffer[0] == '\0') {
        return -1;
    }

    char *endPtr = nullptr;
    errno = 0;
    long value = std::strtol(buffer, &endPtr, 10);

    if (endPtr == buffer || errno != 0) {
        return -1;
    }

    return static_cast<int>(value);
}
} // namespace

IO::IO()
{
    this->m_exported = false;
}

IO::~IO()
{
}

int IO::exportIo()
{
    int fptr;
    std::string gpioExportLocation = "";
    std::string exportLocation = "/sys/class/gpio/export";
    std::string gpioNumber;

    if (this->m_direction == IO_DIR_AIN) {
        this->m_exported = true;
        return 0;
    }

    gpioNumber = this->m_location.substr(this->m_location.find("gpio") + 1);

    fptr = open(exportLocation.c_str(), O_WRONLY);
    if (!fptr)
        return fptr;

    write(fptr, gpioNumber.c_str(), strlen(gpioNumber.c_str()));
    close(fptr);

    gpioExportLocation = this->m_location + "/direction";
    fptr = open(gpioExportLocation.c_str(), O_WRONLY);

    if (this->m_direction == IO_DIR_IN)
        write(fptr, "in", 2);
    else if (this->m_direction == IO_DIR_OUT)
        write(fptr, "out", 3);

    close(fptr);

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
    int fptr;
    std::string gpioValue = this->m_location + "/value";

    if (!this->m_exported)
        this->exportIo();

    fptr = open(gpioValue.c_str(), O_WRONLY);
    if (!fptr)
        return;

    write(fptr, high ? "1" : "0", 1);
    close(fptr);
}

int IO::getAnalogIo()
{
    int fptr;
    char buffer[16] = {0};

    fptr = open(this->m_location.c_str(), O_RDONLY);
    if (!fptr)
        return -1;

    ssize_t bytesRead = read(fptr, buffer, 8);
    close(fptr);

    if (bytesRead <= 0) {
        DBG("Analog IO read failed for location: " + this->m_location);
        return -1;
    }

    int parsedValue = parseIntOrMinusOne(buffer);
    if (parsedValue < 0) {
        DBG("Analog IO parse failed for location: " + this->m_location +
            ", raw='" + std::string(buffer) + "'");
    }

    return parsedValue;
}

int IO::getDigitalIo()
{
    int fptr;
    char buffer[16] = {0};
    std::string gpioValue = this->m_location + "/value";

    fptr = open(gpioValue.c_str(), O_RDONLY);
    if (!fptr)
        return -1;

    ssize_t bytesRead = read(fptr, buffer, 5);
    close(fptr);

    if (bytesRead <= 0) {
        DBG("Digital IO read failed for location: " + gpioValue);
        return -1;
    }

    int parsedValue = parseIntOrMinusOne(buffer);
    if (parsedValue < 0) {
        DBG("Digital IO parse failed for location: " + gpioValue +
            ", raw='" + std::string(buffer) + "'");
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
