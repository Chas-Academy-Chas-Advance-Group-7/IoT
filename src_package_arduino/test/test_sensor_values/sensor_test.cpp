#include "DHT_handler_test_helpers.h"
#include <unity.h>

void setUp() {}
void tearDown() {}

// Test 1: standard reading
void test_standardReading()
{
    setFakeDHTValues(25.2f, 7.3f);
    SensorPacket packet = readDHT_Fake(42, 12345, 89084);

    TEST_ASSERT_EQUAL_UINT8(42, packet.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(12345, packet.sensor_timestamp);
    TEST_ASSERT_EQUAL_UINT32(89084, packet.server_package_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.2f, packet.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 7.3f, packet.humidity);
}

// Test 2: max values
void test_maxValues()
{
    setFakeDHTValues(100.0f, 100.0f);
    SensorPacket packet = readDHT_Fake(255, 99999, 99999);

    TEST_ASSERT_EQUAL_UINT8(255, packet.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(99999, packet.sensor_timestamp);
    TEST_ASSERT_EQUAL_UINT32(99999, packet.server_package_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, packet.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, packet.humidity);
}

// Test 3: min values
void test_minValues()
{
    setFakeDHTValues(0.0f, 0.0f);
    SensorPacket packet = readDHT_Fake(0, 0, 0);

    TEST_ASSERT_EQUAL_UINT8(0, packet.sensor_id);
    TEST_ASSERT_EQUAL_UINT32(0, packet.sensor_timestamp);
    TEST_ASSERT_EQUAL_UINT32(0, packet.server_package_id);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, packet.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, packet.humidity);
}

// Test 4: changing values mid-test
void test_dynamicValues()
{
    setFakeDHTValues(22.0f, 50.0f);
    SensorPacket packet1 = readDHT_Fake(1, 1000, 2000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 22.0f, packet1.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, packet1.humidity);

    setFakeDHTValues(30.0f, 60.0f);
    SensorPacket packet2 = readDHT_Fake(2, 2000, 4000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 30.0f, packet2.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 60.0f, packet2.humidity);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_standardReading);
    RUN_TEST(test_maxValues);
    RUN_TEST(test_minValues);
    RUN_TEST(test_dynamicValues);

    return UNITY_END();
}
