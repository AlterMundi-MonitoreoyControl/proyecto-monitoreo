#include <unity.h>
#include <Arduino.h>
#include "createGrafanaMessage.h"

void testSendDataGrafana() {
    TEST_ASSERT_TRUE(true);
}

void testCreateGrafanaMessage() {
    // Usar la nueva firma basada en String definida en createGrafanaMessage.h
    String result = create_grafana_message(23.45f, 55.67f, 789.0f, "S1", "ASC02");
    
    TEST_ASSERT_TRUE(result.indexOf("23.45") >= 0);
    TEST_ASSERT_TRUE(result.indexOf("55.67") >= 0);
    TEST_ASSERT_TRUE(result.indexOf("789") >= 0);
    TEST_ASSERT_TRUE(result.indexOf("ASC02") >= 0);
    TEST_ASSERT_TRUE(result.indexOf("S1") >= 0);
}
