#include "actionIo.h"

ActionIo::ActionIo() :
        m_duration(0),
        m_io(nullptr) 
{
}

ActionIo::ActionIo(IO *io) : 
        m_io(io),
        m_duration(0)
{
}

void ActionIo::setDuration(int duration)
{
    this->m_duration = duration;
}

void ActionIo::setIo(IO *io)
{
    this->m_io = io;
}
