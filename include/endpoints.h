#ifndef ENDPOINTS_H
#define ENDPOINTS_H

void handleMediciones();
void handleConfiguracion();
void habldePostConfig();
void handleData();
void handleSCD30Calibration();
void handleSettings();
void handleRestart();
void handleConfigReset();

void handleStatus();
void handleHome();
void handleStyle();
void handleConfigJs();
void handleFavicon();
void handleApiAdminInfo();
void handleApiAdminPassword();

// Mediator / actuator endpoints
void handleActuatorCommand();   // POST /actuator/command
void handleActuatorStatus();    // GET  /actuator/status
void handleRulesReload();       // POST /rules/reload
void handleRulesGet();          // GET  /rules
void handleRulesSave();         // POST /rules/save
void handleRulesEditor();       // GET  /rules-editor

#ifdef ENABLE_ESPNOW
void handleESPNowStatus();
#endif

#endif // ENDPOINTS_H