#include "osdpReaderBackend.h"
#include "debug.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>

namespace {
bool isAllDigits(const std::string &value)
{
    if (value.empty()) {
        return false;
    }

    return std::all_of(value.begin(), value.end(), [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) != 0;
    });
}
} // namespace

OsdpReaderBackend::OsdpReaderBackend(const std::string &bus, int ledDuration) :
    ReaderBackend(),
    m_busIdentifier(bus),
    m_baudRate(115200),
    m_ledDuration(ledDuration)
{
    int parsedAddress = 0;
    std::string parsedBusIdentifier;
    if (parseBusSpec(bus, parsedBusIdentifier, parsedAddress)) {
        m_busIdentifier = parsedBusIdentifier;
        addReader(parsedAddress, defaultReaderName(parsedAddress));
    }
}

bool OsdpReaderBackend::parseBusSpec(const std::string &busSpec,
                                     std::string &busIdentifier,
                                     int &readerAddress)
{
    busIdentifier = busSpec;
    readerAddress = 0;

    const size_t atPos = busSpec.rfind('@');
    const size_t colonPos = busSpec.rfind(':');
    size_t splitPos = std::string::npos;

    if (atPos != std::string::npos) {
        splitPos = atPos;
    } else if (colonPos != std::string::npos) {
        splitPos = colonPos;
    }

    if (splitPos == std::string::npos || splitPos + 1 >= busSpec.size()) {
        return false;
    }

    const std::string maybeAddress = busSpec.substr(splitPos + 1);
    if (!isAllDigits(maybeAddress)) {
        return false;
    }

    busIdentifier = busSpec.substr(0, splitPos);
    readerAddress = std::stoi(maybeAddress);
    return !busIdentifier.empty();
}

std::string OsdpReaderBackend::defaultReaderName(int address)
{
    return "rdr[" + std::to_string(address) + "]";
}

void OsdpReaderBackend::addReader(int address)
{
    addReader(address, defaultReaderName(address));
}

void OsdpReaderBackend::setBaudRate(int baudRate)
{
    m_baudRate = baudRate;
}

void OsdpReaderBackend::addReader(int address, const std::string &name)
{
    auto existing = std::find_if(m_readers.begin(), m_readers.end(),
                                 [address](const ReaderRegistration &reader) {
                                     return reader.address == address;
                                 });

    if (existing != m_readers.end()) {
        existing->name = name;
        return;
    }

    m_readers.push_back({address, name});
}

void OsdpReaderBackend::run(const BadgeReadCallback &onBadgeRead,
                            const ErrorCallback &onError)
{
    if (m_readers.empty()) {
        addReader(0);
    }

    m_bus = OsdpBus::getOrCreate(m_busIdentifier, m_baudRate);
    if (m_bus == nullptr) {
        if (onError) {
            onError("Unable to create OSDP bus for '" + m_busIdentifier + "'");
        }
        return;
    }

    for (const ReaderRegistration &reader : m_readers) {
        m_bus->registerReader(reader.address,
                              reader.name,
                              m_baudRate,
                              m_ledDuration,
                              onBadgeRead,
                              onError);
    }
}
