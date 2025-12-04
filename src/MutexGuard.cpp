#include "MutexGuard.h"

MutexGuard::MutexGuard(SemaphoreHandle_t handle, TickType_t timeout)
    : m_handle(handle), m_taken(false) {
    
    // Check for null handle
    if (m_handle == nullptr) {
        MUTEXG_LOG_W("Attempted to create MutexGuard with null handle");
        return;
    }
    
    // Check if we're in ISR context
    if (xPortInIsrContext()) {
        MUTEXG_LOG_E("Cannot use MutexGuard from ISR context");
        m_handle = nullptr;  // Invalidate to prevent unlock attempt
        return;
    }
    
    // Attempt to take the mutex
    m_taken = (xSemaphoreTake(m_handle, timeout) == pdTRUE);
    
    MUTEX_GUARD_LOG("Mutex %s", m_taken ? "locked" : "failed to lock (timeout)");
}

MutexGuard::~MutexGuard() {
    unlock();
}

void MutexGuard::unlock() noexcept {
    if (m_taken && m_handle != nullptr) {
        // Double-check we're not in ISR context
        if (xPortInIsrContext()) {
            MUTEXG_LOG_E("Cannot unlock mutex from ISR context");
            return;
        }

        xSemaphoreGive(m_handle);
        m_taken = false;

        MUTEX_GUARD_LOG("Mutex unlocked");
    }
}