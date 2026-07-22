/*
 * RecursiveMutexGuardLogging.h - part of the ESP32-MutexGuard library
 *
 * Copyright (C) 2025-2026 packerlschupfer
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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