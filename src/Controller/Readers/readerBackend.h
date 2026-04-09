#ifndef __READER_BACKEND_H_
#define __READER_BACKEND_H_

#include <cstdint>
#include <functional>
#include <string>

#include "json.hpp"

using json = nlohmann::json;

struct ReaderDecision {
    bool granted = false;
    json actionOutputs = json::array();
};

class ReaderBackend
{
public:
    using BadgeReadCallback = std::function<ReaderDecision(uint64_t)>;
    using ErrorCallback = std::function<void(const std::string &)>;

    virtual ~ReaderBackend() = default;

    virtual void run(const BadgeReadCallback &onBadgeRead,
                     const ErrorCallback &onError) = 0;
};

#endif
