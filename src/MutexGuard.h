#ifndef _MUTEXGUARD_H_
#define _MUTEXGUARD_H_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "MutexGuardLogging.h"

/**
 * @brief RAII mutex guard for automatic mutex management
 * 
 * This class provides automatic mutex locking and unlocking using RAII pattern.
 * The mutex is locked in the constructor and automatically unlocked in the destructor,
 * ensuring exception safety and preventing forgotten unlocks.
 * 
 * Features:
 * - Automatic mutex management (RAII pattern)
 * - Timeout support for lock acquisition
 * - ISR context detection and protection
 * - Debug logging support
 * - Thread-safe operations
 * 
 * Usage:
 * @code
 * {
 *     MutexGuard lock(myMutex, pdMS_TO_TICKS(100));
 *     if (lock.hasLock()) {
 *         // Critical section - mutex is held
 *         // Mutex will be automatically released when 'lock' goes out of scope
 *     } else {
 *         // Failed to acquire mutex within timeout
 *     }
 * }
 * @endcode
 */
class MutexGuard {
public:
    /**
     * @brief Construct a new Mutex Guard and attempt to lock the mutex
     * 
     * @param handle The FreeRTOS mutex handle to lock
     * @param timeout Timeout in ticks to wait for the mutex (default: 100ms)
     */
    explicit MutexGuard(SemaphoreHandle_t handle, TickType_t timeout = pdMS_TO_TICKS(100));
    
    /**
     * @brief Destroy the Mutex Guard and unlock the mutex if it was locked
     */
    ~MutexGuard();
    
    // Delete copy constructor and assignment operator to prevent double-release
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
    
    // Delete move semantics for safety
    MutexGuard(MutexGuard&&) = delete;
    MutexGuard& operator=(MutexGuard&&) = delete;
    
    /**
     * @brief Check if the mutex was successfully locked
     * @return true if the mutex is currently held by this guard
     */
    bool hasLock() const noexcept { return m_taken; }

    /**
     * @brief Check if the mutex handle is valid
     * @return true if the mutex handle is not null
     */
    bool isValid() const noexcept { return m_handle != nullptr; }

    /**
     * @brief Manually unlock the mutex before the guard is destroyed
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
    SemaphoreHandle_t m_handle;  ///< The mutex handle
    bool m_taken;                ///< Whether the mutex was successfully taken
};

#endif // _MUTEXGUARD_H_