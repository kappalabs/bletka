#ifndef BLETKA_UTILS_H
#define BLETKA_UTILS_H

/**
 * ####################################################################
 *  UTILITIES FOR GENERAL PURPOSES
 * --------------------------------------------------------------------
 *  - provides methods to work with LED & speaker
 * ####################################################################
 */


#include <stdbool.h>
#include <string.h>

#include "main.h"


/**
 * Makes the LED blink at 1Hz once.
 */
extern void led_blink(void);

extern void speaker_err(void);

extern void speaker_ok(void);

/**
 * Compares two strings that end with '\n' character. Their length
 * must be at most 255 characters.
 *
 * @param str1 First string to be compared.
 * @param str2 Second string to be compared.
 * @return If the strings are equal including the last ('\n') character.
 */
extern bool streq(char *str1, char *str2);


#endif //BLETKA_UTILS_H
