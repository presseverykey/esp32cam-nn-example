#include "leds.h"
#include "driver/gpio.h"

#define WHITE_LED_IO (gpio_num_t)4
#define RED_LED_IO (gpio_num_t)33

void leds_init() {
    gpio_config_t ledConf = {
        .pin_bit_mask = (1ULL << WHITE_LED_IO) | (1ULL << RED_LED_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&ledConf);
}

void set_white_led(bool on) {
    gpio_set_level(WHITE_LED_IO, on);
}

void set_red_led(bool on) {
    gpio_set_level(RED_LED_IO, on);
}
