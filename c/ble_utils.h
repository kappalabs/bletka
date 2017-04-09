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
#define BLE_WAKEUP_RESPONSE "+WAKE"

#define STRLEN(S)           (sizeof(S) - 1)
#define BLE_AWAIT_STRING(S) ble_wait_for_string((S), STRLEN(S))
#define BLE_NL_TRANSMIT(S)  ble_ntransmit((S), MAX_BLE_MSG_LENGTH, false)

#define MAX_BLE_MSG_LENGTH  20


/**
 * Initializes the Bluetooth module with name and password.
 */
extern void ble_init(void);

/**
 * Function for sending null-terminated(!) string messages
 * using BLE module with unlimited length.
 *
 * @param string String to be send.
 */
extern void ble_send_string(char *string);

/**
 * Wakes up the BLE module by sending long string.
 * Waits until the Android device is connected.
 */
extern void ble_wake_up(void);

/**
 * TODO
 */
extern void ble_waiting_wake_up(void);

/**
 * Makes the BLE module go into sleep mode.
 */
extern void ble_sleep(void);


#endif //BLETKA_BLE_UTILS_H
