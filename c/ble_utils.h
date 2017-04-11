#ifndef BLETKA_BLE_UTILS_H
#define BLETKA_BLE_UTILS_H

/**
 * ####################################################################
 *  PROVIDES UTILITIES FOR BLE MODULE
 * --------------------------------------------------------------------
 * ####################################################################
 */


#include <string.h>

#include "main.h"
#include "utils.h"


#define DEVICE_NAME         "BLETKA" // max length is 12 chars
#define DEVICE_PASSWORD     "112233" // must be 6 characters long!

#define BLE_NL              "\r\n"
#define BLE_OK              "OK"BLE_NL
#define BLE_AT_CMD          "AT"BLE_NL
#define BLE_AT_RESPONSE     BLE_OK
#define BLE_RENEW_CMD       "AT+RENEW"BLE_NL
#define BLE_RENEW_RESPONSE  "+RENEW"BLE_NL""BLE_OK
#define BLE_NAME_CMD        "AT+NAME"DEVICE_NAME""BLE_NL
#define BLE_NAME_RESPONSE   "+NAME="DEVICE_NAME""BLE_NL""BLE_OK
#define BLE_PASS_CMD        "AT+PASS"DEVICE_PASSWORD""BLE_NL
#define BLE_PASS_RESPONSE   "+PASS="DEVICE_PASSWORD""BLE_NL""BLE_OK
#define BLE_NOTI0_CMD       "AT+NOTI0"BLE_NL
#define BLE_NOTI0_RESPONSE  "+NOTI=0"BLE_NL""BLE_OK
#define BLE_NOTI1_CMD       "AT+NOTI1"BLE_NL
#define BLE_NOTI1_RESPONSE  "+NOTI=1"BLE_NL""BLE_OK
#define BLE_SLEEP_CMD       "AT+SLEEP"BLE_NL
#define BLE_SLEEP_RESPONSE  "+SLEEP"BLE_NL""BLE_OK
#define BLE_WAKEUP_CMD      "000000000000000000000000000000000000000000000000000000000000000000000000000000000"BLE_NL
#define BLE_WAKEUP_RESPONSE "+WAKE"BLE_NL""BLE_OK
#define BLE_CONN_RESPONSE   "OK+CONN"BLE_NL
#define BLE_LOST_RESPONSE   "OK+LOST"BLE_NL

#define STRLEN(S)           (sizeof(S) - 1)
#define BLE_AWAIT_STRING(S) ble_wait_for_string((S), STRLEN(S))
#define BLE_NL_TRANSMIT(S)  ble_ntransmit((S), MAX_BLE_MSG_LENGTH, false)

#define MAX_BLE_MSG_LENGTH  20
#define RECEIVE_BUFFER_LEN  64


/**
 * Copies 'length' bytes of the BLE receive buffer from current read position
 * into given buffer. Read position is not altered.
 *
 * @param buffer Buffer which will contain the data.
 * @param length Number of characters to be copied.
 * @return The 'buffer' containing 'length' characters of BLE receive buffer from current read position.
 */
char *ble_nbuff(char *buffer, unsigned char length);

/**
 * Copies first 'MAX_BLE_MSG_LENGTH' characters of the BLE receive buffer from current read position
 * into given buffer. Read position is not altered.
 *
 * @param buffer Buffer which will contain the data.
 * @return The 'buffer' containing 'length' characters of BLE receive buffer from current read position.
 */
char *ble_buff(char *buffer);

/**
 * Resets the write and read positions and clears
 * the receiving BLE buffer.
 */
void ble_receive_buffer_reset(void);

/**
 * Initializes the Bluetooth module with name and password.
 */
void ble_init(void);

/**
 * Save delay time after sending a message to prevent
 * corruption of the next message.
 */
void ble_safe_delay(void);

/**
 * Function for sending null-terminated(!) string messages
 * using BLE module with unlimited length.
 *
 * @param string String to be send.
 */
void ble_send_string(char *string);

/**
 * Send string with >80 characters to wake the BLE module up.
 * The module does not need to be sleeping to successfully run this method.
 */
void ble_wake_up(void);

/**
 * Wakes up the BLE module, which must be currently in sleep mode.
 * Waits until external device is connected.
 */
void ble_waiting_wake_up(void);

/**
 * Makes the BLE module go into sleep mode.
 */
void ble_sleep(void);

/**
 * Makes the BLE module go into sleep mode, waits for device disconnection
 * before doing it.
 */
void ble_waiting_sleep(void);


#endif //BLETKA_BLE_UTILS_H
