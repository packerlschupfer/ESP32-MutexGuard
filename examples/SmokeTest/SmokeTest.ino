/**
 * @file SmokeTest.ino
 * @brief Minimal compile-smoke example for ESP32-MutexGuard.
 *
 * Exercises the public API:
 *   - create a FreeRTOS mutex
 *   - take it via a MutexGuard RAII scope (hasLock / isValid / operator bool)
 *   - manual unlock()
 *   - same for RecursiveMutexGuard
 *
 * Uses the default (ESP-IDF) logging backend; USE_CUSTOM_LOGGER is NOT defined,
 * so the ESP32-Logger submodule is not required to compile this example.
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "MutexGuard.h"
#include "RecursiveMutexGuard.h"

static SemaphoreHandle_t s_mutex = nullptr;
static SemaphoreHandle_t s_recursiveMutex = nullptr;

void setup() {
    Serial.begin(115200);

    s_mutex = xSemaphoreCreateMutex();
    s_recursiveMutex = xSemaphoreCreateRecursiveMutex();

    // Plain mutex guard scope.
    {
        MutexGuard guard(s_mutex, pdMS_TO_TICKS(100));
        if (guard.isValid() && guard.hasLock()) {
            Serial.println("MutexGuard acquired");
        }
        if (static_cast<bool>(guard)) {
            // Critical section.
        }
        guard.unlock();  // safe to call before destruction
    }  // guard destroyed -> auto-release (no-op, already unlocked)

    // Recursive mutex guard scope.
    {
        RecursiveMutexGuard rguard(s_recursiveMutex, pdMS_TO_TICKS(100));
        if (rguard.isValid() && rguard.hasLock()) {
            Serial.println("RecursiveMutexGuard acquired");
        }
    }  // rguard destroyed -> auto-release
}

void loop() {
    // Re-enter a guarded scope each loop to keep the API referenced.
    MutexGuard guard(s_mutex, pdMS_TO_TICKS(10));
    if (guard.hasLock()) {
        // Critical section.
    }
    delay(1000);
}
