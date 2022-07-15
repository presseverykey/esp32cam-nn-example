#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "camera.h"
#include "network.h"

#include "esp_log.h"
#define TAG "MAIN"

void LED_init() {
    gpio_config_t ledConf = {
        .pin_bit_mask = 1ULL << 4,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&ledConf);
}

void LED_set(bool on) {
    gpio_set_level(GPIO_NUM_4, on);
}

void app_main() {
    LED_init();
    printf("Hello!\n");
    camera_init();
    network_init();
    httpd_addGetHandler("/image.jpg", &jpg_httpd_handler);

    while (1) {
        LED_set(1);
        vTaskDelay(1 / portTICK_RATE_MS);
        LED_set(0);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}


