#ifndef __OSDP_READER_BACKEND_H_
#define __OSDP_READER_BACKEND_H_

#include "readerBackend.h"

class OsdpReaderBackend : public ReaderBackend
{
public:
    void run(const BadgeReadCallback &onBadgeRead,
             const ErrorCallback &onError) override;
};

#endif
