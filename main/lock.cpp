#include "lock.h"

Lock::Lock() {
  semaphore = xSemaphoreCreateRecursiveMutex();
}

void Lock::lock() {
  xSemaphoreTakeRecursive(semaphore, portMAX_DELAY);
}

void Lock::unlock() {
  xSemaphoreGiveRecursive(semaphore);
}