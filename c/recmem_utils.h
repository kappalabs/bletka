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


#define RECORD_LENGTH               5
#define COMPRESSED_LENGTH           4
#define RECORD_DEL_CHAR             0x00
#define RECORD_DEL_LEN              1
#define RECORD_DEL_OFFSET           3
#define RECORD_ROTATE_DEL_OFFSET    2

#define TIMERAM_OFFSET              0x00 // within RTC module
#define TIMERAM_LENGTH              7

#define RECROM_HEADER_OFFSET        0x00
#define RECROM_HEADER_LENGTH        1
#define RECROM_HEADER_SHIFT_MASK    0x07
#define RECROM_PAGE_LENGTH          32
#define RECROM_PAGE_MASK            0x1F

#define RECROM_BYTES                4096
#define RECROM_MAX_RECS             ((RECROM_BYTES - RECROM_HEADER_LENGTH) / RECORD_LENGTH)
#define RECRAM_OFFSET               0x08 // within RTC module
#define RECRAM_MAGIC                "BTK"
#define MAGIC_LOFFSET               0x00 // within local buffer!
#define MAGIC_OFFSET                RECRAM_OFFSET
#define MAGIC_LENGTH                sizeof(RECRAM_MAGIC) - 1
#define NUMREC_LOFFSET              (MAGIC_LOFFSET + MAGIC_LENGTH)
#define NUMREC_OFFSET               (RECRAM_OFFSET + NUMREC_LOFFSET)
#define NUMREC_LENGTH               2
#define FREESLOT_LOFFSET            (NUMREC_LOFFSET + NUMREC_LENGTH)
#define FREESLOT_OFFSET             (RECRAM_OFFSET + FREESLOT_LOFFSET)
#define FREESLOT_LENGTH             2
#define RECRAM_LENGTH               (MAGIC_LENGTH + NUMREC_LENGTH + FREESLOT_LENGTH)


/**
 * Buffer accessed both from assembly & C code.
 */
volatile char _timeram_buff[TIMERAM_LENGTH];


/**
 * Reads and gets meta-information about RECMEM from RECRAM.
 * Initializes the RECRAM if corrupted/not initialized.
 *
 * @param num_recs Number of current records in the RECMEM.
 * @param free_slot Position of the first available slot for a new record.
 */
void recram_read(unsigned int *num_recs, unsigned int *free_slot);

/**
 * Sets metadata in RECRAM to given values.
 *
 * @param new_num_recs New number of records.
 * @param new_free_slot New position of the first free slot.
 * @return If the update was successful.
 */
bool recram_update(unsigned int new_num_recs, unsigned int new_free_slot);

/**
 * Checks consistency of RECMEM, fixes it if corrupted.
 */
void init_recmanager(void);

/**
 * Writes given data to RECROM memory. Can handle data longer than page size.
 *
 * @param position Position in RECROM to start writing to.
 * @param length Number of characters to be written.
 * @param buffer Buffer containing data to be written.
 */
void recrom_write(unsigned int position, unsigned int length, char *buffer);

/**
 * Prints (sends message over BLE) first 5*20 bytes of the RECROM content.
 */
void print_recrom(void);

/**
 * Prints (sends message over BLE) content of RECRAM and its interpretation
 * in human-readable form.
 */
void print_recram(void);

/**
 * Gets 'position'-th record from the RECMEM. The record can be invalid
 * if it was previously removed/destroyed.
 *
 * @param position Represents order of record creation.
 * @param record Buffer to contain resulting record.
 */
void get_record(unsigned int position, char *record);

/**
 * Puts given 'position'-th record to the RECMEM.
 *
 * @param position Represents order of record creation.
 * @param record Buffer containing the record.
 */
void put_record(unsigned int position, char *record);

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
 * @param position Position (order) of the record to delete.
 */
void destroy_record(unsigned int position);

/**
 * Updates the time information in '_tmp_buff' to contain
 * compressed version of current time.
 */
void _update_timestamp(void);

/**
 * Destroys all records in RECROM and prepares the RECRAM
 * for a new set of records.
 */
void recmem_rotate(void);

/**
 * Destroys all records in RECMEM and initializes default settings.
 */
void recmem_reset(void);

/**
 * Deletes the whole RECMEM (records memory).
 * Should be called only once, to prepare the device for functioning.
 */
void recmem_purge(void);

/**
 * Writes 0xFF to whole RECRAM to make it invalid.
 */
void recram_purge(void);


#endif //BLETKA_RECMEM_UTILS_H
