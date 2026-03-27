#ifndef __ACTION_H
#define __ACTION_H

#include <string>
#include <cstdint>
#include <vector>

#include "io.h"
#include "actionIo.h"

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

    void executeSingle(ActionIo io);
    void run();

public:
    Action();

    void fromJson(const json &jsonObject);
    void execute();;

    void start();
};

#endif 