/**
 * @file test_mutex_guard.cpp
 * @brief Unit tests for MutexGuard
 */

#ifdef UNIT_TEST

#include <Arduino.h>
#include <unity.h>
#include <MutexGuard.h>

static SemaphoreHandle_t testMutex = nullptr;

void setUp() {
    if (testMutex == nullptr) {
        testMutex = xSemaphoreCreateMutex();
    }
}

void tearDown() {
    // Ensure mutex is released
    xSemaphoreGive(testMutex);
}

void test_mutex_guard_acquires_lock() {
    {
        MutexGuard guard(testMutex);
        TEST_ASSERT_TRUE(guard.hasLock());
    }
}

void test_mutex_guard_releases_on_destruction() {
    {
        MutexGuard guard(testMutex);
        TEST_ASSERT_TRUE(guard.hasLock());
    }
    // After destruction, mutex should be available
    BaseType_t taken = xSemaphoreTake(testMutex, 0);
    TEST_ASSERT_EQUAL(pdTRUE, taken);
    xSemaphoreGive(testMutex);
}

void test_mutex_guard_timeout() {
    // Take mutex manually first
    xSemaphoreTake(testMutex, portMAX_DELAY);

    // Try to acquire with short timeout - should fail
    MutexGuard guard(testMutex, pdMS_TO_TICKS(10));
    TEST_ASSERT_FALSE(guard.hasLock());

    // Release manual lock
    xSemaphoreGive(testMutex);
}

void test_mutex_guard_null_handle() {
    MutexGuard guard(nullptr);
    TEST_ASSERT_FALSE(guard.hasLock());
    TEST_ASSERT_FALSE(guard.isValid());
}

void test_mutex_guard_is_valid() {
    MutexGuard validGuard(testMutex);
    MutexGuard invalidGuard(nullptr);

    TEST_ASSERT_TRUE(validGuard.isValid());
    TEST_ASSERT_FALSE(invalidGuard.isValid());
}

void test_mutex_guard_bool_conversion() {
    MutexGuard guard(testMutex);

    if (guard) {
        TEST_PASS();
    } else {
        TEST_FAIL_MESSAGE("Guard should convert to true when lock is held");
    }
}

void test_mutex_guard_unlock() {
    MutexGuard guard(testMutex);
    TEST_ASSERT_TRUE(guard.hasLock());

    guard.unlock();
    TEST_ASSERT_FALSE(guard.hasLock());

    // Mutex should now be available
    BaseType_t taken = xSemaphoreTake(testMutex, 0);
    TEST_ASSERT_EQUAL(pdTRUE, taken);
    xSemaphoreGive(testMutex);
}

void test_mutex_guard_double_unlock_safe() {
    MutexGuard guard(testMutex);
    TEST_ASSERT_TRUE(guard.hasLock());

    guard.unlock();
    guard.unlock();  // Should not crash or cause issues

    TEST_ASSERT_FALSE(guard.hasLock());
}

void test_mutex_guard_nested_scope() {
    {
        MutexGuard outer(testMutex);
        TEST_ASSERT_TRUE(outer.hasLock());

        // Inner scope with different mutex
        SemaphoreHandle_t innerMutex = xSemaphoreCreateMutex();
        {
            MutexGuard inner(innerMutex);
            TEST_ASSERT_TRUE(inner.hasLock());
            TEST_ASSERT_TRUE(outer.hasLock());
        }
        vSemaphoreDelete(innerMutex);

        TEST_ASSERT_TRUE(outer.hasLock());
    }
}

void test_mutex_guard_default_timeout() {
    // Take mutex manually
    xSemaphoreTake(testMutex, portMAX_DELAY);

    unsigned long start = millis();
    MutexGuard guard(testMutex);  // Default timeout is 100ms
    unsigned long elapsed = millis() - start;

    // Should have waited approximately 100ms
    TEST_ASSERT_FALSE(guard.hasLock());
    TEST_ASSERT_GREATER_OR_EQUAL(90, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(150, elapsed);

    xSemaphoreGive(testMutex);
}

// Test runner
void runMutexGuardTests() {
    UNITY_BEGIN();

    RUN_TEST(test_mutex_guard_acquires_lock);
    RUN_TEST(test_mutex_guard_releases_on_destruction);
    RUN_TEST(test_mutex_guard_timeout);
    RUN_TEST(test_mutex_guard_null_handle);
    RUN_TEST(test_mutex_guard_is_valid);
    RUN_TEST(test_mutex_guard_bool_conversion);
    RUN_TEST(test_mutex_guard_unlock);
    RUN_TEST(test_mutex_guard_double_unlock_safe);
    RUN_TEST(test_mutex_guard_nested_scope);
    RUN_TEST(test_mutex_guard_default_timeout);

    UNITY_END();
}

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("\n=== MutexGuard Unit Tests ===\n");
    runMutexGuardTests();
}

void loop() {}

#endif // UNIT_TEST
