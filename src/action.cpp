#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <cerrno>

#include "action.h"
#include "io.h"
#include "actionIo.h"

extern std::vector<IO *> *g_ios;

Action::Action()
{
}

void Action::fromJson(const json &jsonObject)
{
	try {
		this->m_name = jsonObject["name"];
	}
	catch (const std::exception& e) {
		this->m_name = "";
		std::cout << "name could not be found in " << jsonObject
			<< std::endl;
	}

    try {
        if (jsonObject["input_type"] == "GPIO")
            this->m_inputType = IN_GPIO;
        else if (jsonObject["input_type"] == "IP")
            this->m_inputType = IN_IP;
    }
    catch (const std::exception& e) {
        std::cout << "input_type, input, or outputs could not be found in " 
            << jsonObject << std::endl;
    }
    
	try
    {
        if (jsonObject["input"]) {
            for (IO *io : *g_ios) {
                if (io->getName() == jsonObject["input"])
                    this->m_input = *io;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    try
    {
        if (jsonObject["outputs"]) {
            for (auto &singleOutput : jsonObject["outputs"].items()) {
                for (IO *io : *g_ios) {
                    if (io->getName() == singleOutput.value()) {
                        ActionIo *actionIo = new ActionIo(io);

                        if (actionIo && singleOutput.value().contains("duration")) {
                            actionIo->setDuration(singleOutput.value()["duration"]);
                        }

                        this->m_outputs.push_back(*actionIo);
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
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

    for (auto singleIo: this->m_outputs)
    {
        actionRunner = std::thread(&Action::executeSingle, this, singleIo);
        actionRunner.detach();
    }
}