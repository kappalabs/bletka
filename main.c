
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include <ctype.h>

#include "./asm/defines.S"

#include "c/main.h"
#include "c/interrupts.h"
#include "c/utils.h"
#include "c/ble_utils.h"
#include "c/recmem_utils.h"


void send_record(unsigned int num) {
    //TODO
}

void send_all_records() {
    //TODO
}

void serve() {
    ble_send_string("Serve() START...");
    char text[32];
    char buffer[MAX_BLE_MSG_LENGTH + 1];
    char do_continue = 1;
    unsigned int num = 0;
    unsigned int indx;
    memset(buffer + MAX_BLE_MSG_LENGTH, 0x00, 1);

    while (do_continue) {
        /* Read the command from client */
        memset(buffer, 0x00, MAX_BLE_MSG_LENGTH + 1);
        ble_nreceive(buffer, MAX_BLE_MSG_LENGTH);

        /* Act based on the command ID */
        switch ((unsigned char) buffer[0]) {
            case CMD_NUM_RECS:
                init_recmanager();
                print_recram();
                break;
            case CMD_LOAD_ALL:
                sprintf(text, "Loading all...\r\n");
                ble_send_string(text);
                send_all_records();
                break;
            case CMD_LOAD_N:
                num = 0;
                indx = 1;
                do {
                    num = num * 10 + buffer[indx] - '0';
                } while (indx <= MAX_BLE_MSG_LENGTH && buffer[indx++] != 0x00 && isdigit(buffer[indx]));
                sprintf(text, "Loading n%d...\r\n", num);
                ble_send_string(text);
                send_record(num);
                break;
            case CMD_REMOVE_ALL:
                //TODO
                sprintf(text, "Removing all...\r\n");
                ble_send_string(text);
                break;
            case CMD_REMOVE_N:
                //TODO
                num = 0;
                indx = 1;
                do {
                    num = num * 10 + buffer[indx] - '0';
                } while (indx <= MAX_BLE_MSG_LENGTH && buffer[indx++] != 0x00 && isdigit(buffer[indx]));
                sprintf(text, "Removing n%d...\r\n", num);
                ble_send_string(text);
                break;
            case CMD_SAVE:
//                sprintf(text, "Saving...\r\n");
//                ble_send_string(text);
                strncpy(_timeram_buff, buffer + 1, TIMERAM_LENGTH);
                compress_time();
                save_record(_timeram_buff);
                break;
            case CMD_SET_TIME:
//                sprintf(text, "Setting time...\r\n");
//                ble_send_string(text);
                sprintf(text, "%sREC\r\n", buffer + 1);
                ble_send_string(text);
                rtc_write(TIMERAM_OFFSET, TIMERAM_LENGTH, buffer + 1);
                break;
            case CMD_GET_TIME:
//                sprintf(text, "Getting time...\r\n");
//                ble_send_string(text);
                _update_timestamp();
                ble_ntransmit(_timeram_buff, COMPRESSED_LENGTH);
                delay_100ms();
                break;
            case CMD_SET_LEDMODE:
                //TODO
                sprintf(text, "Setting ledmode...\r\n");
                ble_send_string(text);
                break;
            case CMD_SET_BEEPMODE:
                //TODO
                sprintf(text, "Setting beepmode...\r\n");
                ble_send_string(text);
                break;
            case CMD_EXIT:
                sprintf(text, "Exiting...\r\n");
                ble_send_string(text);
                do_continue = 0;
                break;
            default:
                sprintf(text, "Unknown command!  \r\n?[%s]\r\n", buffer);
                ble_send_string(text);
        }

        led_blink();
    }
    ble_send_string("Serve() STOP...");
}

/**
 * Provides all necessary initializations.
 */
void init(void) {
    /* Library initializations */
    init_library();

    /* Initialization of the bluetooth module */
    ble_init();

    /* Initialization of RECMEM (records memory) */
//    recmem_purge();

    ble_send_string("RESET");
//    ble_send_string("012345678a012345678b012345678c012345678d012345678e012345678fahoj svete");

//    ble_sleep();
    speaker_beep();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void loop(void) {
    for (;;) {
        /* Read the flags */
        if (is_activate_flagged()) {
//            ble_wake_up();
            speaker_ok();
            ble_send_string("activate flag set");
            serve();
//            ble_sleep();
            mint_flags &= ~(1 << MF_A);
        } else if (is_button_flagged()) {
//            ble_wake_up();
            ble_send_string("button flag set");
            _update_timestamp();
            unsigned char evor = mint_flags & MF_B_MSK;
            memset(_timeram_buff + COMPRESSED_LENGTH, evor, 1);
            save_record(_timeram_buff);
            mint_flags &= ~MF_B_MSK;
            speaker_ok();
//            ble_ntransmit(timeram_buff, 7);
//            delay_100ms();
//            ble_sleep();
        } else {
//            ble_wake_up();
            ble_send_string("no flag is set");
//            ble_sleep();
        }

        /* Go back to power-safe mode */
        deep_sleep();
    };
}
#pragma clang diagnostic pop

/**
 * The main entry function for the whole project.
 *
 * @return Exit code, 0 on success.
 */
int main(void) {
    /* Makes all initializations */
    init();

    /* Start the services */
    loop();

    return 0;
}
