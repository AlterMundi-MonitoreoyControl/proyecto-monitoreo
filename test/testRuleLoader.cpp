/**
 * test/testRuleLoader.cpp
 *
 * Valida que el RuleLoader puede parsear correctamente un archivo JSON
 * de reglas y cargarlas en el ControlMediator.
 *
 * Utiliza ArduinoFake para simular el sistema de archivos SPIFFS en un
 * entorno de pruebas nativo.
 */
#include <unity.h>
#include "core/ControlMediator.h"
#include "core/RuleLoader.h"

#ifndef ARDUINO
#include "MockESP.h" // Provee el objeto ESP global falso
#endif

void test_load_sample_rules_from_spiffs() {
    // 1. Contenido del archivo sample_rule.json
    const char* sample_rules_json = "{\"rules\":[{\"expr\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":0},\"cond\":\"GT\",\"value\":30},\"actuator\":16,\"state\":true,\"priority\":2,\"duration_ms\":0},{\"expr\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":0},\"cond\":\"LT\",\"value\":29},\"actuator\":16,\"state\":false,\"priority\":2,\"duration_ms\":0},{\"expr\":{\"op\":\"AND\",\"left\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":0},\"cond\":\"GT\",\"value\":30},\"right\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":1},\"cond\":\"GT\",\"value\":30}},\"actuator\":17,\"state\":true,\"priority\":2,\"duration_ms\":2000},{\"expr\":{\"op\":\"OR\",\"left\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":0},\"cond\":\"LT\",\"value\":30},\"right\":{\"sensor\":{\"device\":128,\"id\":8289,\"var\":1},\"cond\":\"LT\",\"value\":30}},\"actuator\":17,\"state\":false,\"priority\":2,\"duration_ms\":2000}]}";

    // 2. Simular el comportamiento de SPIFFS y File
#ifndef ARDUINO
    SPIFFS.setMockData(true, sample_rules_json);
#else
    //overrides platform rules to copy sample_rule.json to SPIFFS before test
    SPIFFS.begin(true);
    File f = SPIFFS.open("/rules.json", "w");
    f.print(sample_rules_json);
    f.close();
#endif

    // 3. Ejecutar la función a probar
    ControlMediator mediator;
    int rulesLoaded = RuleLoader::load(mediator);

    // 4. Validar el resultado
    TEST_ASSERT_EQUAL_INT(4, rulesLoaded);
    TEST_ASSERT_EQUAL_UINT8(4, mediator.getRuleCount());

    // 5. Limpiar los mocks para futuras pruebas
#ifndef ARDUINO
    SPIFFS.setMockData(false, "");
#else
    SPIFFS.remove("/rules.json");
#endif
}

void runRuleLoaderTests() {
    RUN_TEST(test_load_sample_rules_from_spiffs);
}