#pragma once

/** initialize the tensorflow network */
void ai_init();

/** run a detection
 * @param bitmap input image, size 96 x 96, grayscale
 * @return probability of a person, -1 if an error occurred
 */
float ai_detect(uint8_t* bitmap);