#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "leds.h"
#include "camera.h"
#include "network.h"
#include "ai.h"
#include "lock.h"
#include "config.h"

#include "esp_log.h"
#define TAG "MAIN"

const float MIN_CONFIDENCE = 0.85;

Lock frameLock;
camera_fb_t lastFrame;

void get_image_and_classify() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) {
    float result = ai_detect(fb->buf);
    ESP_LOGI(TAG, "Detection result: %f%%",result*100);
    set_white_led(result >= MIN_CONFIDENCE);
    frameLock.lock();
    memcpy(lastFrame.buf, fb->buf, lastFrame.len);
    esp_camera_fb_return(fb);
    frameLock.unlock();
  }  
}

esp_err_t get_image_handler(httpd_req_t *req) {
  frameLock.lock();
  esp_err_t err = httpd_send_framebuffer_as_jpg(&lastFrame, req);
  frameLock.unlock();
  return err;
}


extern "C" void app_main() {
    leds_init();
    ai_init();
    camera_init();
    network_init();

    lastFrame.width = IMAGE_WIDTH;
    lastFrame.height = IMAGE_HEIGHT;
    lastFrame.len = IMAGE_WIDTH * IMAGE_HEIGHT;
    lastFrame.format = PIXFORMAT_GRAYSCALE;
    lastFrame.buf = (uint8_t*)malloc(lastFrame.len);

    httpd_add_get_handler("/image.jpg", &get_image_handler);

    while (1) {
        set_red_led(1);
        get_image_and_classify();
        set_red_led(0);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}


