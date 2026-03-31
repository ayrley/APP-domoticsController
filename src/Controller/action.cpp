#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "action.h"
#include "actionIo.h"
#include "debug.h"
#include "io.h"

extern std::vector<IO *> *g_ios;

Action::Action()
{
}

json Action::getJson()
{
    json object = {};
    object += json::object_t::value_type("name", this->m_name);

    if (this->m_input.getName() != "") {
        object += json::object_t::value_type("input", this->m_input.getName());
        if (this->m_inputType == IN_IP) {
            object += json::object_t::value_type("input_type", "IP");
        } else if (this->m_inputType == IN_GPIO) {
            object += json::object_t::value_type("input_type", "GPIO");
        }
    }

    json outputsJson = json::array();
    for (auto &singleOutput : this->m_outputs) {
        json outputJson = {};
        outputJson += json::object_t::value_type("output", singleOutput.getIo()->getName());
        if (singleOutput.getDuration() > 0) {
            outputJson += json::object_t::value_type("duration", singleOutput.getDuration());
        }
        outputsJson.push_back(outputJson);
    }
    object += json::object_t::value_type("outputs", outputsJson);

    return object;
}

void Action::fromJson(const json &jsonObject)
{
    try {
        this->m_name = jsonObject["name"];
    } catch (const std::exception &e) {
        this->m_name = "";
        DBG("name could not be found in " + jsonObject.dump());
    }

    try {
        if (jsonObject["input_type"] == "GPIO")
            this->m_inputType = IN_GPIO;
        else if (jsonObject["input_type"] == "IP")
            this->m_inputType = IN_IP;
    } catch (const std::exception &e) {
        DBG("input_type could not be found in " + jsonObject.dump());
    }

    try {
        if (jsonObject["input"]) {
            for (IO *io : *g_ios) {
                if (io->getName() == jsonObject["input"])
                    this->m_input = *io;
            }
        }
    } catch (const std::exception &e) {
        DBG("input could not be found in " + jsonObject.dump());
    }

    try {
        if (jsonObject.contains("outputs")) {
            for (auto &singleOutput : jsonObject["outputs"].items()) {
                for (IO *io : *g_ios) {
                    if (io->getName() == singleOutput.value()["output"]) {
                        ActionIo actionIo(io);

                        if (singleOutput.value().contains("duration")) {
                            actionIo.setDuration(singleOutput.value()["duration"]);
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
    ioPtr->set();

    if (io.getDuration() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(io.getDuration()));
        ioPtr->clear();
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
        if (this->m_input.get())
            this->execute();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}
void Action::start()
{
    this->m_runner = std::thread(&Action::run, this);
    this->m_runner.detach();
}
