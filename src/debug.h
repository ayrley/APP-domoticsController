#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <iostream>

#if defined(DEBUG)
#define DBG(msg) std::cout << "\tdebug: " << msg << std::endl;
#else
#define DBG(msg)
#endif

#define ERR(msg) std::cerr << "!!! " << msg << " !!!" << std::endl;
#define LOG(msg) std::clog << "--- " << msg << " ---" << std::endl;

#endif
