#ifndef __ACTIONIO_H
#define __ACTIONIO_H

#include <cstdint>
#include <string>
#include <vector>

#include "io.h"

class ActionIo
{
private:
    int m_duration;
    IO *m_io;

public:
    ActionIo();
    ActionIo(IO *io);

    void setDuration(int duration);
    void setIo(IO *io);

    int getDuration() { return this->m_duration; }

    IO *getIo() { return this->m_io; }
};

#endif
