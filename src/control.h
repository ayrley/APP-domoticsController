#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "statusLeds.h"
#include "settings.h"

int main(void);
int loadSettings();
int loadBadges();
int startReaders(StatusLeds &statusLeds);
int loadIos();

Settings *getSetting(enum settingsType settingType);

#ifdef DEBUG
#define DIR_ETC ""
#define DIR_SHARED ""
#else
#define DIR_ETC "/etc/"
#define DIR_SHARED "/mnt/data/"
#endif

#endif
