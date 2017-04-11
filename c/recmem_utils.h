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

#define RECROM_BYTES        4096
#define RECROM_MAX_RECS     (RECROM_BYTES / RECORD_LENGTH)
#define RECROM_DEL_OFFSET   2
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

#define RECROM_HEADER_OFFSET        0x00
#define RECROM_HEADER_LENGTH        1
#define RECROM_HEADER_SHIFT_MASK    0x03


/**
 * Buffer accessed both from assembly & C code.
 */
volatile char _timeram_buff[TIMERAM_LENGTH];


bool recram_update(unsigned int new_num_recs, unsigned int new_free_slot);
void recram_read(unsigned int *num_recs, unsigned int *free_slot);

void init_recmanager(void);

void print_recram(void);

void print_eeprom(void);

void put_record(unsigned int position, char *record);

void get_record(unsigned int position, char *record);

/**
 * Adds given record to the RECMEM.
 *
 * @param record The record to be saved.
 * @return If the record was saved successfully.
 */
bool save_record(char *record);

/**
 * Makes the records at given position invalid by deleting
 * appropriate bytes.
 *
 * @param position Position of the record to delete.
 */
void destroy_record(unsigned int position);

void print_recram(void);

/**
 * Updates the time information in '_tmp_buff' to contain
 * compressed version of current time.
 */
void _update_timestamp(void);

/**
 * Destroys all records in RECROM and prepares the RECRAM
 * for new set of records.
 */
void recmem_rotate(void);

/**
 * Resets the RECRAM to default settings.
 * Destroys all records in RECROM.
 * Writes magic and initializes the variables to zero.
 */
void recram_reset(void);

/**
 * Initializes the RECMEM (records memory).
 * Should be called only once, to prepare the device for functioning.
 */
void recmem_purge(void);


#endif //BLETKA_RECMEM_UTILS_H
