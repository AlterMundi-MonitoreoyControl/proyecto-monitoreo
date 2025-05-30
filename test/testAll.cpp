#include <unity.h>

extern void testCreateConfigFile();
extern void testHandleMediciones();
extern void testHandleConfiguracion();
extern void testSendDataGrafana();
extern void testCreateGrafanaMessage();
extern void testCheckForUpdates();
extern void testGetLatestReleaseTag();

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(testCreateConfigFile);
    RUN_TEST(testHandleMediciones);
    RUN_TEST(testHandleConfiguracion);
    RUN_TEST(testCheckForUpdates);
    RUN_TEST(testGetLatestReleaseTag);       
    RUN_TEST(testCreateGrafanaMessage);
    RUN_TEST(testSendDataGrafana);
    return UNITY_END();
}
//void setup() {
//    UNITY_BEGIN();
//    RUN_TEST(testCreateConfigFile);
//    RUN_TEST(testHandleMediciones);
//    RUN_TEST(testHandleConfiguracion);
//    RUN_TEST(testCheckForUpdates);
//    RUN_TEST(testCreateGrafanaMessage);
//    RUN_TEST(testSendDataGrafana);
//    RUN_TEST(testGetLatestReleaseTag);    
//    UNITY_END();
//}

//void loop() {}
