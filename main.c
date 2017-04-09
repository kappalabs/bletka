
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
    delay_100ms();
}

void send_all_records() {
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);
    char record_buff[RECORD_LENGTH];
    unsigned int i;
    for (i = 0; i < num_recs; ++i) {
        get_record(i, record_buff);
        ble_ntransmit(record_buff, RECORD_LENGTH, true);
        delay_100ms();
    }
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
        ble_nreceive(buffer, MAX_BLE_MSG_LENGTH, false);

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
                num = 0; indx = 1;
                do {
                    num = num * 10 + buffer[indx] - '0';
                } while (indx <= MAX_BLE_MSG_LENGTH && buffer[indx++] != 0x00 && isdigit(buffer[indx]));
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
                num = 0; indx = 1;
                do {
                    num = num * 10 + buffer[indx] - '0';
                } while (indx <= MAX_BLE_MSG_LENGTH && buffer[indx++] != 0x00 && isdigit(buffer[indx]));
                sprintf(text, "Removing n%d...\r\n", num);
                ble_send_string(text);
                destroy_record(num);
                break;
            case CMD_SAVE:
                sprintf(text, "Saving...\r\n");
                ble_send_string(text);
                memcpy(_timeram_buff, buffer + 1, TIMERAM_LENGTH);
                compress_time();
                /* Copy the EVOR */
                _timeram_buff[COMPRESSED_LENGTH] = buffer[8];
                save_record(_timeram_buff);
                break;
            case CMD_SET_TIME:
                sprintf(text, "Setting time...\r\n");
                ble_send_string(text);
                sprintf(text, "%sREC\r\n", buffer + 1);
                ble_send_string(text);
                rtc_write(TIMERAM_OFFSET, TIMERAM_LENGTH, buffer + 1);
                break;
            case CMD_GET_TIME:
//                sprintf(text, "Getting time...\r\n");
//                ble_send_string(text);
                _update_timestamp();
                ble_ntransmit(_timeram_buff, COMPRESSED_LENGTH, true);
                delay_100ms();
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
                do_continue = 0;
                break;
            case CMD_RECMEM_PURGE:
                sprintf(text, "Purging recmem...\r\n");
                ble_send_string(text);
                recmem_purge();
                break;
            default:
                //TODO: toto neposílat, může být příliš dlouhé (>20 znaků)
                sprintf(text, "Unknown command!\r\n");
                ble_send_string(text);
                sprintf(text, "?[%s]\r\n", buffer);
                ble_send_string(text);
        }

//        led_blink();
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

    ble_send_string("RESET\r\n");

    ble_sleep();
    speaker_beep();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void loop(void) {
    for (;;) {

        /* Read the flags */
        if (is_activate_flagged()) {
            //TODO: přehodnotit
            ble_waiting_wake_up();
            speaker_ok();
            ble_send_string("activate flag set");
            serve();
            //TODO: přehodnotit
            ble_sleep();
            mint_flags &= ~(1 << MF_A);
        } else if (is_button_flagged()) {
            //TODO: odstranit
            ble_waiting_wake_up();
            ble_send_string("button flag set");

            _update_timestamp();
            unsigned char evor = mint_flags & MF_B_MSK;
            memset(_timeram_buff + COMPRESSED_LENGTH, evor, 1);
            save_record(_timeram_buff);
            mint_flags &= ~MF_B_MSK;
            speaker_ok();
//            ble_ntransmit(timeram_buff, 7);
//            delay_100ms();

            //TODO: odstranit
            ble_sleep();
        } else {
//            ble_send_string("no flag is set");
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
