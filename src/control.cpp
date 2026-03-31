#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <thread>
#include <filesystem>
#include <exception>
#include <cstdlib>
#include <memory>
#include <dlfcn.h>

#include <stdint.h>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

#include "control.h"
#include "settings.h"
#include "badge.h"
#include "reader.h"
#include "io.h"
#include "screen.h"
#include "debug.h"
#include "backlight.h"
#include "proximity.h"

namespace {
constexpr int SCREENSAVER_BACKLIGHT_BRIGHTNESS = 12;
}

std::vector<Settings *> *g_settings = new std::vector<Settings *>();
std::vector<Badge *> *g_badges = new std::vector<Badge *>();
std::vector<IO *> *g_ios = new std::vector<IO *>();
std::vector<Action *> *g_actions = new std::vector<Action *>();

void clearBadgesCache()
{
	for (Badge *badge : *g_badges) {
		delete badge;
	}
	g_badges->clear();
}

void reloadBadgesCache()
{
	clearBadgesCache();
	loadBadges();
}

Settings * getSetting(enum settingsType settingType)
{
	for(Settings *setting : *g_settings) {
		if (setting->getType() == settingType)
			return setting;
	}

	std::cerr << "No settings found for type: " << settingType << std::endl;

	return nullptr;
}

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

int loadIos()
{
	int ret = 0;
	std::stringstream iosStream;
	std::string iosDir;

	iosStream << DIR_SHARED << "ios";
	iosStream >> iosDir;

    for (const auto & entry : std::filesystem::directory_iterator(iosDir)) {
		json ioFileObject;
		std::ifstream jsonFile(entry.path());
		ioFileObject = json::parse(jsonFile);

		for (auto &singleIo : ioFileObject["IOs"].items())
		{
			IO *io = new IO();
			io->fromJson(singleIo.value());
			g_ios->push_back(io);
		}
	}

   	return ret;
}

int startReaders()
{
	int ret = 0;
	Settings *userSetting = getSetting(SET_USER);

	for (auto singleReader : *userSetting->getReaders()){
		singleReader->start();
	}

   	return ret;
}

int startActions()
{
	int ret = 0;
	Settings *userSetting = getSetting(SET_USER);

	for (auto singleAction : *userSetting->getActions()){
		singleAction->start();
	}

   	return ret;
}

bool hasDisplayServer()
{
	const char *display = std::getenv("DISPLAY");
	const char *waylandDisplay = std::getenv("WAYLAND_DISPLAY");

	return (display != nullptr && display[0] != '\0')
		|| (waylandDisplay != nullptr && waylandDisplay[0] != '\0');
}

bool shouldUseFramebufferGui()
{
	return !hasDisplayServer();
}

bool enableNullPlatformIfSupported()
{
	constexpr int glfwPlatformHint = 0x00050003;
	constexpr int glfwPlatformNull = 0x00060005;
	using GlfwPlatformSupportedFn = int (*)(int);

	void *symbol = dlsym(RTLD_DEFAULT, "glfwPlatformSupported");
	if (symbol == nullptr) {
		ERR("GLFW runtime does not expose platform capability probing; attempting default desktop backend");

		return false;
	}

	auto glfwPlatformSupportedFn = reinterpret_cast<GlfwPlatformSupportedFn>(symbol);
	if (glfwPlatformSupportedFn(glfwPlatformNull) == GLFW_FALSE) {
		ERR("GLFW null platform is unavailable; attempting default desktop backend");

		return false;
	}

	glfwInitHint(glfwPlatformHint, glfwPlatformNull);

	return true;
}

int runGui(Proximity &proximitySensor)
{
	int ret = 0;
	bool useFramebufferGui = shouldUseFramebufferGui();
	std::unique_ptr<Backlight> backlight;
	int wakeBrightness = SCREENSAVER_BACKLIGHT_BRIGHTNESS;
	bool lastBacklightPresence = false;
	bool hasLastBacklightPresence = false;

	try {
		backlight = std::make_unique<Backlight>();
		wakeBrightness = backlight->getBrightness();
		LOG("Backlight control available");
	} catch (const std::exception &e) {
		ERR("Backlight control unavailable: " << e.what());
	}

	LOG("[GUI] Startup mode: "
		      << (useFramebufferGui ? "framebuffer/null platform" : "desktop window (X11/Wayland)"));

	try {
		if (useFramebufferGui) {
			LOG("No display server detected; GUI will run in framebuffer/null-platform mode.");
			enableNullPlatformIfSupported();
		}

		nanogui::init();
		Screen screen(1024, 600, DIR_SHARED "badges", []() {
			reloadBadgesCache();
			LOG("Badges cache reloaded");
		}, [&backlight, wakeBrightness]() {
			if (backlight) {
				backlight->setBrightness(wakeBrightness);
			}
		});
		proximitySensor.setDetectionHandler([&screen, &backlight, &lastBacklightPresence, &hasLastBacklightPresence, wakeBrightness](bool detected) {
			if (backlight && (!hasLastBacklightPresence || lastBacklightPresence != detected)) {
				backlight->setBrightness(detected ? wakeBrightness : SCREENSAVER_BACKLIGHT_BRIGHTNESS);
				lastBacklightPresence = detected;
				hasLastBacklightPresence = true;
			}
			nanogui::async([&screen, detected]() {
				screen.setPresenceDetected(detected);
			});
		});
		proximitySensor.start();
		screen.render();
		nanogui::run();
		proximitySensor.stop();
		nanogui::shutdown();
	} catch (const std::exception &e) {
		ERR("GUI startup failed: " << e.what());
		proximitySensor.stop();
		ret = 1;
	}

	return ret;
}

int main(void)
{

	Proximity proximitySensor;

	loadIos();
	LOG("IOs loaded");

	loadBadges();
	LOG("Badges loaded");

	loadSettings();
	LOG("Settings loaded");

	startReaders();
	LOG("Readers started");

	startActions();
	LOG("Actions started");

	return runGui(proximitySensor);

}
