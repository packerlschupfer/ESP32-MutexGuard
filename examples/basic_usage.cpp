/**
 * @file basic_usage.cpp
 * @brief Example demonstrating MutexGuard and RecursiveMutexGuard usage
 * 
 * This example shows various use cases for the MutexGuard library including:
 * - Basic mutex locking
 * - Timeout handling
 * - Recursive mutex usage
 * - Thread-safe shared resource access
 */

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MutexGuard.h"
#include "RecursiveMutexGuard.h"

// Enable debug logging
#define MUTEX_GUARD_DEBUG
#define RECURSIVE_MUTEX_GUARD_DEBUG

// Global mutexes
SemaphoreHandle_t dataMutex = nullptr;
SemaphoreHandle_t recursiveMutex = nullptr;

// Shared data
volatile int sharedCounter = 0;
volatile bool runTasks = true;

/**
 * @brief Simple task that increments a shared counter with mutex protection
 */
void incrementTask(void* parameter) {
    const char* taskName = (const char*)parameter;
    
    while (runTasks) {
        // Use MutexGuard for automatic locking/unlocking
        MutexGuard lock(dataMutex, pdMS_TO_TICKS(50));
        
        if (lock.hasLock()) {
            // Critical section
            int oldValue = sharedCounter;
            vTaskDelay(pdMS_TO_TICKS(1));  // Simulate work
            sharedCounter = oldValue + 1;
            
            Serial.printf("[%s] Incremented counter: %d -> %d\n", 
                         taskName, oldValue, sharedCounter);
        } else {
            Serial.printf("[%s] Failed to acquire mutex\n", taskName);
        }
        
        // Mutex automatically released here
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    vTaskDelete(NULL);
}

/**
 * @brief Demonstrates recursive mutex usage
 */
void recursiveFunction(int depth, int maxDepth) {
    RecursiveMutexGuard lock(recursiveMutex);
    
    if (!lock) {
        Serial.println("Failed to acquire recursive mutex");
        return;
    }
    
    Serial.printf("Recursive call depth: %d\n", depth);
    
    if (depth < maxDepth) {
        // Same task can lock the recursive mutex again
        recursiveFunction(depth + 1, maxDepth);
    }
    
    Serial.printf("Returning from depth: %d\n", depth);
}

/**
 * @brief Task demonstrating recursive mutex
 */
void recursiveTask(void* parameter) {
    while (runTasks) {
        Serial.println("\n--- Starting recursive operation ---");
        recursiveFunction(1, 3);
        Serial.println("--- Recursive operation complete ---\n");
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    vTaskDelete(NULL);
}

/**
 * @brief Demonstrates early mutex release
 */
void earlyReleaseExample() {
    Serial.println("\n--- Early release example ---");
    
    MutexGuard lock(dataMutex);
    
    if (lock.hasLock()) {
        Serial.println("Mutex acquired");
        
        // Do some work
        int value = sharedCounter;
        
        // Conditionally release early
        if (value > 10) {
            Serial.println("Releasing mutex early");
            lock.unlock();
            
            // Mutex is now free for other tasks
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        // Safe to call unlock again (idempotent)
        lock.unlock();
    }
}

/**
 * @brief Thread-safe class example
 */
class ThreadSafeBuffer {
private:
    static const size_t BUFFER_SIZE = 10;
    int buffer[BUFFER_SIZE];
    size_t writeIndex;
    size_t readIndex;
    size_t count;
    SemaphoreHandle_t mutex;
    
public:
    ThreadSafeBuffer() : writeIndex(0), readIndex(0), count(0) {
        mutex = xSemaphoreCreateMutex();
        memset(buffer, 0, sizeof(buffer));
    }
    
    ~ThreadSafeBuffer() {
        if (mutex) {
            vSemaphoreDelete(mutex);
        }
    }
    
    bool push(int value) {
        MutexGuard lock(mutex, pdMS_TO_TICKS(100));
        
        if (!lock) {
            return false;  // Timeout
        }
        
        if (count >= BUFFER_SIZE) {
            return false;  // Buffer full
        }
        
        buffer[writeIndex] = value;
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
        count++;
        
        return true;
    }
    
    bool pop(int& value) {
        MutexGuard lock(mutex, pdMS_TO_TICKS(100));
        
        if (!lock) {
            return false;  // Timeout
        }
        
        if (count == 0) {
            return false;  // Buffer empty
        }
        
        value = buffer[readIndex];
        readIndex = (readIndex + 1) % BUFFER_SIZE;
        count--;
        
        return true;
    }
    
    size_t size() {
        MutexGuard lock(mutex);
        return lock ? count : 0;
    }
};

// Global buffer for demonstration
ThreadSafeBuffer* safeBuffer = nullptr;

/**
 * @brief Producer task for thread-safe buffer
 */
void producerTask(void* parameter) {
    int value = 0;
    
    while (runTasks) {
        if (safeBuffer->push(value)) {
            Serial.printf("Producer: Added %d to buffer\n", value);
            value++;
        } else {
            Serial.println("Producer: Buffer full or timeout");
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    vTaskDelete(NULL);
}

/**
 * @brief Consumer task for thread-safe buffer
 */
void consumerTask(void* parameter) {
    int value;
    
    while (runTasks) {
        if (safeBuffer->pop(value)) {
            Serial.printf("Consumer: Got %d from buffer\n", value);
        } else {
            Serial.println("Consumer: Buffer empty or timeout");
        }
        
        vTaskDelay(pdMS_TO_TICKS(750));
    }
    
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== MutexGuard Example Starting ===\n");
    
    // Create mutexes
    dataMutex = xSemaphoreCreateMutex();
    recursiveMutex = xSemaphoreCreateRecursiveMutex();
    
    if (dataMutex == nullptr || recursiveMutex == nullptr) {
        Serial.println("Failed to create mutexes!");
        return;
    }
    
    // Create thread-safe buffer
    safeBuffer = new ThreadSafeBuffer();
    
    // Create tasks
    xTaskCreate(incrementTask, "Task1", 4096, (void*)"Task1", 1, NULL);
    xTaskCreate(incrementTask, "Task2", 4096, (void*)"Task2", 1, NULL);
    xTaskCreate(recursiveTask, "RecursiveTask", 4096, NULL, 1, NULL);
    xTaskCreate(producerTask, "Producer", 4096, NULL, 1, NULL);
    xTaskCreate(consumerTask, "Consumer", 4096, NULL, 1, NULL);
    
    Serial.println("Tasks created successfully\n");
}

void loop() {
    static unsigned long lastReport = 0;
    static int iteration = 0;
    
    unsigned long now = millis();
    
    // Report status every 3 seconds
    if (now - lastReport >= 3000) {
        lastReport = now;
        
        // Use mutex to safely read shared counter
        {
            MutexGuard lock(dataMutex);
            if (lock) {
                Serial.printf("\n[Main] Shared counter value: %d\n", sharedCounter);
            }
        }
        
        Serial.printf("[Main] Buffer size: %d\n", safeBuffer->size());
        
        // Demonstrate early release after a few iterations
        if (++iteration == 3) {
            earlyReleaseExample();
        }
        
        // Stop after 10 iterations
        if (iteration >= 10) {
            Serial.println("\n=== Stopping example ===");
            runTasks = false;
            
            // Clean up
            delay(1000);
            vSemaphoreDelete(dataMutex);
            vSemaphoreDelete(recursiveMutex);
            delete safeBuffer;
            
            Serial.println("=== Example complete ===");
            while (true) {
                delay(1000);
            }
        }
    }
    
    delay(10);
}