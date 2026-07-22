/*
 * RecursiveMutexGuard.h - part of the ESP32-MutexGuard library
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

#ifndef _RECURSIVEMUTEXGUARD_H_
#define _RECURSIVEMUTEXGUARD_H_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "RecursiveMutexGuardLogging.h"

/**
 * @brief RAII recursive mutex guard for automatic recursive mutex management
 * 
 * This class provides automatic recursive mutex locking and unlocking using RAII pattern.
 * The recursive mutex is locked in the constructor and automatically unlocked in the destructor,
 * ensuring exception safety and preventing forgotten unlocks.
 * 
 * Recursive mutexes can be locked multiple times by the same task and must be
 * unlocked the same number of times before other tasks can acquire the mutex.
 * 
 * Features:
 * - Automatic recursive mutex management (RAII pattern)
 * - Timeout support for lock acquisition
 * - ISR context detection and protection
 * - Debug logging support
 * - Thread-safe operations
 * - Same task can lock multiple times
 * 
 * Usage:
 * @code
 * void recursiveFunction(SemaphoreHandle_t mutex, int depth) {
 *     RecursiveMutexGuard lock(mutex);
 *     if (lock.hasLock()) {
 *         // Can safely call this function recursively
 *         if (depth > 0) {
 *             recursiveFunction(mutex, depth - 1);
 *         }
 *     }
 * }
 * @endcode
 */
class RecursiveMutexGuard {
public:
    /**
     * @brief Construct a new Recursive Mutex Guard and attempt to lock the mutex
     * 
     * @param handle The FreeRTOS recursive mutex handle to lock
     * @param timeout Timeout in ticks to wait for the mutex (default: 100ms)
     */
    explicit RecursiveMutexGuard(SemaphoreHandle_t handle, TickType_t timeout = pdMS_TO_TICKS(100));
    
    /**
     * @brief Destroy the Recursive Mutex Guard and unlock the mutex if it was locked
     */
    ~RecursiveMutexGuard();
    
    // Delete copy constructor and assignment operator to prevent double-release
    RecursiveMutexGuard(const RecursiveMutexGuard&) = delete;
    RecursiveMutexGuard& operator=(const RecursiveMutexGuard&) = delete;
    
    // Delete move semantics for safety
    RecursiveMutexGuard(RecursiveMutexGuard&&) = delete;
    RecursiveMutexGuard& operator=(RecursiveMutexGuard&&) = delete;
    
    /**
     * @brief Check if the recursive mutex was successfully locked
     * @return true if the mutex is currently held by this guard
     */
    bool hasLock() const noexcept { return m_taken; }

    /**
     * @brief Check if the recursive mutex handle is valid
     * @return true if the mutex handle is not null
     */
    bool isValid() const noexcept { return m_handle != nullptr; }

    /**
     * @brief Manually unlock the recursive mutex before the guard is destroyed
     *
     * This method is safe to call multiple times. After calling unlock(),
     * hasLock() will return false.
     */
    void unlock() noexcept;

    /**
     * @brief Convert to bool for convenient if-statement usage
     * @return true if the mutex is locked
     */
    explicit operator bool() const noexcept { return hasLock(); }

private:
    SemaphoreHandle_t m_handle;  ///< The recursive mutex handle
    bool m_taken;                ///< Whether the mutex was successfully taken
};

#endif // _RECURSIVEMUTEXGUARD_H_