#ifndef __BADGE_PROTOCOL_H_
#define __BADGE_PROTOCOL_H_

#include <cstdint>
#include <string>

class BadgeProtocol
{
public:
    bool parseBadgeFromPayload(const std::string &payload, uint64_t &badge) const;
    std::string buildReply(bool validBadge, uint64_t badge, const std::string &actionName,
                           bool validPayload) const;
};

#endif
