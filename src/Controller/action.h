#ifndef __ACTION_H
#define __ACTION_H

#include <cstdint>
#include <string>
#include <vector>

#include "actionIo.h"
#include "io.h"

enum InputType {
    IN_GPIO = 0,
    IN_IP
};

class Action
{
private:
    IO m_input;

    std::vector<ActionIo> m_outputs;

    std::string m_name;

    std::thread m_runner;

    enum InputType m_inputType;

    bool setRemoteGpio(IO &io, int value, int durationMs);
    void executeSingle(ActionIo io);
    void run();

public:
    Action();

    void fromJson(const json &jsonObject);
    void execute();
    void start();

    json getJson();

    std::string getName() { return this->m_name; }
};

#endif
