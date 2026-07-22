/*
 * RecursiveMutexGuard.cpp - part of the ESP32-MutexGuard library
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

#include "RecursiveMutexGuard.h"

RecursiveMutexGuard::RecursiveMutexGuard(SemaphoreHandle_t handle, TickType_t timeout)
    : m_handle(handle), m_taken(false) {
    
    // Check for null handle
    if (m_handle == nullptr) {
        RMUTEXG_LOG_W("Attempted to create RecursiveMutexGuard with null handle");
        return;
    }
    
    // Check if we're in ISR context
    if (xPortInIsrContext()) {
        RMUTEXG_LOG_E("Cannot use RecursiveMutexGuard from ISR context");
        m_handle = nullptr;  // Invalidate to prevent unlock attempt
        return;
    }
    
    // Attempt to take the recursive mutex
    m_taken = (xSemaphoreTakeRecursive(m_handle, timeout) == pdTRUE);
    
    RECURSIVE_MUTEX_GUARD_LOG("Recursive mutex %s", m_taken ? "locked" : "failed to lock (timeout)");
}

RecursiveMutexGuard::~RecursiveMutexGuard() {
    unlock();
}

void RecursiveMutexGuard::unlock() noexcept {
    if (m_taken && m_handle != nullptr) {
        // Double-check we're not in ISR context
        if (xPortInIsrContext()) {
            RMUTEXG_LOG_E("Cannot unlock recursive mutex from ISR context");
            return;
        }

        xSemaphoreGiveRecursive(m_handle);
        m_taken = false;

        RECURSIVE_MUTEX_GUARD_LOG("Recursive mutex unlocked");
    }
}