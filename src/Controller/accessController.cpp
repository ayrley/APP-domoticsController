#include <cerrno>
#include <string>

#include <stdint.h>

#include "action.h"
#include "badge.h"
#include "debug.h"
#include "eventLog.h"
#include "accessController.h"
#include "accessZone.h"

#include "Readers/networkReaderBackend.h"
#include "Readers/osdpReaderBackend.h"
#include "Readers/wiegandReaderBackend.h"

AccessController::AccessController()
{
    this->m_badges = nullptr;
    this->m_grantedAction = nullptr;
    this->m_deniedAction = nullptr;
    this->m_zone = nullptr;
    this->m_ledDuration = 3000;
}

AccessController::AccessController(std::vector<Badge *> *badges)
{
    this->m_badges = badges;
    this->m_grantedAction = nullptr;
    this->m_deniedAction = nullptr;
    this->m_zone = nullptr;
    this->m_ledDuration = 3000;
}

AccessController::~AccessController()
{
}

void AccessController::setBadges(std::vector<Badge *> *badges)
{
    this->m_badges = badges;
}

void AccessController::setErrorHandler(const std::function<void(readerLocationType, const std::string &)> &handler)
{
    this->m_errorHandler = handler;
}

void AccessController::reportError(const std::string &message)
{
    if (this->m_errorHandler) {
        this->m_errorHandler(this->m_readerLocationType, message);
    }
}

void AccessController::start()
{
    this->initReader();
    this->m_runner = std::thread(&AccessController::handle, this);
    this->m_runner.detach();
}

int AccessController::initReader()
{
    if (this->m_readerLocationType == RDR_LOC_IP) {
        this->m_backend = std::make_unique<NetworkReaderBackend>(
            this->m_readerName,
            this->m_readerLocation);
        return 0;
    }

    if (this->m_readerLocationType != RDR_LOC_LOCAL) {
        this->reportError("Unknown reader location type");
        return -EINVAL;
    }

    if (this->m_readerProtocol == RDR_WIEGAND) {
        this->m_backend = std::make_unique<WiegandReaderBackend>(
            this->m_readerLocation,
            this->m_ledDuration);
        return 0;
    }

    if (this->m_readerProtocol == RDR_OSDP) {
        this->m_backend = std::make_unique<OsdpReaderBackend>();
        return 0;
    }

    this->reportError("Unknown reader type");
    return -EINVAL;
}

int AccessController::initReader(readerProtocol protocol)
{
    this->m_readerProtocol = protocol;
    return this->initReader();
}

ReaderDecision AccessController::onBadgeRead(uint64_t badge)
{
    ReaderDecision decision;
    Action *actionToExecute = this->m_deniedAction;
    const std::string zoneName = this->m_zone != nullptr ? this->m_zone->getName() : "";

    if (this->m_badges != nullptr) {
        for (auto singleBadge : *this->m_badges) {
            if (singleBadge == nullptr) {
                continue;
            }

            if (singleBadge->getBadgeNumber() != badge) {
                continue;
            }

            std::string failureReason;
            if (singleBadge->valid(badge, zoneName, &failureReason)) {
                decision.granted = true;
                decision.firstName = singleBadge->getFirstName();
                decision.lastName = singleBadge->getLastName();
                actionToExecute = this->m_grantedAction;
                break;
            }

            DBG("Badge access denied for zone '" + zoneName + "': " + failureReason);
        }
    } else {
        DBG("No badges loaded; access denied");
    }

    if (decision.granted && this->m_zone != nullptr && this->m_zone->isAntipassbackEnabled()) {
        if (this->m_readerIOputType == RDR_IN || this->m_readerIOputType == RDR_IN_OUT) {
            this->m_zone->recordEntry(badge, this->m_readerIOputType);
        } else if (this->m_readerIOputType == RDR_OUT) {
            if (!this->m_zone->validateAndRecordExit(badge, this->m_readerIOputType)) {
                decision.antipassbackViolation = true;
                decision.granted = false;
                actionToExecute = this->m_deniedAction;
                DBG("Antipassback violation: badge " + std::to_string(badge) + " trying to exit without entry");
            }
        }
    }

    if (!decision.granted)
        actionToExecute = this->m_deniedAction;

    if (actionToExecute) {
        actionToExecute->execute();

        json actionJson = actionToExecute->getJson();
        if (actionJson.contains("outputs")) {
            decision.actionOutputs = actionJson["outputs"];
        }
    }

    EventLog::addBadgeRead(this->m_readerName, badge, decision.granted,
                           decision.firstName, decision.lastName);


    return decision;
}

void AccessController::handle()
{
    try {
        if (this->m_backend == nullptr && this->initReader() != 0)
            return;

        this->m_backend->run(
            [this](uint64_t badge) { return this->onBadgeRead(badge); },
            [this](const std::string &message) { this->reportError(message); });
    } catch (const std::exception &e) {
        reportError(e.what());
    }
}

json AccessController::getJson()
{
    json object = {};
    object += json::object_t::value_type("name", this->m_readerName);
    object += json::object_t::value_type("location", this->m_readerLocation);
    object += json::object_t::value_type("location_type", this->m_readerLocationType == RDR_LOC_IP ? "IP" : "LOCAL");
    object += json::object_t::value_type("protocol", this->m_readerProtocol == RDR_OSDP ? "OSDP" : "WIEGAND");
    if (this->m_readerIOputType == RDR_IN) {
        object += json::object_t::value_type("type", "IN");
    } else if (this->m_readerIOputType == RDR_OUT) {
        object += json::object_t::value_type("type", "OUT");
    } else {
        object += json::object_t::value_type("type", "IN_OUT");
    }
    if (this->m_grantedAction) {
        object += json::object_t::value_type("granted", this->m_grantedAction->getJson());
    }
    if (this->m_deniedAction) {
        object += json::object_t::value_type("denied", this->m_deniedAction->getJson());
    }

    return object;
}

void AccessController::fromJson(const json &jsonObject)
{
    std::string tmpHelp;

    this->m_readerName = jsonObject["name"];
    this->m_readerLocation = jsonObject["location"];
    this->m_readerLocationType = RDR_LOC_LOCAL;
    this->m_readerProtocol = RDR_WIEGAND;

    tmpHelp = jsonObject["location_type"];
    if (tmpHelp == "IP")
        this->m_readerLocationType = RDR_LOC_IP;

    tmpHelp = jsonObject["protocol"];
    if (tmpHelp == "OSDP")
        this->m_readerProtocol = RDR_OSDP;

    if (jsonObject.contains("type")) {
        tmpHelp = jsonObject["type"];
        if (tmpHelp == "IN")
            m_readerIOputType = RDR_IN;
        else if (tmpHelp == "OUT")
            m_readerIOputType = RDR_OUT;
        else
            m_readerIOputType = RDR_IN_OUT;
    }

    if (jsonObject.contains("granted")) {
        json grantedActionJson = jsonObject["granted"];
        Action *grantedAction = new Action();
        grantedAction->fromJson(grantedActionJson);
        this->m_grantedAction = grantedAction;
    }

    if (jsonObject.contains("denied")) {
        json deniedActionJson = jsonObject["denied"];
        Action *deniedAction = new Action();
        deniedAction->fromJson(deniedActionJson);
        this->m_deniedAction = deniedAction;
    }
}
