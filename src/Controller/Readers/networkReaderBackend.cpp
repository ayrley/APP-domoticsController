#include <string>

#include "debug.h"
#include "networkReaderBackend.h"
#include "remoteBadgeChannel.h"

namespace {
std::string toLedString(ledColor color, bool granted)
{
    switch (color) {
    case LED_RED:
        return "red";
    case LED_GREEN:
        return "green";
    case LED_YELLOW:
        return "yellow";
    case LED_NONE:
    default:
        return granted ? "green" : "red";
    }
}

json buildFeedbackOutput(const ReaderDecision &decision)
{
    json feedback = {
        {"output_type", "reader_feedback"},
        {"led", toLedString(decision.led, decision.granted)},
        {"buzzer", decision.buzzer},
        {"duration", decision.buzzerDurationMs}};

    return feedback;
}
} // namespace

NetworkReaderBackend::NetworkReaderBackend(const std::string &readerName,
                                           const std::string &readerLocation)
{
    this->m_readerName = readerName;
    this->m_readerLocation = readerLocation;
}

void NetworkReaderBackend::run(const BadgeReadCallback &onBadgeRead,
                               const ErrorCallback &onError)
{
    RemoteBadgeChannel channel(this->m_readerLocation);
    LOG("Network reader '" << this->m_readerName << "' is starting on " << this->m_readerLocation);

    bool started = channel.serve([onBadgeRead](uint64_t badge) {
        RemoteBadgeReply reply;
        ReaderDecision decision = onBadgeRead(badge);

        reply.valid = decision.granted;
        reply.badge = badge;
        reply.action = decision.actionOutputs;
        reply.action.push_back(buildFeedbackOutput(decision));
        reply.validPayload = true;

        return reply;
    });

    if (!started && onError) {
        onError("Network reader '" + this->m_readerName +
                "' failed to start on " + this->m_readerLocation);
    }
}
