#ifndef MUTEXGUARD_LOGGING_H
#define MUTEXGUARD_LOGGING_H

#define MUTEXG_LOG_TAG "MutexGuard"

#include <esp_log.h>  // Required for ESP_LOG_* constants

// Define log levels based on debug flag
#ifdef MUTEXGUARD_DEBUG
    // Debug mode: Show all levels
    #define MUTEXG_LOG_LEVEL_E ESP_LOG_ERROR
    #define MUTEXG_LOG_LEVEL_W ESP_LOG_WARN
    #define MUTEXG_LOG_LEVEL_I ESP_LOG_INFO
    #define MUTEXG_LOG_LEVEL_D ESP_LOG_DEBUG
    #define MUTEXG_LOG_LEVEL_V ESP_LOG_VERBOSE
#else
    // Release mode: Only Error, Warn, Info
    #define MUTEXG_LOG_LEVEL_E ESP_LOG_ERROR
    #define MUTEXG_LOG_LEVEL_W ESP_LOG_WARN
    #define MUTEXG_LOG_LEVEL_I ESP_LOG_INFO
    #define MUTEXG_LOG_LEVEL_D ESP_LOG_NONE  // Suppress
    #define MUTEXG_LOG_LEVEL_V ESP_LOG_NONE  // Suppress
#endif

// Route to custom logger or ESP-IDF
#ifdef USE_CUSTOM_LOGGER
    #include <LogInterface.h>
    #define MUTEXG_LOG_E(...) LOG_WRITE(MUTEXG_LOG_LEVEL_E, MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_W(...) LOG_WRITE(MUTEXG_LOG_LEVEL_W, MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_I(...) LOG_WRITE(MUTEXG_LOG_LEVEL_I, MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_D(...) LOG_WRITE(MUTEXG_LOG_LEVEL_D, MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_V(...) LOG_WRITE(MUTEXG_LOG_LEVEL_V, MUTEXG_LOG_TAG, __VA_ARGS__)
#else
    // ESP-IDF logging with compile-time suppression
    #include <esp_log.h>
    #define MUTEXG_LOG_E(...) ESP_LOGE(MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_W(...) ESP_LOGW(MUTEXG_LOG_TAG, __VA_ARGS__)
    #define MUTEXG_LOG_I(...) ESP_LOGI(MUTEXG_LOG_TAG, __VA_ARGS__)
    #ifdef MUTEXGUARD_DEBUG
        #define MUTEXG_LOG_D(...) ESP_LOGD(MUTEXG_LOG_TAG, __VA_ARGS__)
        #define MUTEXG_LOG_V(...) ESP_LOGV(MUTEXG_LOG_TAG, __VA_ARGS__)
    #else
        #define MUTEXG_LOG_D(...) ((void)0)
        #define MUTEXG_LOG_V(...) ((void)0)
    #endif
#endif

// Keep backward compatibility with old debug macro
#ifdef MUTEX_GUARD_DEBUG
    #define MUTEX_GUARD_LOG(...) MUTEXG_LOG_I(__VA_ARGS__)
#else
    #define MUTEX_GUARD_LOG(...) ((void)0)
#endif

#endif // MUTEXGUARD_LOGGING_H