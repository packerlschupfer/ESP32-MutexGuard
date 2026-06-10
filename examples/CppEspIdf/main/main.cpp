// Native ESP-IDF (non-Arduino) example exercising the ESP32-MutexGuard library.
//
// Demonstrates the RAII MutexGuard and RecursiveMutexGuard against plain
// FreeRTOS semaphore handles, using IDF-native logging (esp_log).

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "MutexGuard.h"
#include "RecursiveMutexGuard.h"

static const char *TAG = "CppEspIdf";

// Recursively lock a recursive mutex to show the same task may take it
// multiple times via nested RAII guards.
static void recursive_demo(SemaphoreHandle_t mutex, int depth) {
    RecursiveMutexGuard lock(mutex, pdMS_TO_TICKS(100));
    if (lock.hasLock()) {
        ESP_LOGI(TAG, "recursive lock acquired at depth %d", depth);
        if (depth > 0) {
            recursive_demo(mutex, depth - 1);
        }
    } else {
        ESP_LOGW(TAG, "failed to acquire recursive lock at depth %d", depth);
    }
    // lock released automatically on scope exit
}

extern "C" void app_main(void) {
    // --- Standard (non-recursive) mutex via MutexGuard ---
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    if (mutex == nullptr) {
        ESP_LOGE(TAG, "failed to create mutex");
        return;
    }

    {
        MutexGuard guard(mutex, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "MutexGuard isValid=%d hasLock=%d",
                 guard.isValid(), guard.hasLock());

        if (guard) {  // explicit operator bool
            std::string section = "critical-section";
            ESP_LOGI(TAG, "inside %s, mutex is held", section.c_str());
            // Manual early release; safe to call again on destruction.
            guard.unlock();
            ESP_LOGI(TAG, "after manual unlock hasLock=%d", guard.hasLock());
        }
    }  // guard destructor runs here (no-op since already unlocked)

    vSemaphoreDelete(mutex);

    // --- Recursive mutex via RecursiveMutexGuard ---
    SemaphoreHandle_t rmutex = xSemaphoreCreateRecursiveMutex();
    if (rmutex == nullptr) {
        ESP_LOGE(TAG, "failed to create recursive mutex");
        return;
    }

    recursive_demo(rmutex, 3);
    vSemaphoreDelete(rmutex);

    ESP_LOGI(TAG, "MutexGuard ESP-IDF example complete");
}
