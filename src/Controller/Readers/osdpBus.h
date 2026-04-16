#ifndef __OSDP_BUS_H_
#define __OSDP_BUS_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <osdp.hpp>
#include <hardware/Uart.hpp>

#include "readerBackend.h"

class OsdpBus : public OSDP::ControlPanel
{
public:
    using BadgeReadCallback = ReaderBackend::BadgeReadCallback;
    using ErrorCallback = ReaderBackend::ErrorCallback;

    static std::shared_ptr<OsdpBus> getOrCreate(const std::string &busIdentifier,
                                                int requestedBaudRate);

    ~OsdpBus();

    int registerReader(int address,
                       const std::string &name,
                       int baudRate,
                       int ledDuration,
                       const BadgeReadCallback &onBadgeRead,
                       const ErrorCallback &onError);

    std::string getBusName() const { return m_busName; }
    std::string getBusLocation() const { return m_busLocation; }
    int getBaudRate() const { return m_baudRate; }

private:
    struct ReaderSlot {
        std::string name;
        osdp_pd_info_t info;
        int ledDuration;
        enum osdp_led_color_e pendingLedColor;
        int pendingBuzzerDurationMs;
        BadgeReadCallback onBadgeRead;
        ErrorCallback onError;
    };

    OsdpBus(const std::string &busName,
            const std::string &busLocation,
            int baudRate);

    static bool decodeCardReadToBadge(const osdp_event_cardread &cardRead,
                                      uint64_t &badge);
    static enum osdp_led_color_e mapLedColor(ledColor color);

    static std::shared_ptr<OsdpBus> findExistingLocked(const std::string &location);
    static bool resolveBusConfig(const std::string &identifier,
                                 std::string &busName,
                                 std::string &busLocation,
                                 int &baudRate);

    void startWorkerLocked();
    void stopWorker();
    void runWorker();

    int send(uint8_t *buf, int len);
    int recv(uint8_t *buf, int len);
    int event(int pd, struct osdp_event *event);

    void reportErrorForReader(size_t index, const std::string &message);
    void reportErrorForAll(const std::string &message);

    static std::mutex s_registryMutex;
    static std::vector<std::shared_ptr<OsdpBus>> s_busses;

    std::string m_busName;
    std::string m_busLocation;
    int m_baudRate;

    std::unique_ptr<funcmod::Uart> m_uart;
    osdp_channel m_channel;

    std::mutex m_mutex;
    std::vector<ReaderSlot> m_readers;
    bool m_setupDirty;

    std::atomic<bool> m_running;
    std::thread m_worker;
};

#endif
