#include <unity.h>
//#include <Arduino.h>
#include "createGrafanaMessage.h"

void testSendDataGrafana() {
     //= sendDataGrafana(float temperature, float humidity, float co2);
     TEST_ASSERT_TRUE(true);

}

void testCreateGrafanaMessage() {
    char buffer[256];
    createGrafanaMessage(buffer, sizeof(buffer), "ASC02", 23.45, 55.67, 789.0, 1234567890000000000ULL);

    const char* expected = "medicionesCO2,device=ASC02 temp=23.45,hum=55.67,co2=789.00 1234567890000000000";
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}
