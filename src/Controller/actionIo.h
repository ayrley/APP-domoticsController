#ifndef __ACTIONIO_H
#define __ACTIONIO_H

#include <cstdint>
#include <string>

#include "io.h"

class ActionIo
{
private:
    int m_duration;
    bool m_inverted = false;

    IO *m_io;

public:
    ActionIo();
    ActionIo(IO *io);

    void setDuration(int duration);
    void setInverted(bool inverted);
    void setIo(IO *io);
    void set();
    void clear();

    int getDuration() { return this->m_duration; }

    bool isInverted() { return this->m_inverted; }

    IO *getIo() { return this->m_io; }
};

#endif
