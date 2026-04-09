#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#include <cerrno>
#include <cstdlib>

#include <system/File.hpp>

#include "debug.h"
#include "wiegandReaderBackend.h"

WiegandReaderBackend::WiegandReaderBackend(const std::string &readerLocation,
                                           int ledDuration)
{
    this->m_readerLocation = readerLocation;
    this->m_ledDuration = ledDuration;
    this->m_keypadBytes = 0;
    memset(this->m_keypad, 0, sizeof(this->m_keypad));
}

int WiegandReaderBackend::plainTextCode(const char *binaryCardString,
                                        uint64_t *cardNumber,
                                        int length)
{
    int i;
    uint64_t bit;
    uint64_t newCardNumber = 0;
    int offset = 0;
    uint64_t swappedCardNumber = 0;

    if (length > 64)
        offset = length - 64 - 1;

    for (i = offset; i < length; i++) {
        bit = ((binaryCardString[i] == '0') ||
               (binaryCardString[i] && 0x01 == 0))
                  ? 0x00
                  : 0x01;
        newCardNumber = newCardNumber << 1;
        newCardNumber = newCardNumber & 0xfffffffffffffffe;
        newCardNumber |= (bit & 0x01);
    }

    swappedCardNumber = newCardNumber;
    if (*cardNumber != swappedCardNumber && swappedCardNumber != 0)
        *cardNumber = swappedCardNumber;

    return 0;
}

int WiegandReaderBackend::getKeypad(uint64_t *cardNumber, int count)
{
    char tmpKeypad[2] = {0};

    if (this->m_keypadBytes >= 10) {
        *cardNumber = atoi(this->m_keypad);
    } else if (count <= 8) {
        tmpKeypad[0] = (*cardNumber & 0x0f) + 48;

        if (((*cardNumber) & 0x0f) > 9) {
            *cardNumber = atoi(this->m_keypad);
            this->m_keypadBytes = 0;
            memset(this->m_keypad, 0, 12);
        } else {
            *cardNumber = 0;
            strncat(this->m_keypad, tmpKeypad, 1);
            this->m_keypadBytes++;
        }
    }

    return 0;
}

int WiegandReaderBackend::getWiegandBadge(uint64_t *badge)
{
    std::string wiegandString;
    std::string buffer;
    int count;
    int readVal;
    uint64_t cardNumber = 0;

    std::string devicePath = "/dev/" + this->m_readerLocation;
    std::string countPath = "/sys/class/idtech/" + this->m_readerLocation +
                            "/device/count";

    if (!File::exists(countPath)) {
        DBG("Wiegand reader count path does not exist: " + countPath);
        *badge = 0;
        return -EINVAL;
    }

    if (!File::exists(devicePath)) {
        DBG("Wiegand reader device path does not exist: " + devicePath);
        *badge = 0;
        return -EINVAL;
    }

    File::catFile(countPath, buffer);
    count = stol(buffer);

    File::catFile(devicePath, wiegandString);
    readVal = wiegandString.size();

    if (readVal < 0)
        return -EINVAL;

    this->plainTextCode(wiegandString.c_str(), &cardNumber, count);
    this->getKeypad(&cardNumber, count);
    *badge = cardNumber;

    return 0;
}

void WiegandReaderBackend::setWiegandLed(enum ledColor color)
{
    std::string ledPath = "/sys/class/idtech/" + this->m_readerLocation +
                          "/device/color";

    File::writeFile(ledPath, std::to_string(color));
    std::this_thread::sleep_for(std::chrono::milliseconds(this->m_ledDuration));
    File::writeFile(ledPath, "0");
}

void WiegandReaderBackend::run(const BadgeReadCallback &onBadgeRead,
                               const ErrorCallback &)
{
    uint64_t badge = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (this->getWiegandBadge(&badge))
            continue;

        if (!badge)
            continue;

        ReaderDecision decision = onBadgeRead(badge);
        enum ledColor color = decision.granted ? LED_GREEN : LED_RED;

        std::thread ledRunner(&WiegandReaderBackend::setWiegandLed, this, color);
        ledRunner.detach();
    }
}
