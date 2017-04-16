
#include <ctype.h>

#include "utils.h"
#include "ble_utils.h"


void led_blink(void) {
    led_on();
    delay_500ms();
    led_off();
    delay_500ms();
}

void speaker_err(void) {
    speaker_beep();
    delay_50ms();
    speaker_beep();
    delay_50ms();
    speaker_beep();
    delay_50ms();
}

void speaker_ok(void) {
    speaker_beep();
    speaker_beep();
    speaker_beep();
    speaker_beep();
    delay_50ms();
}

bool streq(char *str1, char *str2) {
    unsigned char pos = 0;
    while (pos < 255 && str1[pos] != '\n' && str2[pos] != '\n' && str1[pos] == str2[pos]) {
        ++pos;
    }
    return str1[pos] == '\n' && str2[pos] == '\n';
}

bool parse_number(char *buff, unsigned int *number) {
    /* Check if there is a number to be parsed */
    if (!isdigit(buff[0])) {
        return false;
    }

    *number = 0;
    unsigned int indx = 0;
    do {
        *number = *number * 10 + buff[indx] - '0';
    } while (indx <= MAX_BLE_MSG_LENGTH && buff[indx++] != 0x00 && isdigit(buff[indx]));

    return true;
}
