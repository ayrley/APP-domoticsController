#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <thread>
#include <filesystem>

#include <stdint.h>

#include "control.h"
#include "settings.h"
#include "badge.h"

std::vector<Settings *> *g_settings = new std::vector<Settings *>();
std::vector<Badge *> *g_badges = new std::vector<Badge *>();

int loadSettings()
{
	int ret = 0;

	std::stringstream userSettingsStream;
	std::stringstream factorySettingsStream;

	std::string userSettingsFile;
	std::string factorySettingsFile;

	userSettingsStream << DIR_SHARED << "user.settings.domotics";
	userSettingsStream >> userSettingsFile;

	factorySettingsStream << DIR_ETC << "factory.settings.domotics";
	factorySettingsStream >> factorySettingsFile;

	Settings *factory = new Settings(factorySettingsFile);
	Settings *user;

	bool settingsExist = std::filesystem::exists(userSettingsFile);

	g_settings->push_back(factory);

	if (!settingsExist)
		std::filesystem::copy(factorySettingsFile, userSettingsFile);

	user = new Settings(userSettingsFile);

	if (user->getType() != SET_USER) {
		user->setType(SET_USER);
		user->write();
	}

	g_settings->push_back(user);

	factory->parse();
	user->parse();

	return ret;
}

int loadBadges()
{
	int ret = 0;

	std::stringstream badgesStream;

	std::string badgesDir;

	badgesStream << DIR_SHARED << "badges";
	badgesStream >> badgesDir;

    for (const auto & entry : std::filesystem::directory_iterator(badgesDir)) {
		std::string badgePath = entry.path();
		Badge *badge = new Badge(badgePath);
		g_badges->push_back(badge);
	}

   	return ret;
}

int main(void)
{
	int ret = 0;

	loadSettings();
	loadBadges();

	return ret;

}
