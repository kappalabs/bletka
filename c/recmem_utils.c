
#include "recmem_utils.h"


/**
 * Reads the RECROM header information.
 *
 * @return The RECROM header information.
 */
char get_recrom_header(void) {
    char recrom_header_buff[RECROM_HEADER_LENGTH];
    recrom_read(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_header_buff);
    return recrom_header_buff[0];
}

/**
 * Reads the current RECROM shift information.
 *
 * @return The current RECROM shift information.
 */
unsigned char get_recrom_shift(void) {
    return (unsigned char) (get_recrom_header() & RECROM_HEADER_SHIFT_MASK);
}

/**
 * Reads and gets meta-information about RECMEM from RECRAM.
 * Initializes the RECRAM if corrupted/not initialized.
 *
 * @param num_recs Number of current records in the RECMEM.
 * @param free_slot Position of the first available slot for a new record.
 */
void recram_read(unsigned int *num_recs, unsigned int *free_slot) {
    /* Reads the RECRAM */
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    /* Compares the MAGIC, if corrupted, recalculate the metadata */
    if (strncmp(recram_buff, RECRAM_MAGIC, MAGIC_LENGTH) == 0) {
        /* Decode the metadata from RTC registers (2B H:L) */
        *num_recs = (unsigned int) (
                ((unsigned char) recram_buff[NUMREC_LOFFSET] << 8)
                | (unsigned char) recram_buff[NUMREC_LOFFSET + 1]);
        *free_slot = (unsigned int) (
                ((unsigned char) recram_buff[FREESLOT_LOFFSET] << 8)
                | (unsigned char) recram_buff[FREESLOT_LOFFSET + 1]);
    } else {
        /* Write the magic */
        memcpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
        rtc_write(MAGIC_OFFSET, MAGIC_LENGTH, recram_buff);

        /* Go throught the RECROM and initialize the metadata */
        char recrom_buff[1];
        unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH + RECROM_DEL_OFFSET);
        unsigned int i = offset;
        *num_recs = 0;
        *free_slot = 0x00;
        while (i < RECROM_BYTES) {
            recrom_read(i, 1, recrom_buff);
            if (recram_buff[0] != RECORD_DEL_CHAR) {
                //TODO: případně závorky
                ++*num_recs;
                *free_slot = i + RECORD_LENGTH - RECROM_DEL_OFFSET;
            }
            i += RECORD_LENGTH;
        }

        /* Write the new values into the RECRAM */
        recram_update(*num_recs, *free_slot);
    }
}

/**
 * Sets metadata in RECRAM to given values.
 *
 * @param new_num_recs New number of records.
 * @param new_free_slot New position of the first free slot.
 * @return If the update was successful.
 */
bool recram_update(unsigned int new_num_recs, unsigned int new_free_slot) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    /* Check the free/available space for records */
    if (new_num_recs > RECROM_MAX_RECS) {
        //TODO: report nutnost rotace
        speaker_err();
        return false;
    }

    /* Fix the magic if needed */
    if (strncmp(recram_buff, RECRAM_MAGIC, MAGIC_LENGTH) != 0) {
        memcpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
        rtc_write(MAGIC_OFFSET, MAGIC_LENGTH, recram_buff + MAGIC_LOFFSET);
    }

    /* Update metadata in RECRAM */
    recram_buff[NUMREC_LOFFSET] = (char) ((new_num_recs >> 8) & 0xFF);
    recram_buff[NUMREC_LOFFSET + 1] = (char) (new_num_recs & 0xFF);
    recram_buff[FREESLOT_LOFFSET] = (char) ((new_free_slot >> 8) & 0xFF);
    recram_buff[FREESLOT_LOFFSET + 1] = (char) (new_free_slot & 0xFF);
    rtc_write(NUMREC_OFFSET, NUMREC_LENGTH, recram_buff + NUMREC_LOFFSET);
    rtc_write(FREESLOT_OFFSET, FREESLOT_LENGTH, recram_buff + FREESLOT_LOFFSET);

    return true;
}

void init_recmanager(void) {
    unsigned int _x;
    recram_read(&_x, &_x);
}

void print_eeprom(void) {
    char eeprom_buff[MAX_BLE_MSG_LENGTH];
    int i;
    unsigned int position = 0x00;
    ble_send_string("RECROM bytes:\r\n");
//    for (i = 0; i < RECROM_BYTES / MAX_BLE_MSG_LENGTH; ++i) {
    for (i = 0; i < 5; ++i) {
        recrom_read(position, MAX_BLE_MSG_LENGTH, eeprom_buff);
        ble_ntransmit(eeprom_buff, MAX_BLE_MSG_LENGTH, true);
        position += MAX_BLE_MSG_LENGTH;
        delay_100ms();
    }
}

void get_record(unsigned int position, char *record) {
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH);

    recrom_read(position * RECORD_LENGTH + offset, RECORD_LENGTH, record);
}

void put_record(unsigned int position, char *record) {
    /* Find the address to write to */
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH);

    /* Make the write to RECROM */
    recrom_write(RECORD_LENGTH * position + offset, RECORD_LENGTH, record);
}

bool save_record(char *record) {
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);

    /* Does the RECROM still have free space? */
    bool err = false;
    if (free_slot < RECROM_MAX_RECS) {
        /* Write the record into memory */
        put_record(free_slot, record);

        /* Update metadata in RECRAM */
        if (!recram_update(++num_recs, ++free_slot)) {
            err = true;
        }
    } else {
        err = true;
    }

    //TODO: přesunout
    if (err) {
        //TODO: error, nedostatek místa!
        speaker_err();
    }

    return !err;
}

void destroy_record(unsigned int position) {
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH + RECORD_DEL_OFFSET);
    char record[RECORD_DEL_LEN];
    memset(record, RECORD_DEL_CHAR, RECORD_DEL_LEN);

    //TODO: potřeba?
    /* Check RECMEM consistency */
    init_recmanager();

    //TODO: není potřeba mazat na pozici, která bude třetí při další rotaci?
    /* Delete the record from RECROM */
    recrom_write(RECORD_LENGTH * position + offset, RECORD_DEL_LEN, record);
    speaker_beep();

    /* Increase the index of the first available position */
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    print_eeprom();
}

void print_recram(void) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
    unsigned int num_recs = (unsigned int) (
            ((unsigned char) recram_buff[NUMREC_LOFFSET] << 8)
            | (unsigned char) recram_buff[NUMREC_LOFFSET + 1]);
    unsigned int free_slot = (unsigned int) (
            ((unsigned char) recram_buff[FREESLOT_LOFFSET] << 8)
            | (unsigned char) recram_buff[FREESLOT_LOFFSET + 1]);

    ble_ntransmit(recram_buff, RECRAM_LENGTH, true);
    delay_100ms();
    delay_100ms();
    char text[32];
    sprintf(text, "NR=%u, FS=%u\r\n", num_recs, free_slot);
    ble_send_string(text);
}

void _update_timestamp(void) {
    char rtc_buff[TIMERAM_LENGTH];
    memset(rtc_buff, 0x00, TIMERAM_LENGTH);
    rtc_read(TIMERAM_OFFSET, TIMERAM_LENGTH, rtc_buff);
    memcpy(_timeram_buff, rtc_buff, TIMERAM_LENGTH);
    compress_time();
}

void recram_reset(void) {
    char recram_buff[RECRAM_LENGTH];
    memcpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
    memset(recram_buff + NUMREC_LOFFSET, 0x00, NUMREC_LENGTH);
    memset(recram_buff + FREESLOT_LOFFSET, 0x00, FREESLOT_LENGTH);
    rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
}

void recmem_rotate(void) {
    /* Read current record offset */
    char recrom_header_buff[RECROM_HEADER_LENGTH];
    recrom_read(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_header_buff);
    char header = recrom_header_buff[0];
    unsigned char offset = (unsigned char) (header & RECROM_HEADER_SHIFT_MASK);

    /* Next RECMEM write will start with new offset */
    offset = (unsigned char) ((offset + 1) & RECROM_HEADER_SHIFT_MASK);
    recrom_header_buff[0] = offset;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_header_buff);
//    delay_10ms();

    /* Set 3rd byte of every record to zero */
    char recrom_buff_[RECORD_DEL_LEN];
    memset(recrom_buff_, RECORD_DEL_CHAR, RECORD_DEL_LEN);
    unsigned int i = offset + RECROM_HEADER_LENGTH + RECROM_DEL_OFFSET;
    while (i < RECROM_BYTES) {
        recrom_write(i, RECORD_DEL_LEN, recrom_buff_);
//        delay_10ms();
        i += RECORD_LENGTH;
    }

    //TODO: pouze debug
    print_recram();
    print_eeprom();
}

void recmem_reset(void) {
    /* Set 3rd byte of every record to zero */
    char recrom_buff_[1];
    recrom_buff_[0] = 0x00;
    unsigned int i = RECROM_HEADER_LENGTH + RECROM_DEL_OFFSET;
    while (i < RECROM_BYTES) {
        recrom_write(i, 1, recrom_buff_);
//        delay_10ms();
        i += RECORD_LENGTH;
    }

    /* Next RECMEM write will start with default shift 0 */
    char recrom_buff[RECROM_HEADER_LENGTH];
    recrom_buff[0] = 0x00;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);

    /* Reset the pointers position to default */
    recram_reset();

    //TODO: pouze debug
    print_recram();
    print_eeprom();
}

#define DELETE_BUFF_LEN 32U // The RECROM is capable of max 32-byte page writes

void recmem_purge(void) {
    /* Set the whole RECROM to zeros */
    char recrom_buff[DELETE_BUFF_LEN];
    memset(recrom_buff, 0x00, DELETE_BUFF_LEN);
    unsigned int i = 0;
    while (i < RECROM_BYTES) {
        recrom_write(i, DELETE_BUFF_LEN, recrom_buff);
//        delay_10ms();
        i += DELETE_BUFF_LEN;
    }

    /* Reset the pointers position to default */
    recram_reset();

    //TODO: pouze debug
    print_recram();
    print_eeprom();
}
