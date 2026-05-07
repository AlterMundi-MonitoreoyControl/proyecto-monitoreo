#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// Debug level definitions
// Set via build_flags in platformio.ini: -DDEBUG_LEVEL=2
#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL 2  // Default: Info level
#endif

// Level constants
#define DEBUG_NONE    0  // Production - no serial output
#define DEBUG_ERROR   1  // Errors only
#define DEBUG_INFO    2  // Normal operation (errors + info)
#define DEBUG_VERBOSE 3  // Full debug (errors + info + verbose)

// Serial initialization macro
#if DEBUG_LEVEL > DEBUG_NONE
  #define DEBUG_BEGIN(baud) Serial.begin(baud)
#else
  #define DEBUG_BEGIN(baud) ((void)0)
#endif

// ERROR level macros (Level 1+)
#if DEBUG_LEVEL >= DEBUG_ERROR
  #ifndef ARDUINO
  // ─── ENTORNO NATIVO (Pruebas en PC) ───
    #include <cstdio>
    #define DBG_ERROR(...)    printf(__VA_ARGS__)
  #else
  // ─── ENTORNO HARDWARE (ESP32) ───
    #include <Arduino.h>
    #define DBG_ERROR(...)    Serial.printf(__VA_ARGS__)
  #endif
  #ifndef ARDUINO
    #define DBG_ERRORLN(x)    printf("%s\n", x)
  #else
    #define DBG_ERRORLN(x)    Serial.println(x)
  #endif
#else
  #define DBG_ERROR(...)      ((void)0)
  #define DBG_ERRORLN(x)      ((void)0)
#endif

// INFO level macros (Level 2+)
#if DEBUG_LEVEL >= DEBUG_INFO
  #ifndef ARDUINO
  // ─── ENTORNO NATIVO (Pruebas en PC) ───
    #include <cstdio>
    #define DBG_INFO(...)    printf(__VA_ARGS__)
  #else
  // ─── ENTORNO HARDWARE (ESP32) ───
    #include <Arduino.h>
    #define DBG_INFO(...)    Serial.printf(__VA_ARGS__)
  #endif
  #ifndef ARDUINO
    #define DBG_INFOLN(x)     printf("%s\n", x)
  #else
    #define DBG_INFOLN(x)     Serial.println(x)
  #endif
#else
  #define DBG_INFO(...)       ((void)0)
  #define DBG_INFOLN(x)       ((void)0)
#endif


// VERBOSE level macros (Level 3 only)
#if DEBUG_LEVEL >= DEBUG_VERBOSE
  #ifndef ARDUINO
  // ─── ENTORNO NATIVO (Pruebas en PC) ───
    #include <cstdio>
    #define DBG_VERBOSE(...)    printf(__VA_ARGS__)
  #else
  // ─── ENTORNO HARDWARE (ESP32) ───
    #include <Arduino.h>
    #define DBG_VERBOSE(...)    Serial.printf(__VA_ARGS__)
  #endif
  #ifndef ARDUINO
    #define DBG_VERBOSELN(x)    printf("%s\n", x)
  #else
    #define DBG_VERBOSELN(x)    Serial.println(x)
  #endif
#else
  #define DBG_VERBOSE(...)    ((void)0)
  #define DBG_VERBOSELN(x)    ((void)0)
#endif

// Conditional block execution
#if DEBUG_LEVEL >= DEBUG_VERBOSE
  #define IF_VERBOSE(code) code
#else
  #define IF_VERBOSE(code)
#endif

#if DEBUG_LEVEL >= DEBUG_INFO
  #define IF_INFO(code) code
#else
  #define IF_INFO(code)
#endif

#endif // DEBUG_H
