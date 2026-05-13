#include <unity.h>

#ifndef ARDUINO
#include <ArduinoFake.h>
using namespace fakeit;
#else
#include <Arduino.h>
#endif

extern void testCreateConfigFile();
extern void testHandleMediciones();
extern void testHandleConfiguracion();
extern void testSendDataGrafana();
extern void testCreateGrafanaMessage();
extern void testCheckForUpdates();
extern void testGetLatestReleaseTag();
extern void runRuleConditionTests();
extern void runSensorIdentityTests();
extern void runAutomationFlowTests();
extern void runRuleLoaderTests();
extern void runActuatorTimingTests();
extern void testWifiPasswordLength_EmptyIsValid();
extern void testWifiPasswordLength_AcceptsWPA2Range();
extern void testWifiPasswordLength_RejectsTooShort();
extern void testWifiPasswordLength_RejectsTooLong();
extern void testAdminPassword_RejectsShort();
extern void testAdminPassword_RejectsTooLong();
extern void testAdminPassword_RejectsSameAsCurrent();
extern void testAdminPassword_RequiresCurrentWhenConfigured();
extern void testAdminPassword_AllowsFirstSetWhenUnconfigured();

void setUp() {
#ifndef ARDUINO
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000);
#endif
}
void tearDown() {}

#ifndef ARDUINO
int main() {
    UNITY_BEGIN();
    RUN_TEST(testCreateConfigFile);
    RUN_TEST(testHandleMediciones);
    RUN_TEST(testHandleConfiguracion);
    RUN_TEST(testCheckForUpdates);
    RUN_TEST(testCreateGrafanaMessage);
    RUN_TEST(testSendDataGrafana);
    RUN_TEST(testGetLatestReleaseTag);

    runRuleConditionTests();
    runSensorIdentityTests();
    runAutomationFlowTests();
    runRuleLoaderTests();
    runActuatorTimingTests();

    RUN_TEST(testWifiPasswordLength_EmptyIsValid);
    RUN_TEST(testWifiPasswordLength_AcceptsWPA2Range);
    RUN_TEST(testWifiPasswordLength_RejectsTooShort);
    RUN_TEST(testWifiPasswordLength_RejectsTooLong);
    RUN_TEST(testAdminPassword_RejectsShort);
    RUN_TEST(testAdminPassword_RejectsTooLong);
    RUN_TEST(testAdminPassword_RejectsSameAsCurrent);
    RUN_TEST(testAdminPassword_RequiresCurrentWhenConfigured);
    RUN_TEST(testAdminPassword_AllowsFirstSetWhenUnconfigured);
    return UNITY_END();
}
#else
void setup() {
    // Damos un pequeño retraso para que el monitor serie se conecte
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(testCreateConfigFile);
    RUN_TEST(testHandleMediciones);
    RUN_TEST(testHandleConfiguracion);
    RUN_TEST(testCheckForUpdates);
    RUN_TEST(testCreateGrafanaMessage);
    RUN_TEST(testSendDataGrafana);
    RUN_TEST(testGetLatestReleaseTag);

    runRuleConditionTests();
    runSensorIdentityTests();
    runAutomationFlowTests();
    runRuleLoaderTests();
    runActuatorTimingTests();
    UNITY_END();
}
void loop() {}
#endif

