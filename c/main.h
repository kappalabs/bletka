#ifndef BLETKA_MAIN_H
#define BLETKA_MAIN_H

/**
 * ####################################################################
 *  MAIN PROGRAM DEFINITIONS
 * --------------------------------------------------------------------
 *  - general definitions
 *  - external methods provided by assembly library
 * ####################################################################
 */


#include <stdbool.h>


/*
 * Defines of command codes. Must be unique for each one of them.
 * External (Android) software must use the same codes for the communication to work.
 */
/**
 * Command for getting number of available records in RECMEM.
 * | CODE | 0x0A |
 */
#define CMD_NUM_RECS        0x80
/**
 * Command for sending all available records from RECMEM.
 * | CODE | 0x0A |
 */
#define CMD_LOAD_ALL        0x81
/**
 * Command for sending i-th record from RECMEM.
 * | CODE | n_1 | n_2 | n_3 | ... | n_k | 0x0A |
 * where 'k' belongs to [1;6], n_1 n_2 ... n_k represents number in decimal
 * system (i.e. if n_1='4' and n_2='2' for k=2, then 42-th record will be send).
 */
#define CMD_LOAD_N          0x82
#define CMD_REMOVE_ALL      0x83
#define CMD_REMOVE_N        0x84
#define CMD_SAVE            0x85
#define CMD_SET_TIME        0x86
#define CMD_GET_TIME        0x87
#define CMD_SET_LEDMODE     0x88
#define CMD_SET_BEEPMODE    0x89
#define CMD_EXIT            0x8A
#define CMD_OK              0x8B
#define CMD_ERR             0x8C
#define CMD_RECMEM_PURGE    0x8D
#define CMD_RECMEM_PRINT    0x8E

#define MIN(X, Y)           (((X) < (Y)) ? (X) : (Y))

//#define DEBUG


/* delays.S */
extern void delay_10ms(void);
extern void delay_20ms(void);
extern void delay_50ms(void);
extern void delay_100ms(void);
extern void delay_1s(void);
extern void delay_500ms(void);


/* inits.S */
/**
 * Makes all initializations needed for this device.
 */
extern void init_library(void);


/* twi_peripherals.S */
/* Real Time Clock functions */
/**
 * Reads and stores registers of real time clock module from given
 * 'position' of given 'length' into given 'buffer'.
 *
 * @param position Position of the first register to read from.
 * @param length Number of characters to be read.
 * @param buffer Buffer where the data will be stored.
 */
extern void rtc_read(unsigned char position, unsigned char length, char *buffer);
/**
 * Writes given 'buffer' of given 'length' characters into RTC module
 * beginning at register 'position'.
 *
 * @param position Address of the first register to write to.
 * @param length Number of characters to be written.
 * @param buffer Data to be written.
 */
extern void rtc_write(unsigned char position, unsigned char length, char *buffer);
/**
 * Reads 'length' characters into 'buffer' from given 'position'
 * in RTC EEPROM memory.
 *
 * @param position Position to read from.
 * @param length Number of characters to read.
 * @param buffer Buffer for the received data.
 */
extern void recrom_read(unsigned int position, unsigned char length, char *buffer);
/**
 * Writes 'length' characters from given 'buffer' starting at 'position'
 * into RTC module EEPROM page. Page is 32B long, after reaching the end
 * of it it rotates back to the first byte in the page, therefore using
 * 'length' > 32 makes no sense.
 *
 * @param position Position where the write will start.
 * @param length Number of characters which will be written.
 * @param buffer Buffer containing the data to be written.
 */
extern void recrom_page_write(unsigned int position, unsigned char length, char *buffer);

/* usart_peripherals.S */
/**
 * Transmits bytes through BLE module until it receives 0x0A='\n'
 * or limit for number of characters 'length' is reached.
 *
 * @param buffer Buffer with data to be transmitted.
 * @param length Maximum number of characters to be transmitted.
 * @param strict If true, exactly 'length' characters will be transmitted, '\n' character is therefore ignored in that case.
 */
extern void ble_ntransmit(char *buffer, unsigned char length, bool strict);
/**
 * Reads characters transmitted by BLE module and stores them into '_usart_rb'
 * buffer until 'length' characters is read, or 'strict' is false
 * and 0x0A='\n' character was received.
 *
 * @param length Maximum number of characters to be received.
 * @param strict If true, exactly 'length' characters will be read, '\n' character is therefore ignored.
 */
extern void ble_nreceive(unsigned char length, bool strict);
/**
 * Allows interrupt-driven receive of characters from USART into '_usart_rb'.
 */
extern void ble_allow_receive(void);
/**
 * Disallows interrupt-driven receive of characters from USART into '_usart_rb'.
 */
extern void ble_disallow_receive(void);


/* utils.S */
/**
 * Makes the whole device go into sleep mode with minimum
 * power demands. Only button press can wake it back up.
 */
extern void deep_sleep(void);
/**
 * Turns the LED on.
 */
extern void led_on(void);
/**
 * Turns the LED off.
 */
extern void led_off(void);
/**
 * Makes a beep sound.
 */
extern void speaker_beep(void);
/**
 * Checks if the Activate flag is set.
 *
 * @return Returns 1 if set, 0 if cleared.
 */
extern char is_activate_flagged(void);
/**
 * Checks if the Activate flag is set.
 *
 * @return Returns 1 if set, 0 if cleared.
 */
extern char is_button_flagged(void);
/**
 * Makes a compression of 7B in '_timeram_buff' retrieved from
 * RTC module into 4B.
 */
extern void compress_time(void);


/**
 * Variable shared between assembly & C code containing custom interrupt flags.
 */
volatile unsigned char mint_flags;


#endif //BLETKA_MAIN_H
