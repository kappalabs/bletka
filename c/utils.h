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
 * Makes the LED blink once at 1Hz once.
 */
void led_blink(void);

/**
 * Makes error sound.
 */
void speaker_err(void);

/**
 * Makes OK sound.
 */
void speaker_ok(void);

/**
 * Compares two strings that end with '\n' character. Their length
 * must be at most 255 characters.
 *
 * @param str1 First string to be compared.
 * @param str2 Second string to be compared.
 * @return If the strings are equal including the last ('\n') character.
 */
bool streq(char *str1, char *str2);

/**
 * Parses a decimal number from given string.
 *
 * @param buff String containing the decimal number in human-readable form.
 * @param number Will contain decoded number.
 * @return If the operation was successful.
 */
bool parse_number(char *buff, unsigned int *number);


#endif //BLETKA_UTILS_H
