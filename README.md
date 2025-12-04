# MutexGuard Library

A modern C++ RAII-based mutex guard library for ESP32 FreeRTOS applications. This library provides automatic mutex management through the RAII (Resource Acquisition Is Initialization) pattern, ensuring mutexes are always properly released.

## Overview

The MutexGuard library provides two main classes:
- `MutexGuard`: For standard FreeRTOS mutexes
- `RecursiveMutexGuard`: For recursive FreeRTOS mutexes

Both classes automatically lock the mutex on construction and unlock it on destruction, preventing common synchronization bugs like forgotten unlocks or unlocks in error paths.

## Features

- **RAII Pattern**: Automatic mutex management prevents resource leaks
- **Exception Safe**: Mutex is always released, even if exceptions occur
- **Timeout Support**: Configurable timeout for lock acquisition
- **ISR Protection**: Detects and prevents usage from interrupt context
- **Debug Mode**: Optional logging for debugging synchronization issues
- **Thread Safe**: Designed for multi-threaded FreeRTOS applications
- **Zero Overhead**: No runtime cost when debug mode is disabled
- **Type Safe**: Strong typing prevents mutex type mismatches

## Installation

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/packerlschupfer/MutexGuard.git#v0.1.0
```

### Logging Configuration

This library supports flexible logging configuration:

#### Using ESP-IDF Logging (Default)
No configuration needed. The library will use ESP-IDF logging.

#### Using Custom Logger
Define `USE_CUSTOM_LOGGER` in your build flags:
```ini
build_flags = -DUSE_CUSTOM_LOGGER
```

In your application's main.cpp:
```cpp
#include <Logger.h>
#include <LogInterfaceImpl.h>

void setup() {
    Logger::getInstance().init(1024);
    // MutexGuard will now log through your custom Logger
}
```

#### Debug Logging
To enable debug/verbose logging for this library:
```ini
build_flags = 
    -DMUTEXGUARD_DEBUG      ; Enable debug for MutexGuard
    -DRECURSIVEMUTEXGUARD_DEBUG  ; Enable debug for RecursiveMutexGuard
```

Or for backward compatibility:
```ini
build_flags = 
    -DMUTEX_GUARD_DEBUG     ; Old style debug flag for MutexGuard
    -DRECURSIVE_MUTEX_GUARD_DEBUG  ; Old style debug flag for RecursiveMutexGuard
```

#### Complete Example
```ini
[env:debug]
build_flags = 
    -DUSE_CUSTOM_LOGGER     ; Use custom logger
    -DMUTEXGUARD_DEBUG      ; Enable debug for this library
```

### Manual Installation

1. Clone or download this repository
2. Copy the `src` folder contents to your project
3. Include the appropriate header file in your code

## Usage

### Basic Mutex Guard

```cpp
#include "MutexGuard.h"

SemaphoreHandle_t myMutex = xSemaphoreCreateMutex();

void criticalOperation() {
    // Mutex is automatically locked here
    MutexGuard lock(myMutex);
    
    if (lock.hasLock()) {
        // Critical section - mutex is held
        performCriticalWork();
        // Mutex is automatically unlocked when 'lock' goes out of scope
    } else {
        // Failed to acquire mutex within timeout
        ESP_LOGW(TAG, "Could not acquire mutex");
    }
}
```

### Using Custom Timeout

```cpp
void timeoutExample() {
    // Wait up to 500ms for the mutex
    MutexGuard lock(myMutex, pdMS_TO_TICKS(500));
    
    if (lock) {  // Convenient boolean conversion
        // Got the mutex
        doWork();
    }
}
```

### Manual Unlock

```cpp
void earlyReleaseExample() {
    MutexGuard lock(myMutex);
    
    if (lock.hasLock()) {
        if (checkCondition()) {
            lock.unlock();  // Manually release early
            // Mutex is now free
        }
        // Safe to call unlock() again - it's idempotent
    }
}
```

### Recursive Mutex Guard

```cpp
#include "RecursiveMutexGuard.h"

SemaphoreHandle_t recursiveMutex = xSemaphoreCreateRecursiveMutex();

void recursiveFunction(int depth) {
    RecursiveMutexGuard lock(recursiveMutex);
    
    if (lock.hasLock()) {
        ESP_LOGI(TAG, "Depth: %d", depth);
        
        if (depth > 0) {
            // Same task can lock again
            recursiveFunction(depth - 1);
        }
    }
}
```


## API Reference

### MutexGuard Class

#### Constructor
```cpp
explicit MutexGuard(SemaphoreHandle_t handle, TickType_t timeout = pdMS_TO_TICKS(100))
```
- `handle`: The FreeRTOS mutex handle to lock
- `timeout`: Timeout in ticks to wait for the mutex (default: 100ms)

#### Methods
- `bool hasLock() const`: Returns true if the mutex was successfully acquired
- `bool isValid() const`: Returns true if the mutex handle is valid (not null)
- `void unlock()`: Manually unlock the mutex (safe to call multiple times)
- `operator bool() const`: Allows usage in boolean contexts

### RecursiveMutexGuard Class

The RecursiveMutexGuard class has the same API as MutexGuard but works with recursive mutexes created with `xSemaphoreCreateRecursiveMutex()`.

## Design Patterns

### RAII (Resource Acquisition Is Initialization)

The library implements the RAII pattern, which ties resource management to object lifetime:

```cpp
{
    MutexGuard lock(mutex);  // Mutex locked
    if (lock) {
        // Do work
    }
}  // Mutex automatically unlocked here
```

### Scoped Locking

Use scoped blocks to control mutex lifetime:

```cpp
void processData() {
    prepareData();
    
    {  // Start of critical section
        MutexGuard lock(dataMutex);
        if (lock) {
            modifySharedData();
        }
    }  // End of critical section - mutex released
    
    postProcess();
}
```

## Thread Safety Example

```cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MutexGuard.h"

class ThreadSafeCounter {
private:
    SemaphoreHandle_t mutex;
    int count;
    
public:
    ThreadSafeCounter() : count(0) {
        mutex = xSemaphoreCreateMutex();
    }
    
    ~ThreadSafeCounter() {
        vSemaphoreDelete(mutex);
    }
    
    void increment() {
        MutexGuard lock(mutex);
        if (lock) {
            count++;
        }
    }
    
    int getValue() {
        MutexGuard lock(mutex);
        if (lock) {
            return count;
        }
        return -1;  // Error
    }
};
```

## Best Practices

1. **Always Check Lock Success**: Use `hasLock()` or the boolean operator to verify lock acquisition
2. **Keep Critical Sections Small**: Minimize the code within the mutex guard scope
3. **Avoid Nested Locks**: Unless using recursive mutexes, avoid locking multiple mutexes
4. **Use Appropriate Timeouts**: Set timeouts based on your application's requirements
5. **Enable Debug Mode During Development**: Helps identify synchronization issues

## Performance Considerations

- **Zero Overhead**: When debug mode is disabled, the classes have minimal overhead beyond the FreeRTOS mutex operations
- **Stack Usage**: Each guard uses minimal stack space (typically 8-16 bytes)
- **ISR Check**: The ISR context check (`xPortInIsrContext()`) has negligible performance impact

## Limitations

- Cannot be used from ISR context (by design - FreeRTOS limitation)
- Copy and move operations are disabled for safety
- Requires FreeRTOS (ESP-IDF or Arduino with FreeRTOS)

## License

This library is released under the GPL-3 License. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Support

For issues, questions, or contributions, please visit the [GitHub repository](https://github.com/packerlschupfer/MutexGuard).