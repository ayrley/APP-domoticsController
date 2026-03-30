#include <string>
#include <fstream>
#include <cerrno>
#include <iostream>
#include <vector>

#include <stdint.h>

#include "settings.h"
#include "network.h"

#define SETTINGS_VERSION 1

Settings::Settings()
{
	this->m_readers = new std::vector<Reader *>();
	this->m_actions = new std::vector<Action *>();
	this->m_settingsFile = "/etc/factory.settigetReadersngs.domotics";
	this->m_network = new Network();
	this->read();
}

Settings::Settings(std::string settingsFile)
{
	this->m_readers = new std::vector<Reader *>();
	this->m_actions = new std::vector<Action *>();
	this->m_settingsFile = settingsFile;
	this->m_network = new Network();
	this->read();
}

Settings::~Settings()
{
	this->settingsType = SET_NONE;
	this->m_settingsFile = "";

	delete this->m_network;
}

void Settings::parse()
{
	m_network->fromJson(this->m_settings["network"]);
}

int Settings::setSettingsFile(std::string settingsFile)
{
	this->m_settingsFile = settingsFile;

	return 0;
}

int Settings::parseSettingsType()
{
	if (this->m_settings["type"] == "FACTORY")
		this->settingsType = SET_FACTORY;
	else if (this->m_settings["type"] == "USER")
		this->settingsType = SET_USER;
	else
		this->settingsType = SET_NONE;

	return 0;
}

int Settings::parseReaders()
{
 	for (auto &sinlgeReader : this->m_settings["readers"].items())
	{
		Reader *rdr = new Reader();
		rdr->fromJson(sinlgeReader.value());
		this->m_readers->push_back(rdr);
    }    
    
    return 0;
}

int Settings::parseActions()
{
 	for (auto &singleAction : this->m_settings["actions"].items())
	{
		Action *action = new Action();
		action->fromJson(singleAction.value());
		this->m_actions->push_back(action);
	}

	return 0;
}

int Settings::read()
{
	std::ifstream jsonFile(this->m_settingsFile);

	this->m_settings = json::parse(jsonFile);
	if (this->m_settings.is_discarded())
		return -ENOENT;

	this->parseSettingsType();
	this->parseReaders();
	this->parseActions();

	return 0;
}

int Settings::read(std::string settingsFile)
{
	this->setSettingsFile(settingsFile);

	return this->read();
}

int Settings::write()
{
	std::ofstream settingsFile(this->m_settingsFile);
	this->m_settings.clear();
	this->setType(this->settingsType);
	this->m_settings["version"] = SETTINGS_VERSION;

	this->m_settings.push_back(json::object_t::value_type("network", 
		this->m_network->getJson()));

	settingsFile << std::setw(4) << this->m_settings << std::endl;

	return 0;
}

int Settings::write(std::string settingsFile)
{
	this->setSettingsFile(settingsFile);
	return 0;
}

void Settings::setType(enum settingsType type)
{
	this->settingsType = type;

	switch (type) {
		case SET_FACTORY:
			this->m_settings["type"] = "FACTORY";
			break;
		case SET_USER:
			this->m_settings["type"] = "USER";
			break;
		case SET_NONE:
		default :
			this->m_settings["type"] = "NONE";
			break;
	}
}

json Settings::getFromSettings(const std::string &key)
{
	return this->m_settings[key];
}

int Settings::version()
{
	if (!this->m_settings.contains("version"))
		return 0;

	return this->m_settings["version"];
}

enum settingsType Settings::getType()
{
	return this->settingsType;
}

Network *Settings::getNetwork()
{
	return this->m_network;
}

std::vector<Reader *> *Settings::getReaders()
{
	return this->m_readers;
}

std::vector<Action *> *Settings::getActions()
{
	return this->m_actions;
}