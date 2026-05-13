#ifndef SECRET_MANAGER_H
#define SECRET_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "debug.h"

// Lightweight wrapper around NVS for the admin password used by Basic Auth.
// Other secrets (WiFi password, sensor tokens, etc.) keep using their existing
// storage to avoid a wider migration in this PR.
class SecretManager {
private:
    Preferences prefs;
    bool ready;

    String getSecret(const char* key) {
        if (!ready) return "";
        if (!prefs.isKey(key)) return "";
        return prefs.getString(key, "");
    }

public:
    SecretManager() : ready(false) {}

    bool begin() {
        ready = prefs.begin("monitoreo", false);
        if (!ready) {
            DBG_ERROR("[Secrets] Failed to open NVS namespace\n");
        }
        return ready;
    }

    String getAdminPassword() {
        return getSecret("admin_pass");
    }

    bool hasAdminPassword() {
        return getSecret("admin_pass").length() >= 8;
    }

    bool setAdminPassword(const String& value) {
        if (!ready) return false;
        if (value.length() == 0) {
            return prefs.remove("admin_pass");
        }
        return prefs.putString("admin_pass", value) > 0;
    }
};

#endif // SECRET_MANAGER_H
