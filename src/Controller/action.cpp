#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <croncpp.h>

#include <remoteGpioChannel.h>

#include "action.h"
#include "actionIo.h"
#include "debug.h"
#include "eventLog.h"
#include "io.h"

extern std::vector<IO *> *g_ios;

Action::Action()
{
    this->m_inputType = IN_GPIO;
    this->m_cronValid = false;
    this->m_cronExpressionText = "";
    this->m_outputDefinitions = json::array();
}

bool Action::setRemoteGpio(IO &io, int value, int durationMs)
{
    RemoteGpioChannel channel(io.getLocation());
    RemoteGpioCommand command;
    command.name = io.getName();
    command.value = value;
    command.duration = durationMs;

    RemoteGpioReply reply;
    if (!channel.send(command, reply)) {
        return false;
    }

    if (!reply.ok) {
        ERR("Remote GPIO endpoint rejected command for '" << io.getName() << "': " << reply.error);
        return false;
    }

    return true;
}

json Action::getJson()
{
    json object = {};
    object += json::object_t::value_type("name", this->m_name);

    if (this->m_inputType == IN_TIME) {
        object += json::object_t::value_type("input_type", "TIME");
        if (!this->m_cronExpressionText.empty()) {
            object += json::object_t::value_type("cron", this->m_cronExpressionText);
        }
    } else if (this->m_input.getName() != "") {
        object += json::object_t::value_type("input", this->m_input.getName());
        if (this->m_inputType == IN_IP) {
            object += json::object_t::value_type("input_type", "IP");
        } else if (this->m_inputType == IN_GPIO) {
            object += json::object_t::value_type("input_type", "GPIO");
        }
    }

    json outputsJson = json::array();
    if (!this->m_outputDefinitions.empty()) {
        outputsJson = this->m_outputDefinitions;
    } else {
        for (auto &singleOutput : this->m_outputs) {
            json outputJson = {};
            IO *outputIo = singleOutput.getIo();
            if (outputIo == nullptr) {
                continue;
            }

            outputJson += json::object_t::value_type("output", outputIo->getName());
            outputJson += json::object_t::value_type(
                "output_type",
                outputIo->getLocationType() == IO_LOC_IP ? "ip" : "gpio");
            if (singleOutput.getDuration() > 0) {
                outputJson += json::object_t::value_type("duration", singleOutput.getDuration());
            }
            if (singleOutput.isInverted()) {
                outputJson += json::object_t::value_type("inverted", true);
            }
            outputsJson.push_back(outputJson);
        }
    }
    object += json::object_t::value_type("outputs", outputsJson);

    return object;
}

void Action::fromJson(const json &jsonObject)
{
    this->m_inputType = IN_GPIO;
    this->m_cronValid = false;
    this->m_cronExpressionText = "";
    this->m_outputs.clear();
    this->m_outputDefinitions = json::array();

    try {
        this->m_name = jsonObject["name"];
    } catch (const std::exception &e) {
        this->m_name = "";
        DBG("name could not be found in " + jsonObject.dump());
    }

    if (jsonObject.contains("input_type")) {
        if (jsonObject["input_type"] == "GPIO")
            this->m_inputType = IN_GPIO;
        else if (jsonObject.contains("input_type") && jsonObject["input_type"] == "IP")
            this->m_inputType = IN_IP;
        else if (jsonObject.contains("input_type") && jsonObject["input_type"] == "TIME")
            this->m_inputType = IN_TIME;
    }

    if (jsonObject.contains("input") && this->m_inputType != IN_TIME) {
        if (jsonObject.contains("input") && !jsonObject["input"].is_null()) {
            for (IO *io : *g_ios) {
                if (io->getName() == jsonObject["input"])
                    this->m_input = *io;
            }
        }
    }

    if (this->m_inputType == IN_TIME) {
        if (jsonObject.contains("cron") && !jsonObject["cron"].is_null()) {
            try {
                std::string cronExpr = jsonObject["cron"];
                this->m_cronExpressionText = cronExpr;
                this->m_cronExpression = cron::make_cron(cronExpr);
                this->m_cronValid = true;
            } catch (const std::exception &e) {
                ERR("Invalid cron expression in action '" << this->m_name << "': " << e.what());
            }
        } else {
            ERR("No cron expression found for time-based action '" << this->m_name << "'");
        }
    }

    try {
        if (jsonObject.contains("outputs")) {
            for (auto &singleOutput : jsonObject["outputs"].items()) {
                this->m_outputDefinitions.push_back(singleOutput.value());

                for (IO *io : *g_ios) {
                    if (io->getName() == singleOutput.value()["output"]) {
                        ActionIo actionIo(io);

                        if (singleOutput.value().contains("duration")) {
                            actionIo.setDuration(singleOutput.value()["duration"]);
                        }

                        if (singleOutput.value().contains("inverted")) {
                            actionIo.setInverted(singleOutput.value()["inverted"]);
                        }

                        this->m_outputs.push_back(actionIo);
                    }
                }
            }
        }
    } catch (const std::exception &e) {
        DBG("outputs could not be found in " + jsonObject.dump());
    }
}

void Action::executeSingle(ActionIo io)
{
    IO *ioPtr = io.getIo();
    if (!ioPtr) {
        return;
    }

    std::string outputName = ioPtr->getName();
    if (outputName.empty()) {
        outputName = ioPtr->getLocation();
    }

    if (ioPtr->getLocationType() == IO_LOC_IP) {
        if (this->setRemoteGpio(*ioPtr, 1, io.getDuration())) {
            EventLog::addOutputToggle(outputName, true, "action:" + this->m_name);
            if (io.getDuration() > 0) {
                EventLog::addOutputToggle(outputName, false, "action:" + this->m_name);
            }
        }
        return;
    }

    io.set();
    EventLog::addOutputToggle(outputName, true, "action:" + this->m_name);

    if (io.getDuration() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(io.getDuration()));
        io.clear();
        EventLog::addOutputToggle(outputName, false, "action:" + this->m_name);
    }
}

void Action::execute()
{
    std::thread actionRunner;

    for (auto singleIo : this->m_outputs) {
        actionRunner = std::thread(&Action::executeSingle, this, singleIo);
        actionRunner.detach();
    }
}

void Action::run()
{
    while (1) {
        if (this->m_inputType == IN_TIME) {
            if (!this->m_cronValid) {
                ERR("Skipping invalid time-based action '" << this->m_name << "'");
                return;
            }

            auto now = std::chrono::system_clock::now();
            auto next = cron::cron_next(this->m_cronExpression, now);
            std::this_thread::sleep_until(next);
        } else {
            while (!this->m_input.get()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        }

        this->execute();
    }
}

void Action::start()
{
    if (this->m_inputType == IN_TIME && !this->m_cronValid) {
        ERR("Not starting action with invalid cron: '" << this->m_name << "'");
        return;
    }

    this->m_runner = std::thread(&Action::run, this);
    this->m_runner.detach();
}
