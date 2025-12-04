#ifndef RECURSIVEMUTEXGUARD_LOGGING_H
#define RECURSIVEMUTEXGUARD_LOGGING_H

#define RMUTEXG_LOG_TAG "RecursiveMutexGuard"

#include <esp_log.h>  // Required for ESP_LOG_* constants

// Define log levels based on debug flag
#ifdef RECURSIVEMUTEXGUARD_DEBUG
    // Debug mode: Show all levels
    #define RMUTEXG_LOG_LEVEL_E ESP_LOG_ERROR
    #define RMUTEXG_LOG_LEVEL_W ESP_LOG_WARN
    #define RMUTEXG_LOG_LEVEL_I ESP_LOG_INFO
    #define RMUTEXG_LOG_LEVEL_D ESP_LOG_DEBUG
    #define RMUTEXG_LOG_LEVEL_V ESP_LOG_VERBOSE
#else
    // Release mode: Only Error, Warn, Info
    #define RMUTEXG_LOG_LEVEL_E ESP_LOG_ERROR
    #define RMUTEXG_LOG_LEVEL_W ESP_LOG_WARN
    #define RMUTEXG_LOG_LEVEL_I ESP_LOG_INFO
    #define RMUTEXG_LOG_LEVEL_D ESP_LOG_NONE  // Suppress
    #define RMUTEXG_LOG_LEVEL_V ESP_LOG_NONE  // Suppress
#endif

// Route to custom logger or ESP-IDF
#ifdef USE_CUSTOM_LOGGER
    #include <LogInterface.h>
    #define RMUTEXG_LOG_E(...) LOG_WRITE(RMUTEXG_LOG_LEVEL_E, RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_W(...) LOG_WRITE(RMUTEXG_LOG_LEVEL_W, RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_I(...) LOG_WRITE(RMUTEXG_LOG_LEVEL_I, RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_D(...) LOG_WRITE(RMUTEXG_LOG_LEVEL_D, RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_V(...) LOG_WRITE(RMUTEXG_LOG_LEVEL_V, RMUTEXG_LOG_TAG, __VA_ARGS__)
#else
    // ESP-IDF logging with compile-time suppression
    #include <esp_log.h>
    #define RMUTEXG_LOG_E(...) ESP_LOGE(RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_W(...) ESP_LOGW(RMUTEXG_LOG_TAG, __VA_ARGS__)
    #define RMUTEXG_LOG_I(...) ESP_LOGI(RMUTEXG_LOG_TAG, __VA_ARGS__)
    #ifdef RECURSIVEMUTEXGUARD_DEBUG
        #define RMUTEXG_LOG_D(...) ESP_LOGD(RMUTEXG_LOG_TAG, __VA_ARGS__)
        #define RMUTEXG_LOG_V(...) ESP_LOGV(RMUTEXG_LOG_TAG, __VA_ARGS__)
    #else
        #define RMUTEXG_LOG_D(...) ((void)0)
        #define RMUTEXG_LOG_V(...) ((void)0)
    #endif
#endif

// Keep backward compatibility with old debug macro
#ifdef RECURSIVE_MUTEX_GUARD_DEBUG
    #define RECURSIVE_MUTEX_GUARD_LOG(...) RMUTEXG_LOG_I(__VA_ARGS__)
#else
    #define RECURSIVE_MUTEX_GUARD_LOG(...) ((void)0)
#endif

#endif // RECURSIVEMUTEXGUARD_LOGGING_H