/**
 * @file test_thread_safety.cpp
 * @brief Thread safety tests for MutexGuard
 */

#ifdef UNIT_TEST

#include <Arduino.h>
#include <unity.h>
#include <MutexGuard.h>

#define TEST_THREADS 4
#define TEST_ITERATIONS 500

static SemaphoreHandle_t testMutex = nullptr;
static SemaphoreHandle_t startSemaphore = nullptr;
static volatile int sharedCounter = 0;
static volatile int threadsDone = 0;

void incrementTask(void* param) {
    int taskId = (int)(intptr_t)param;

    // Wait for start signal
    xSemaphoreTake(startSemaphore, portMAX_DELAY);

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        MutexGuard guard(testMutex, pdMS_TO_TICKS(1000));
        if (guard.hasLock()) {
            // Critical section
            int temp = sharedCounter;
            vTaskDelay(1);  // Introduce delay to increase race condition chance
            sharedCounter = temp + 1;
        }
    }

    threadsDone++;
    vTaskDelete(NULL);
}

void test_concurrent_increment() {
    sharedCounter = 0;
    threadsDone = 0;

    testMutex = xSemaphoreCreateMutex();
    startSemaphore = xSemaphoreCreateBinary();

    // Create worker tasks
    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(incrementTask, "Inc", 2048, (void*)(intptr_t)i, 1, NULL);
    }

    // Start all tasks simultaneously
    xSemaphoreGive(startSemaphore);

    // Wait for all tasks to complete
    while (threadsDone < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // With proper mutex protection, counter should equal total increments
    TEST_ASSERT_EQUAL(TEST_THREADS * TEST_ITERATIONS, sharedCounter);

    vSemaphoreDelete(testMutex);
    vSemaphoreDelete(startSemaphore);
}

static volatile bool noDeadlock = true;

void deadlockTestTask(void* param) {
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)param;

    for (int i = 0; i < 100; i++) {
        MutexGuard guard(mutex, pdMS_TO_TICKS(50));
        if (guard.hasLock()) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    threadsDone++;
    vTaskDelete(NULL);
}

void test_no_deadlock() {
    threadsDone = 0;
    noDeadlock = true;

    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

    // Create multiple tasks competing for same mutex
    for (int i = 0; i < 4; i++) {
        xTaskCreate(deadlockTestTask, "DL", 2048, mutex, 1, NULL);
    }

    // Wait with timeout - if deadlock occurs, this will timeout
    unsigned long start = millis();
    while (threadsDone < 4 && (millis() - start) < 10000) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    TEST_ASSERT_EQUAL(4, threadsDone);

    vSemaphoreDelete(mutex);
}

static volatile int resourceAccessCount = 0;
static volatile int maxConcurrentAccess = 0;
static volatile int currentAccess = 0;

void resourceAccessTask(void* param) {
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)param;

    xSemaphoreTake(startSemaphore, portMAX_DELAY);

    for (int i = 0; i < 200; i++) {
        MutexGuard guard(mutex, pdMS_TO_TICKS(500));
        if (guard.hasLock()) {
            currentAccess++;
            if (currentAccess > maxConcurrentAccess) {
                maxConcurrentAccess = currentAccess;
            }

            vTaskDelay(1);
            resourceAccessCount++;

            currentAccess--;
        }
    }

    threadsDone++;
    vTaskDelete(NULL);
}

void test_mutual_exclusion() {
    threadsDone = 0;
    resourceAccessCount = 0;
    maxConcurrentAccess = 0;
    currentAccess = 0;

    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    startSemaphore = xSemaphoreCreateBinary();

    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(resourceAccessTask, "Res", 2048, mutex, 1, NULL);
    }

    xSemaphoreGive(startSemaphore);

    while (threadsDone < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Max concurrent access should be exactly 1 (mutex provides exclusion)
    TEST_ASSERT_EQUAL(1, maxConcurrentAccess);
    TEST_ASSERT_EQUAL(TEST_THREADS * 200, resourceAccessCount);

    vSemaphoreDelete(mutex);
    vSemaphoreDelete(startSemaphore);
}

void runThreadSafetyTests() {
    UNITY_BEGIN();

    RUN_TEST(test_concurrent_increment);
    RUN_TEST(test_no_deadlock);
    RUN_TEST(test_mutual_exclusion);

    UNITY_END();
}

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("\n=== MutexGuard Thread Safety Tests ===\n");
    runThreadSafetyTests();
}

void loop() {}

#endif // UNIT_TEST
