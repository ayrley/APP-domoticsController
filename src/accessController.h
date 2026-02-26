#ifndef __ACCESSCONTROLLER_H_
#define __ACCESSCONTROLLER_H_

#include <string>
#include <mutex>

#include <stdint.h>

#include "json.hpp"

#include "reader.h"

using json = nlohmann::json;


class AccessController
{
private:
	std::string m_name;
	Reader * m_reader;

public:
	AccessController();
	~AccessController();
	
	void start();
};

#endif
