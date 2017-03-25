#ifndef BLETKA_RECMEM_UTILS_H
#define BLETKA_RECMEM_UTILS_H

/**
 * ####################################################################
 *  UTILITIES FOR RECMEM COMMUNICATION
 * --------------------------------------------------------------------
 *  - RECMEM consists of RECRAM (in RTC registers)
 *    & RECROM (in RTC EEPROM available on I2C)
 * ####################################################################
 */


#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "ble_utils.h"


#define RECORD_LENGTH       5
#define COMPRESSED_LENGTH   4
#define RECORD_DEL_CHAR     0x00
#define RECORD_DEL_LEN      1
#define RECORD_DEL_OFFSET   3

#define TIMERAM_OFFSET      0x00 // within RTC module
#define TIMERAM_LENGTH      7

#define RECRAM_BYTES        4096
#define RECRAM_MAX_RECS     (RECRAM_BYTES / RECORD_LENGTH)
#define RECRAM_OFFSET       0x08 // within RTC module
#define RECRAM_MAGIC        "BTK"
#define MAGIC_LOFFSET       0x00 // within local buffer!
#define MAGIC_OFFSET        RECRAM_OFFSET
#define MAGIC_LENGTH        sizeof(RECRAM_MAGIC) - 1
#define NUMREC_LOFFSET      (MAGIC_LOFFSET + MAGIC_LENGTH)
#define NUMREC_OFFSET       (RECRAM_OFFSET + NUMREC_LOFFSET)
#define NUMREC_LENGTH       2
#define FREESLOT_LOFFSET    (NUMREC_LOFFSET + NUMREC_LENGTH)
#define FREESLOT_OFFSET     (RECRAM_OFFSET + FREESLOT_LOFFSET)
#define FREESLOT_LENGTH     2
#define RECRAM_LENGTH       (MAGIC_LENGTH + NUMREC_LENGTH + FREESLOT_LENGTH)

#define RECROM_HEADER_OFFSET    0X00
#define RECROM_HEADER_LENGTH    1


/**
 * Buffer accessed both from assembly & C code.
 */
char _timeram_buff[TIMERAM_LENGTH];


extern void init_recmanager(void);

extern void print_eeprom(void);

extern void put_record(char *record, unsigned int position);

extern bool save_record(char *record);

extern void remove_record(unsigned int position);

extern void print_recram(void);

/**
 * Updates the time information in '_tmp_buff' to contain
 * compressed version of current time.
 */
extern void _update_timestamp(void);

/**
 * Resets the RECRAM to default settings.
 * Writes magic and initializes the variables to zero.
 */
extern void recram_reset(void);

/**
 * Initializes the RECMEM (records memory).
 * Should be called only once, to prepare the device for functioning.
 */
extern void recmem_purge(void);


#endif //BLETKA_RECMEM_UTILS_H
