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