#include <cctype>
#include <cstdlib>
#include <string>

#include "json.hpp"

#include "badgeProtocol.h"

using json = nlohmann::json;

namespace
{
bool parseUnsignedBadge(const std::string &text, uint64_t &badge)
{
    if (text.empty())
        return false;

    char *endPtr = nullptr;
    unsigned long long parsedBadge = std::strtoull(text.c_str(), &endPtr, 10);
    if (endPtr == text.c_str() || *endPtr != '\0')
        return false;

    badge = static_cast<uint64_t>(parsedBadge);

    return true;
}
} // namespace

bool BadgeProtocol::parseBadgeFromPayload(const std::string &payload, uint64_t &badge) const
{
    std::string trimmed = payload;
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front()))) {
        trimmed.erase(0, 1);
    }
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) {
        trimmed.pop_back();
    }

    if (trimmed.empty())
        return false;

    if (trimmed.front() == '{') {
        json object;

        try {
            object = json::parse(trimmed);
        } catch (const std::exception &) {
            return false;
        }

        if (!object.is_object())
            return false;

        if (object.contains("badge")) {
            if (object["badge"].is_number_unsigned()) {
                badge = object["badge"].get<uint64_t>();
                return true;
            }

            if (object["badge"].is_string()) {
                return parseUnsignedBadge(object["badge"].get<std::string>(), badge);
            }
        }

        if (object.contains("badgeNumber")) {
            if (object["badgeNumber"].is_number_unsigned()) {
                badge = object["badgeNumber"].get<uint64_t>();
                return true;
            }

            if (object["badgeNumber"].is_string()) {
                return parseUnsignedBadge(object["badgeNumber"].get<std::string>(), badge);
            }
        }

        return false;
    }

    return parseUnsignedBadge(trimmed, badge);
}

std::string BadgeProtocol::buildReply(bool validBadge, uint64_t badge,
                                      const json &actionOutputs, bool validPayload) const
{
    json reply = {
        {"valid", validBadge},
        {"badge", badge},
        {"action", actionOutputs}};

    if (!validPayload)
        reply["error"] = "invalid badge payload";

    return reply.dump();
}
