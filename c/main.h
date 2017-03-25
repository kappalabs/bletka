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

#define MIN(X, Y)           (((X) < (Y)) ? (X) : (Y))


/* delays.S */
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
extern void rtc_read(unsigned char position, unsigned char length, char *buffer);
extern void rtc_write(unsigned char position, unsigned char length, char *buffer);
extern void recrom_read(unsigned int position, unsigned char length, char *buffer);
extern void recrom_write(unsigned int position, unsigned char length, char *buffer);

/* usart_peripherals.S */
extern void ble_nreceive(char *buffer, unsigned char length);
extern void ble_ntransmit(char *buffer, unsigned char length);

/* utils.S */
/**
 * Makes the whole device go into sleep mode with minimum
 * power demands. Only button press can wake it back up.
 */
extern void deep_sleep(void);
extern void led_on(void);
extern void led_off(void);
extern void speaker_beep(void);
extern void compress_time(void);
extern char is_activate_flagged(void);
extern char is_button_flagged(void);


/**
 * Variable shared between assembly & C code containing custom interrupt flags.
 */
volatile unsigned char mint_flags;


#endif //BLETKA_MAIN_H
