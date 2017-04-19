
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "./asm/defines.S"

#include "c/main.h"
#include "c/interrupts.h"
#include "c/utils.h"
#include "c/ble_utils.h"
#include "c/recmem_utils.h"


void send_record(unsigned int num) {
    char record_buff[RECORD_LENGTH];
    get_record(num, record_buff);
    ble_ntransmit(record_buff, RECORD_LENGTH, true);
    ble_safe_delay();
}

void send_all_records() {
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);
    char record_buff[RECORD_LENGTH];
    unsigned int i;

    /* Function get_record() returns all (including invalid) records,
     * therefore 'free_slot' is used instead of 'num_recs' */
    for (i = 0; i < free_slot; ++i) {
        get_record(i, record_buff);
        ble_ntransmit(record_buff, RECORD_LENGTH, true);
        ble_safe_delay();
    }
}

void save_current_time(void) {
    _update_timestamp();
    unsigned char evor = mint_flags & MF_B_MSK;
    *(_timeram_buff + COMPRESSED_LENGTH) = evor;
    if (!save_record((char *) _timeram_buff)) {
        speaker_err();
    } else {
        speaker_ok();
    }
}

/**
 * Starts to serve the external device.
 */
void serve() {
    ble_send_string("Serve() START...");
    char text[32];
    bool do_continue = true;
    bool disconnected = false;
    char buffer[MAX_BLE_MSG_LENGTH + 1];
    *(buffer + MAX_BLE_MSG_LENGTH) = 0x00;
    unsigned int num = 0;

    while (do_continue) {
        /* Read the command from client */
        ble_receive_buffer_reset();
        ble_allow_receive();
        ble_nreceive(MAX_BLE_MSG_LENGTH, false);
        ble_disallow_receive();

        /* Act based on the command ID */
        switch ((unsigned char) ble_buff(buffer)[0]) {
            case CMD_NUM_RECS:
                sprintf(text, "Getting #recs...\r\n");
                ble_send_string(text);
                print_recram();
                break;
            case CMD_LOAD_ALL:
                sprintf(text, "Loading all...\r\n");
                ble_send_string(text);
                send_all_records();
                break;
            case CMD_LOAD_N:
                if (!parse_number(ble_buff(buffer) + 1, &num)) {
                    break;
                }
                sprintf(text, "Loading n%d...\r\n", num);
                ble_send_string(text);
                send_record(num);
                break;
            case CMD_REMOVE_ALL:
                sprintf(text, "Removing all...\r\n");
                ble_send_string(text);
                recmem_rotate();
                break;
            case CMD_REMOVE_N:
                if (!parse_number(ble_buff(buffer) + 1, &num)) {
                    break;
                }
                sprintf(text, "Removing n%d...\r\n", num);
                ble_send_string(text);
                destroy_record(num);
                break;
            case CMD_SAVE:
                sprintf(text, "Saving...\r\n");
                ble_send_string(text);
                memcpy((void *) _timeram_buff, ble_buff(buffer) + 1, TIMERAM_LENGTH);
                compress_time();
                /* Copy the EVOR */
                _timeram_buff[COMPRESSED_LENGTH] = ble_buff(buffer)[8];
                if (!save_record((char *) _timeram_buff)) {
                    speaker_err();
                } else {
                    speaker_ok();
                }
                break;
            case CMD_SET_TIME:
                sprintf(text, "Setting time...\r\n");
                ble_send_string(text);
                sprintf(text, "%sREC\r\n", ble_buff(buffer) + 1);
                ble_send_string(text);
                rtc_write(TIMERAM_OFFSET, TIMERAM_LENGTH, ble_buff(buffer) + 1);
                break;
            case CMD_GET_TIME:
                sprintf(text, "Getting time...\r\n");
                ble_send_string(text);
                _update_timestamp();
                ble_ntransmit((char *) _timeram_buff, COMPRESSED_LENGTH, true);
                ble_safe_delay();
                break;
            case CMD_SET_LEDMODE:
                //TODO
                sprintf(text, "Setting led...\r\n");
                ble_send_string(text);
                break;
            case CMD_SET_BEEPMODE:
                //TODO
                sprintf(text, "Setting beep...\r\n");
                ble_send_string(text);
                break;
            case CMD_EXIT:
                sprintf(text, "Exiting...\r\n");
                ble_send_string(text);
                do_continue = false;
                break;
            case CMD_RECMEM_PURGE:
                sprintf(text, "Purging recmem...\r\n");
                ble_send_string(text);
                recmem_purge();
                break;
            case CMD_RECMEM_PRINT:
                sprintf(text, "Printing recmem...\r\n");
                ble_send_string(text);
                print_recram();
                print_recrom();
                break;
            default:
                /* External device disconnected itself? */
                if (streq(ble_buff(buffer), BLE_LOST_RESPONSE)) {
                    do_continue = false;
                    disconnected = true;
                    break;
                }

                /* Notify the device about the wrong command */
                sprintf(text, "Unknown command!\r\n");
                ble_send_string(text);
                sprintf(text, "?[%s]\r\n", ble_buff(buffer));
                ble_ntransmit(text, MAX_BLE_MSG_LENGTH, true);
                ble_safe_delay();
        }

        speaker_beep();
    }
    ble_send_string("Serve() STOP...");

    /* Turn BLE to sleep mode based on external device's connection */
    ble_allow_receive();
    if (disconnected) {
        ble_sleep();
    } else {
        ble_waiting_sleep();
    }
}

/**
 * Provides all necessary initializations.
 */
void init(void) {
    /* Library initializations */
    init_library();

    /* Consistency checks of RECMEM */
    init_recmanager();

    /* Initialization of the bluetooth module */
    ble_init();

    /* Initialization of RECMEM (records memory) */
//    recmem_purge();

    ble_sleep();
    speaker_beep();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void loop(void) {
    for (;;) {
        /* Read the flags */
        if (is_activate_flagged()) {
            /* Wake up the BLE module and wait for device connection */
            ble_waiting_wake_up();
            delay_1s();

            speaker_ok();
            ble_send_string("activate flag set");
            serve();

            mint_flags &= 0x00;
        } else if (is_button_flagged()) {
            save_current_time();
            mint_flags &= ~MF_B_MSK;
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
    /* Does all initializations */
    init();

    /* Start the services */
    loop();

    return 0;
}
