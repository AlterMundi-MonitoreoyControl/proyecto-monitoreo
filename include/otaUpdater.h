#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>

String getLatestReleaseTag(const char* repoOwner, const char* repoName);
void checkForUpdates();

#endif // OTA_UPDATER_H