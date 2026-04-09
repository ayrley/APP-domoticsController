#include <cerrno>
#include <string>

#include <stdint.h>

#include "action.h"
#include "badge.h"
#include "debug.h"
#include "eventLog.h"
#include "accessController.h"

#include "Readers/networkReaderBackend.h"
#include "Readers/osdpReaderBackend.h"
#include "Readers/wiegandReaderBackend.h"

AccessController::AccessController()
{
    this->m_badges = nullptr;
    this->m_grantedAction = nullptr;
    this->m_deniedAction = nullptr;
    this->m_ledDuration = 3000;
}

AccessController::AccessController(std::vector<Badge *> *badges)
{
    this->m_badges = badges;
    this->m_grantedAction = nullptr;
    this->m_deniedAction = nullptr;
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
    this->init_reader();
    this->m_runner = std::thread(&AccessController::handle, this);
    this->m_runner.detach();
}

int AccessController::init_reader()
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

    if (this->m_readerType == RDR_WIEGAND) {
        this->m_backend = std::make_unique<WiegandReaderBackend>(
            this->m_readerLocation,
            this->m_ledDuration);
        return 0;
    }

    if (this->m_readerType == RDR_OSDP) {
        this->m_backend = std::make_unique<OsdpReaderBackend>();
        return 0;
    }

    this->reportError("Unknown reader type");
    return -EINVAL;
}

int AccessController::init_reader(readerType type)
{
    this->m_readerType = type;
    return this->init_reader();
}

ReaderDecision AccessController::onBadgeRead(uint64_t badge)
{
    ReaderDecision decision;
    Action *actionToExecute = this->m_deniedAction;

    if (this->m_badges != nullptr) {
        for (auto singleBadge : *this->m_badges) {
            if (singleBadge && singleBadge->valid(badge)) {
                decision.granted = true;
                decision.firstName = singleBadge->getFirstName();
                decision.lastName = singleBadge->getLastName();
                actionToExecute = this->m_grantedAction;
                break;
            }
        }
    } else {
        DBG("No badges loaded; access denied");
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
        if (this->m_backend == nullptr && this->init_reader() != 0)
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
    object += json::object_t::value_type("type", this->m_readerType == RDR_OSDP ? "OSDP" : "WIEGAND");
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
    this->m_readerType = RDR_WIEGAND;

    tmpHelp = jsonObject["location_type"];
    if (tmpHelp == "IP")
        this->m_readerLocationType = RDR_LOC_IP;

    tmpHelp = jsonObject["type"];
    if (tmpHelp == "OSDP")
        this->m_readerType = RDR_OSDP;

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
