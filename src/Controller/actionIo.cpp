#include "actionIo.h"

ActionIo::ActionIo() :
    m_duration(0),
    m_io(nullptr),
    m_inverted(false)
{
}

ActionIo::ActionIo(IO *io) :
    m_io(io),
    m_duration(0),
    m_inverted(false)
{
}

void ActionIo::set()
{
    if (this->m_io) {
        if (this->m_inverted) {
            this->m_io->clear();
        } else {
            this->m_io->set();
        }
    }
}

void ActionIo::clear()
{
    if (this->m_io) {
        if (this->m_inverted) {
            this->m_io->set();
        } else {
            this->m_io->clear();
        }
    }
}

void ActionIo::setDuration(int duration)
{
    this->m_duration = duration;
}

void ActionIo::setInverted(bool inverted)
{
    this->m_inverted = inverted;
}

void ActionIo::setIo(IO *io)
{
    this->m_io = io;
}
