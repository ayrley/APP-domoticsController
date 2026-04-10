#ifndef __BADGE_PROTOCOL_H_
#define __BADGE_PROTOCOL_H_

#include <cstdint>
#include <string>

#include "json.hpp"

using json = nlohmann::json;

class BadgeProtocol
{
private:
    static bool parseUnsignedBadge(const std::string &text, uint64_t &badge);
public:
    bool parseBadgeFromPayload(const std::string &payload, uint64_t &badge) const;
    std::string buildReply(bool validBadge, uint64_t badge, const json &actionOutputs,
                           bool validPayload) const;
};

#endif
