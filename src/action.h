#ifndef __ACTION_H
#define __ACTION_H

#include <string>
#include <cstdint>
#include <vector>

#include "io.h"

enum InputType {
    IN_GPIO = 0,
    IN_IP
};

class Action
{
private:
    IO m_input;
    std::vector<IO> m_outputs;

    std::string m_name;

    enum InputType m_inputType;
    
public:
    Action();

    void fromJson(const json &jsonObject);
    void execute();;
};

#endif 