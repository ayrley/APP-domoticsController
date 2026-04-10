#ifndef __OSDP_READER_BACKEND_H_
#define __OSDP_READER_BACKEND_H_

#include <osdp.hpp>
#include <hardware/Uart.hpp>

#include "readerBackend.h"

class OsdpReaderBackend : public ReaderBackend, public OSDP::ControlPanel
{
private:
    std::string m_bus;

    std::vector<osdp_pd_info_t> m_readers;

    bool m_updated;
    bool m_running;

    int m_baudRate;

    funcmod::Uart *m_uart;

    osdp_channel m_channel;

    const BadgeReadCallback *m_onBadgeRead;
    const ErrorCallback *m_onError;

    int send(void *data, uint8_t *buf, int len);
    int recv(void *data, uint8_t *buf, int len);
    int event(void *data, int pd, struct osdp_event *event);

    bool decodeCardReadToBadge(const osdp_event_cardread &cardRead, uint64_t &badge);

    void run();

public:
    OsdpReaderBackend(const std::string &bus);

    void run(const BadgeReadCallback &onBadgeRead,
             const ErrorCallback &onError) override;

    void addReader(int address);
    void addReader(int address, const std::string &name);

    void setBaudRate(int baudRate);

    int getBaudRate() const { return m_baudRate; }
    int getReaderCount() const { return m_readers.size(); }
};

#endif
