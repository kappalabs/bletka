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
 * Makes the BLE module go into sleep mode.
 */
extern void ble_sleep(void);


#endif //BLETKA_BLE_UTILS_H
