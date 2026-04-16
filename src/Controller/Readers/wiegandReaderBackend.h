#ifndef __WIEGAND_READER_BACKEND_H_
#define __WIEGAND_READER_BACKEND_H_

#include <cstdint>
#include <string>

#include "readerBackend.h"
#include "readerTypes.h"

class WiegandReaderBackend : public ReaderBackend
{
private:
    std::string m_readerLocation;
    int m_ledDuration;

    char m_keypad[12];
    int m_keypadBytes;

    int plainTextCode(const char *binaryCardString, uint64_t *cardNumber,
                      int length);
    int getKeypad(uint64_t *cardNumber, int count);
    int getWiegandBadge(uint64_t *badge);
    void setWiegandFeedback(enum ledColor color,
                            bool buzzer,
                            int buzzerDurationMs);

public:
    explicit WiegandReaderBackend(const std::string &readerLocation,
                                  int ledDuration);

    void run(const BadgeReadCallback &onBadgeRead,
             const ErrorCallback &onError) override;
};

#endif
