#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Lock {
private:
  SemaphoreHandle_t semaphore;
public:
  Lock();
  void lock();
  void unlock();
};