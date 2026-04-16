#ifndef __OSDP_READER_BACKEND_H_
#define __OSDP_READER_BACKEND_H_

#include <memory>
#include <string>
#include <vector>

#include "osdpBus.h"
#include "readerBackend.h"

class OsdpReaderBackend : public ReaderBackend
{
private:
    struct ReaderRegistration {
        int address;
        std::string name;
    };

    std::string m_busIdentifier;
    std::vector<ReaderRegistration> m_readers;

    int m_ledDuration;
    int m_baudRate;

    std::shared_ptr<OsdpBus> m_bus;

    static bool parseBusSpec(const std::string &busSpec,
                             std::string &busIdentifier,
                             int &readerAddress);

    static std::string defaultReaderName(int address);

public:
    OsdpReaderBackend(const std::string &bus, int ledDuration);

    void run(const BadgeReadCallback &onBadgeRead,
             const ErrorCallback &onError) override;

    void addReader(int address);
    void addReader(int address, const std::string &name);

    void setBaudRate(int baudRate);

    int getBaudRate() const { return m_baudRate; }
    int getReaderCount() const { return static_cast<int>(m_readers.size()); }
};

#endif
