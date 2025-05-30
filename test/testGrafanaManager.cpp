#include <unity.h>
#include "createGrafanaMessage.h"
#include <ctime>     // para time_t, time()
#include <cstdio>    // para snprintf

// Simulamos la variable global
char deviceName[64] = "ASC02";

// Mockeamos time()
extern "C" time_t time(time_t* t) {
    return 1234567890; // segundos simulados
}

void testSendDataGrafana() {
     //= sendDataGrafana(float temperature, float humidity, float co2);
     TEST_ASSERT_TRUE(true);

}

void testCreateGrafanaMessage() {
    char buffer[256];
    createGrafanaMessage(buffer, sizeof(buffer), 23.45, 55.67, 789.0);

    char expected[256];
    snprintf(expected, sizeof(expected),
             "medicionesCO2,device=%s temp=23.45,hum=55.67,co2=789.00 1234567890000000000",
             deviceName);

    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}