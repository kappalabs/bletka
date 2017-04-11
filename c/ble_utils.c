
#include "ble_utils.h"


/**
 * Buffer for interrupt-driven USART receive.
 */
volatile char _usart_rb[RECEIVE_BUFFER_LEN];
/**
 * Position of character in '_receive_buffer' to be read from.
 */
volatile char _usart_rbr;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
/**
 * Position of character in '_receive_buffer' to be written to.
 * Used from assembly code.
 */
volatile char _usart_rbw;
#pragma clang diagnostic pop


char *ble_nbuff(char *buffer, unsigned char length) {
    memcpy(buffer, (const void *) (_usart_rb + _usart_rbr), length);
    return buffer;
}

char *ble_buff(char *buffer) {
    return ble_nbuff(buffer, MAX_BLE_MSG_LENGTH);
}

void ble_receive_buffer_reset(void) {
    _usart_rbr = 0;
    _usart_rbw = 0;
    memset((void *) _usart_rb, 0x00, RECEIVE_BUFFER_LEN);
}

/**
 * Actively waits until the required 'string' is received
 * from the BLE module.
 *
 * @param string String expected to be received.
 * @param len Length of the expected string.
 */
void ble_wait_for_string(char *string, unsigned char len) {
#ifdef DEBUG
    char pom[len + 2];
    memset(pom, 0x00, len + 2);
    pom[0] = '<'; pom[len + 1] = '>';
#endif
    do {
        delay_10ms();
    } while (strncmp(string, (const char *) (_usart_rb + _usart_rbr), len) != 0);
#ifdef DEBUG
    memcpy(pom + 1, _usart_rb + _usart_rbr, len);
    ble_ntransmit(pom, (unsigned char) (len + 2), true);
#endif
}

/**
 * Sends commands required for defined default BLE module configuration.
 */
void ble_init(void) {
    ble_receive_buffer_reset();
    ble_allow_receive();

    /* Wakes up the module if it sleeps */
    ble_wake_up();

    /* Reset the device to default settings */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_RENEW_CMD);
    BLE_AWAIT_STRING(BLE_RENEW_RESPONSE);

    /* Sets the bluetooth-visible name */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_NAME_CMD);
    BLE_AWAIT_STRING(BLE_NAME_RESPONSE);

    //NOTE: This command is probably not supported, does not have any effects
//    /* Sets bluetooth connection password */
//    ble_receive_buffer_reset();
//    BLE_NL_TRANSMIT(BLE_PASS_CMD);
//    BLE_AWAIT_STRING(BLE_PASS_RESPONSE);

    /* Applies the new settings */
    ble_send_string("AT+RESET\r\n");
    delay_1s();
}

void ble_safe_delay(void) {
    delay_100ms();
    delay_100ms();
}

/**
 * Sends long null-terminated string in small packets which BLE module can handle.
 * Waits after each packet, if safe transmission is required.
 *
 * @param string Null-terminated string to be send.
 * @param wait Decides if the method should after each packet wait.
 */
void ble_delaying_send_string(char *string, bool wait) {
    char len = (unsigned char) strlen(string);
    char *currStart = string;

    /* Send the given long message in minimal number of parts */
    while (len > 0) {
        ble_ntransmit(currStart, (unsigned char) MIN(MAX_BLE_MSG_LENGTH, len), true);
        currStart += MAX_BLE_MSG_LENGTH;
        len -= MAX_BLE_MSG_LENGTH;

        if (wait) {
            ble_safe_delay();
        }
    }
}

void ble_send_string(char *string) {
    ble_delaying_send_string(string, true);
}

void ble_wake_up(void) {
    ble_receive_buffer_reset();
    ble_send_string(BLE_WAKEUP_CMD);
    //NOTE: alternatively try to wait for BLE_WAKEUP_RESPONSE to speed this up
    delay_500ms();

    /* Allow interrupts from BLE again */
    ble_allow_receive();

    /* Enables external device connection notifications */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_NOTI1_CMD);
    BLE_AWAIT_STRING(BLE_NOTI1_RESPONSE);

    ble_receive_buffer_reset();
}

void ble_waiting_wake_up(void) {
    /* Allow interrupts from BLE */
    ble_allow_receive();

    /* Wakes up the BLE module */
    ble_receive_buffer_reset();
    ble_delaying_send_string(BLE_WAKEUP_CMD, false);
    BLE_AWAIT_STRING(BLE_WAKEUP_RESPONSE);

    /* Enables external device connection notifications */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_NOTI1_CMD);
    BLE_AWAIT_STRING(BLE_NOTI1_RESPONSE);

    /* Wait for external device connection */
    ble_receive_buffer_reset();
    BLE_AWAIT_STRING(BLE_CONN_RESPONSE);

    ble_receive_buffer_reset();
}

void ble_sleep(void) {
    //NOTE: sometimes sleep does not work if done too quickly
    delay_1s();

    /* Disables external device connection notifications */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_NOTI0_CMD);
    BLE_AWAIT_STRING(BLE_NOTI0_RESPONSE);

    /* Makes the module go to sleep mode */
    ble_receive_buffer_reset();
    BLE_NL_TRANSMIT(BLE_SLEEP_CMD);
    BLE_AWAIT_STRING(BLE_SLEEP_RESPONSE);

    /* Interrupts from BLE are now unnecessary */
    ble_disallow_receive();
}

void ble_waiting_sleep(void) {
    /* Wait for external device disconnection */
    ble_receive_buffer_reset();
    BLE_AWAIT_STRING(BLE_LOST_RESPONSE);

    /* Go to sleep mode */
    ble_sleep();
}
