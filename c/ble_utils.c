
#include "ble_utils.h"


char _ret_buff[MAX_BLE_MSG_LENGTH];


void ble_wait_for_string(char *string, unsigned char len) {
//    char pom[len + 2];
//    memset(pom, 0x00, len + 2);
//    pom[0] = '<'; pom[len + 1] = '>';
    do {
//        memset(_ret_buff, 0x00, MAX_BLE_MSG_LENGTH);
        ble_nreceive(_ret_buff, len, true);
//        memcpy(pom + 1, _ret_buff, len);
//        ble_ntransmit(pom, (unsigned char) (len + 2), true);
    } while (strncmp(string, _ret_buff, len) != 0);

    delay_1s();
}

/**
 * Clears the input USART buffer.
 */
void ble_clear_input(void) {
    BLE_NL_TRANSMIT(BLE_AT_CMD);
//    memset(_ret_buff, 0x00, MAX_BLE_MSG_LENGTH);
    ble_nreceive(_ret_buff, MAX_BLE_MSG_LENGTH, false);

//    char pom[MAX_BLE_MSG_LENGTH + 2];
//    memset(pom, 0x00, MAX_BLE_MSG_LENGTH + 2);
//    pom[0] = '{'; pom[MAX_BLE_MSG_LENGTH + 1] = '}';
//    memcpy(pom + 1, _ret_buff, MAX_BLE_MSG_LENGTH);
//    ble_ntransmit(pom, (unsigned char) (MAX_BLE_MSG_LENGTH + 2), true);

//    delay_100ms();
//    delay_100ms();
//    delay_100ms();
//    delay_500ms();
}

/**
 * Sends commands required for defined default BLE module configuration.
 */
void ble_init(void) {
    /* Wakes up the module if it sleeps */
    ble_wake_up();
    ble_clear_input();

    /* Reset the device to default settings */
    BLE_NL_TRANSMIT(BLE_RENEW_CMD);
    BLE_AWAIT_STRING(BLE_RENEW_RESPONSE);
    ble_clear_input();

    /* Sets the bluetooth-visible name */
    BLE_NL_TRANSMIT(BLE_NAME_CMD);
    BLE_AWAIT_STRING(BLE_NAME_RESPONSE);
    ble_clear_input();

    //NOTE: This command is probably not supported, does nothing
//    /* Sets bluetooth connection password */
//    BLE_NL_TRANSMIT(BLE_PASS_CMD);
//    BLE_AWAIT_STRING(BLE_PASS_RESPONSE);
//    ble_clear_input();

    /* Applies the new settings */
    ble_send_string("AT+RESET\r\n");
    delay_1s();
    ble_clear_input();
}

void ble_delaying_send_string(char *string, int ms_hundreds) {
    char len = (unsigned char) strlen(string);
    char *currStart = string;

    /* Send the given long message in minimal number of parts */
    while (len > 0) {
        ble_ntransmit(currStart, (unsigned char) MIN(MAX_BLE_MSG_LENGTH, len), true);
        currStart += MAX_BLE_MSG_LENGTH;
        len -= MAX_BLE_MSG_LENGTH;

        int i;
        for (i = 0; i < ms_hundreds; ++i) {
            delay_100ms();
        }
    }
}

void ble_send_string(char *string) {
    ble_delaying_send_string(string, 2);
}

/**
 * Send string with >80 characters to wake the BLE module up.
 * The module does not need to be sleeping to successfully run this method.
 */
void ble_wake_up(void) {
    ble_send_string(BLE_WAKEUP_CMD);
}

/**
 * Wakes up the BLE module, which must be currently in sleep mode.
 * Waits until external device is connected.
 */
void ble_waiting_wake_up(void) {
    /* Wakes up the BLE module */
    ble_delaying_send_string(BLE_WAKEUP_CMD, 0);
    BLE_AWAIT_STRING(BLE_WAKEUP_RESPONSE);
    ble_clear_input();

    /* Enables external device connection notifications */
    BLE_NL_TRANSMIT(BLE_NOTI1_CMD);
    BLE_AWAIT_STRING(BLE_NOTI1_RESPONSE);
    ble_clear_input();

    //TODO
    led_blink();
    led_blink();
}

void ble_sleep(void) {
    /* Disables external device connection notifications */
    BLE_NL_TRANSMIT(BLE_NOTI0_CMD);
    BLE_AWAIT_STRING(BLE_NOTI0_RESPONSE);
    ble_clear_input();

    /* Makes the module go to sleep mode */
    BLE_NL_TRANSMIT(BLE_SLEEP_CMD);
    BLE_AWAIT_STRING(BLE_SLEEP_RESPONSE);

    //TODO
    speaker_err();
}
