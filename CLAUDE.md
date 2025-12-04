# MutexGuard - CLAUDE.md

## Overview
RAII mutex guard for FreeRTOS semaphores. Automatically locks in constructor and unlocks in destructor, preventing forgotten unlocks and ensuring exception safety.

## Key Features
- RAII pattern for automatic mutex management
- Timeout support for lock acquisition
- ISR context detection and protection
- Non-copyable, non-movable (safe ownership)
- Debug logging support

## Usage
```cpp
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

void criticalSection() {
    MutexGuard lock(mutex, pdMS_TO_TICKS(100));
    if (lock.hasLock()) {
        // Protected code here
        // Mutex released automatically when lock goes out of scope
    } else {
        // Failed to acquire within timeout
    }
}
```

## Also Includes

### RecursiveMutexGuard
For recursive mutex support (same task can lock multiple times):
```cpp
SemaphoreHandle_t rmutex = xSemaphoreCreateRecursiveMutex();
RecursiveMutexGuard lock(rmutex);
```

## Best Practices
1. Always check `hasLock()` before accessing protected data
2. Keep critical sections as short as possible
3. Avoid calling functions that might also need the mutex (use RecursiveMutexGuard if needed)
4. Use reasonable timeouts to prevent deadlocks

## Thread Safety
- Designed specifically for multi-threaded environments
- ISR detection prevents usage from interrupt context
- Atomic operations for state tracking

## Build Configuration
```ini
build_flags =
    -DMUTEXGUARD_DEBUG  ; Enable debug logging with timing
```
