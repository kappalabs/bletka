
#include "utils.h"


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
