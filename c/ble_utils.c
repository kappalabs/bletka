
#include "ble_utils.h"


void ble_init(void) {
    /* Wait for the BLE module to be ready */
    delay_1s();

    /* Send initializing BLE commands */
    ble_send_string("AT+RENEW\r\n");
    ble_send_string("AT+NAME"DEVICE_NAME"\r\n");
    ble_send_string("AT+PASS"DEVICE_PASSWORD"\r\n");
    ble_send_string("AT+RESET\r\n");
}

void ble_send_string(char *string) {
    char len = (unsigned char) strlen(string);
    char *currStart = string;

    /* Send the given long message in minimal number of parts */
    while (len > 0) {
        ble_ntransmit(currStart, (unsigned char) MIN(MAX_BLE_MSG_LENGTH, len));
        currStart += MAX_BLE_MSG_LENGTH;
        len -= MAX_BLE_MSG_LENGTH;
        delay_100ms();
        delay_100ms();
    }
}

void ble_wake_up(void) {
    /* Send string with >80 characters to wake the BLE module up */
    ble_send_string("000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    char ret[20];
    do {
        ble_nreceive(ret, 20);
    } while (!streq("OK+CONN\r\n", ret));
    led_blink();
    led_blink();
}

void ble_sleep(void) {
    ble_send_string("AT+SLEEP");
}
