#pragma once

#include <stdbool.h>

/** initialize LEDs */
void leds_init();

/** set the white flash led 
 * @param on true to turn on, false to turn off
 */
void set_white_led(bool on);

/** set the red indicator led (slightly hidden) 
 * @param on true to turn on, false to turn off
 */
void set_red_led(bool on);
