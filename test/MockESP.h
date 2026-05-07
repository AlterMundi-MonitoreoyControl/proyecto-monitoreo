#pragma once

#ifndef ARDUINO
#include <stdint.h>

// Mock global del objeto ESP para proveer dependencias de hardware (ej: MAC Address)
static struct _MockESP {
    uint64_t getEfuseMac() const { return 0x112233445566AABB; }
} ESP;

#endif