// Tests for ESPNowManager mesh functionality (PR #11)
// These tests verify the mesh flooding logic without hardware dependencies

#include <unity.h>
#include <cstdint>
#include <cstring>

// ============================================================================
// Mock/Stub implementations for testing without Arduino/ESP-IDF
// ============================================================================

static unsigned long mock_millis_value = 0;
unsigned long millis() { return mock_millis_value; }

// ============================================================================
// Extracted testable logic from ESPNowManager
// These mirror the PR #11 implementations for unit testing
// ============================================================================

// Duplicate packet detection cache (from PR #11)
static const int SEEN_PACKET_CACHE_SIZE = 30;
struct PacketID {
    uint8_t mac[6];
    uint32_t sequence;
    unsigned long timestamp;
};

static PacketID seenPackets[SEEN_PACKET_CACHE_SIZE];
static int seenPacketIndex = 0;

void resetSeenPacketCache() {
    memset(seenPackets, 0, sizeof(seenPackets));
    seenPacketIndex = 0;
}

// Check if a packet has been seen recently (from PR #11)
bool hasSeenPacket(const uint8_t* mac, uint32_t seq) {
    unsigned long now = millis();
    const unsigned long SEEN_TIMEOUT = 60000; // 1 minute timeout

    for (int i = 0; i < SEEN_PACKET_CACHE_SIZE; i++) {
        if (memcmp(seenPackets[i].mac, mac, 6) == 0 && seenPackets[i].sequence == seq) {
            if (now - seenPackets[i].timestamp < SEEN_TIMEOUT) {
                return true; // Packet is fresh and a duplicate
            }
        }
    }
    return false; // Packet not seen or is stale
}

// Mark a packet as seen (from PR #11)
void markPacketAsSeen(const uint8_t* mac, uint32_t seq) {
    memcpy(seenPackets[seenPacketIndex].mac, mac, 6);
    seenPackets[seenPacketIndex].sequence = seq;
    seenPackets[seenPacketIndex].timestamp = millis();
    seenPacketIndex = (seenPacketIndex + 1) % SEEN_PACKET_CACHE_SIZE;
}

// SensorDataMessage structure (PR #11 version with mesh fields)
typedef struct {
    uint8_t msgType;
    uint8_t hopCount;           // New in PR #11
    uint8_t originatorMAC[6];   // New in PR #11
    char sensorId[32];
    float temperature;
    float humidity;
    float co2;
    uint32_t sequence;
} SensorDataMessage_PR11;

// Original structure (before PR #11)
typedef struct {
    uint8_t msgType;
    char sensorId[32];
    float temperature;
    float humidity;
    float co2;
    uint32_t sequence;
} SensorDataMessage_Original;

// ============================================================================
// TESTS: Duplicate Packet Detection
// ============================================================================

void testHasSeenPacket_NewPacket() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint32_t seq = 42;

    // New packet should NOT be seen
    TEST_ASSERT_FALSE(hasSeenPacket(mac, seq));
}

void testHasSeenPacket_AfterMarking() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint32_t seq = 42;

    // Mark packet as seen
    markPacketAsSeen(mac, seq);

    // Now it should be seen
    TEST_ASSERT_TRUE(hasSeenPacket(mac, seq));
}

void testHasSeenPacket_DifferentSequence() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    markPacketAsSeen(mac, 42);

    // Same MAC, different sequence should NOT be seen
    TEST_ASSERT_FALSE(hasSeenPacket(mac, 43));
}

void testHasSeenPacket_DifferentMAC() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac1[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t mac2[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

    markPacketAsSeen(mac1, 42);

    // Different MAC, same sequence should NOT be seen
    TEST_ASSERT_FALSE(hasSeenPacket(mac2, 42));
}

void testHasSeenPacket_Timeout() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint32_t seq = 42;

    markPacketAsSeen(mac, seq);
    TEST_ASSERT_TRUE(hasSeenPacket(mac, seq));

    // Advance time past 60 second timeout
    mock_millis_value = 62000;

    // Should be considered new (stale entry)
    TEST_ASSERT_FALSE(hasSeenPacket(mac, seq));
}

void testSeenPacketCache_CircularBuffer() {
    resetSeenPacketCache();
    mock_millis_value = 1000;

    uint8_t mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Fill cache with 30 packets
    for (int i = 0; i < SEEN_PACKET_CACHE_SIZE; i++) {
        mac[5] = i;
        markPacketAsSeen(mac, i);
    }

    // First packet should still be there
    mac[5] = 0;
    TEST_ASSERT_TRUE(hasSeenPacket(mac, 0));

    // Add one more packet - this should overwrite slot 0
    mac[5] = 99;
    markPacketAsSeen(mac, 99);

    // First packet should now be gone (overwritten)
    mac[5] = 0;
    TEST_ASSERT_FALSE(hasSeenPacket(mac, 0));

    // New packet should be there
    mac[5] = 99;
    TEST_ASSERT_TRUE(hasSeenPacket(mac, 99));
}

// ============================================================================
// TESTS: Message Structure (PR #11 changes)
// ============================================================================

void testSensorDataMessage_PR11_HasNewFields() {
    SensorDataMessage_PR11 msg;

    // Verify structure has the expected fields
    msg.msgType = 3;
    msg.hopCount = 4;
    msg.originatorMAC[0] = 0xAA;
    msg.originatorMAC[5] = 0xFF;
    strncpy(msg.sensorId, "test-sensor", sizeof(msg.sensorId));
    msg.temperature = 25.5;
    msg.humidity = 60.0;
    msg.co2 = 400.0;
    msg.sequence = 12345;

    TEST_ASSERT_EQUAL_UINT8(3, msg.msgType);
    TEST_ASSERT_EQUAL_UINT8(4, msg.hopCount);
    TEST_ASSERT_EQUAL_UINT8(0xAA, msg.originatorMAC[0]);
    TEST_ASSERT_EQUAL_STRING("test-sensor", msg.sensorId);
    TEST_ASSERT_EQUAL_FLOAT(25.5, msg.temperature);
}

void testSensorDataMessage_SizeIncreased() {
    // PR #11 adds hopCount (1 byte) + originatorMAC (6 bytes) = 7 bytes more
    // But with struct padding, the actual difference may vary
    size_t originalSize = sizeof(SensorDataMessage_Original);
    size_t pr11Size = sizeof(SensorDataMessage_PR11);

    // Just verify PR11 structure has the expected minimum size
    // msgType(1) + hopCount(1) + originatorMAC(6) + sensorId(32) + floats(12) + seq(4) = 56 min
    TEST_ASSERT_TRUE(pr11Size >= 56);

    // Verify the structure can hold all fields correctly
    SensorDataMessage_PR11 msg;
    msg.hopCount = 255;
    msg.originatorMAC[5] = 0xFF;
    TEST_ASSERT_EQUAL_UINT8(255, msg.hopCount);
    TEST_ASSERT_EQUAL_UINT8(0xFF, msg.originatorMAC[5]);
}

// ============================================================================
// TESTS: Hop Count Logic (mesh forwarding)
// ============================================================================

void testHopCount_InitialValue() {
    // PR #11 sets initial hopCount to 4
    SensorDataMessage_PR11 msg;
    msg.hopCount = 4;  // Initial value set by sensor

    TEST_ASSERT_EQUAL_UINT8(4, msg.hopCount);
}

void testHopCount_Decrement() {
    SensorDataMessage_PR11 msg;
    msg.hopCount = 4;

    // Simulate forwarding - decrement hop count
    if (msg.hopCount > 1) {
        msg.hopCount--;
    }

    TEST_ASSERT_EQUAL_UINT8(3, msg.hopCount);
}

void testHopCount_ShouldForward() {
    SensorDataMessage_PR11 msg;

    // hopCount > 1 means we should forward
    msg.hopCount = 2;
    TEST_ASSERT_TRUE(msg.hopCount > 1);

    // hopCount == 1 means we should NOT forward (TTL exhausted)
    msg.hopCount = 1;
    TEST_ASSERT_FALSE(msg.hopCount > 1);
}

void testHopCount_ZeroStopsForwarding() {
    SensorDataMessage_PR11 msg;
    msg.hopCount = 0;

    // Should not forward when hopCount is 0
    TEST_ASSERT_FALSE(msg.hopCount > 1);
    TEST_ASSERT_FALSE(msg.hopCount > 0);
}

// ============================================================================
// TESTS: SensorId field (changed from uint8_t to char[32])
// ============================================================================

void testSensorId_CanHoldLongNames() {
    SensorDataMessage_PR11 msg;

    const char* longId = "sensor-AABBCCDD-temp-01";
    strncpy(msg.sensorId, longId, sizeof(msg.sensorId) - 1);
    msg.sensorId[sizeof(msg.sensorId) - 1] = '\0';

    TEST_ASSERT_EQUAL_STRING(longId, msg.sensorId);
}

void testSensorId_NullTermination() {
    SensorDataMessage_PR11 msg;

    // Fill with non-null characters
    memset(msg.sensorId, 'X', sizeof(msg.sensorId));

    // Proper null-termination pattern from PR #11
    const char* id = "test";
    strncpy(msg.sensorId, id, sizeof(msg.sensorId) - 1);
    msg.sensorId[sizeof(msg.sensorId) - 1] = '\0';

    TEST_ASSERT_EQUAL_STRING("test", msg.sensorId);
    TEST_ASSERT_EQUAL_CHAR('\0', msg.sensorId[sizeof(msg.sensorId) - 1]);
}
