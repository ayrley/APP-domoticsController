#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "statusLeds.h"
#include "settings.h"
#include "proximity.h"
#include "tamperSwitch.h"

#ifdef DEBUG
#define DIR_ETC ""
#define DIR_SHARED ""
#else
#define DIR_ETC "/etc/"
#define DIR_SHARED "/mnt/data/"
#endif

bool hasNetworkReadersConfigured();
bool hasDisplayServer();
bool shouldUseFramebufferGui();
bool enableNullPlatformIfSupported();

void clearBadgesCache();
void reloadBadgesCache();

int loadSettings();
int loadBadges();
int loadIos();
int startReaders(StatusLeds &statusLeds);
int startActions();
int runGui(Proximity &proximitySensor, StatusLeds &statusLeds, TamperSwitch &tamperSwitch);
int main(void);

#endif
