#ifndef __NETWORK_READER_BACKEND_H_
#define __NETWORK_READER_BACKEND_H_

#include <string>

#include "readerBackend.h"

class NetworkReaderBackend : public ReaderBackend
{
private:
    std::string m_readerName;
    std::string m_readerLocation;

public:
    NetworkReaderBackend(const std::string &readerName,
                         const std::string &readerLocation);

    void run(const BadgeReadCallback &onBadgeRead,
             const ErrorCallback &onError) override;
};

#endif
