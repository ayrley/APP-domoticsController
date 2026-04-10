#include <osdp.hpp>

#include "osdpReaderBackend.h"
#include "debug.h"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <iostream>
#include <thread>

#include <hardware/Uart.hpp>

bool OsdpReaderBackend::decodeCardReadToBadge(const osdp_event_cardread &cardRead, uint64_t &badge)
{
    badge = 0;

    if (cardRead.format == OSDP_CARD_FMT_ASCII) {
        if (cardRead.length <= 0 || cardRead.length > OSDP_EVENT_CARDREAD_MAX_DATALEN) {
            return false;
        }

        for (int i = 0; i < cardRead.length; ++i) {
            const unsigned char ch = cardRead.data[i];
            if (!std::isdigit(ch)) {
                return false;
            }

            badge = (badge * 10u) + static_cast<uint64_t>(ch - '0');
        }

        return true;
    }

    if (cardRead.format != OSDP_CARD_FMT_RAW_UNSPECIFIED &&
        cardRead.format != OSDP_CARD_FMT_RAW_WIEGAND) {
        return false;
    }

    if (cardRead.length <= 0 || cardRead.length > 64) {
        return false;
    }

    for (int i = 0; i < cardRead.length; ++i) {
        const int bitIndex = (cardRead.direction == 0)
                                 ? i
                                 : (cardRead.length - 1 - i);
        const int byteIndex = bitIndex / 8;
        const int bitInByte = 7 - (bitIndex % 8);
        const uint8_t bit = (cardRead.data[byteIndex] >> bitInByte) & 0x01;

        badge = (badge << 1u) | static_cast<uint64_t>(bit);
    }

    return true;
}

OsdpReaderBackend::OsdpReaderBackend(const std::string &bus) :
    ReaderBackend(), OSDP::ControlPanel(),
    m_bus(bus),
    m_baudRate(115200)
{
    this->m_channel.data = this;
    this->m_channel.send = [](void *data, uint8_t *buf, int len) {
        return static_cast<OsdpReaderBackend *>(data)->send(data, buf, len);
    };
    this->m_channel.recv = [](void *data, uint8_t *buf, int len) {
        return static_cast<OsdpReaderBackend *>(data)->recv(data, buf, len);
    };
    this->m_channel.flush = nullptr;
    this->m_channel.close = nullptr;

    this->logger_init(("osdp::bus" + m_bus).c_str(), OSDP_LOG_DEBUG, NULL);

    this->m_uart = new funcmod::Uart(this->m_bus, this->m_baudRate);
}

void OsdpReaderBackend::addReader(int address)
{
    this->addReader(address, "rdr[" + std::to_string(address) + "]");
}

void OsdpReaderBackend::setBaudRate(int baudRate)
{
    this->m_baudRate = baudRate;
}

void OsdpReaderBackend::addReader(int address, const std::string &name)
{
    bool shouldStartRunner = false;

    osdp_pd_info_t reader = {
        .name = name.c_str(),
        .baud_rate = this->m_baudRate,
        .address = address,
        .flags = 0,
        .id = {},
        .cap = nullptr,
        .scbk = nullptr,
    };

    if (this->m_running) {
        shouldStartRunner = true;
        this->m_running = false;
    }

    this->m_readers.push_back(reader);

    if (shouldStartRunner) {
        this->m_running = true;
    }
}

int OsdpReaderBackend::send(void *data, uint8_t *buf, int len)
{
    this->m_uart->writeNonBlocking(buf, len);

    return len;
}

int OsdpReaderBackend::recv(void *data, uint8_t *buf, int len)
{
    this->m_uart->readNonBlocking(buf, len);

    return 0;
}

int OsdpReaderBackend::event(void *data, int pd, struct osdp_event *event)
{
    if (event == nullptr) {
        return 0;
    }

    std::cout << "PD" << pd << " EVENT: " << event->type << std::endl;

    if (event->type != OSDP_EVENT_CARDREAD) {
        return 0;
    }

    if (this->m_onBadgeRead == nullptr) {
        DBG("OSDP card read ignored: no badge callback registered");
        return 0;
    }

    uint64_t badge = 0;
    if (!this->decodeCardReadToBadge(event->cardread, badge)) {
        DBG("OSDP card read ignored: unable to decode badge payload");
        return -EINVAL;
    }

    (*this->m_onBadgeRead)(badge);

    return 0;
}

void OsdpReaderBackend::run()
{
    if (this->m_readers.size() > 0)
        this->setup(&m_channel, this->m_readers.size(), this->m_readers.data());
    else
        this->setup(&m_channel, 0, nullptr);

    this->set_event_callback([](void *data, int pd, struct osdp_event *event) {
        return static_cast<OsdpReaderBackend *>(data)->event(data, pd, event);
    },
    this);

    this->m_running = true;

    while (this->m_running) {
        this->refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void OsdpReaderBackend::run(const BadgeReadCallback &onBadgeRead,
                            const ErrorCallback &onError)
{
    this->m_onBadgeRead = &onBadgeRead;
    this->m_onError = &onError;

    this->run();
}
