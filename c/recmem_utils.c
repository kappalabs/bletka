
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
        /* Go throught the RECROM and initialize the metadata */
        char recrom_buff[2];
        unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH + RECORD_ROTATE_DEL_OFFSET);
        unsigned int i = offset;
        unsigned int iters = 0;
        *num_recs = 0;
        *free_slot = 0;
        while (i < RECROM_BYTES) {
            recrom_read(i, 2, recrom_buff);
            if (recrom_buff[0] != RECORD_DEL_CHAR && recrom_buff[1] != RECORD_DEL_CHAR) {
                ++*num_recs;
                *free_slot = iters + 1;
            }
            i += RECORD_LENGTH;
            ++iters;
        }

        /* Write the new values into the RECRAM */
        if (recram_update(*num_recs, *free_slot)) {
            /* Write the magic */
            memcpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
            rtc_write(MAGIC_OFFSET, MAGIC_LENGTH, recram_buff);
        }

        speaker_err();
    }
}

bool recram_update(unsigned int new_num_recs, unsigned int new_free_slot) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    /* Check the free/available space for records */
    if (new_num_recs > RECROM_MAX_RECS) {
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

void recrom_write(unsigned int position, unsigned int length, char *buffer) {
    unsigned char part_len;
    while (length > 0) {
        /* Calculate the length in current page */
        part_len = (unsigned char) (RECROM_PAGE_LENGTH - (position & RECROM_PAGE_MASK));
        if (length < RECROM_PAGE_LENGTH) {
            part_len = MIN(part_len, (unsigned char) length);
        }
        recrom_page_write(position, part_len, buffer);
        position += part_len;
        buffer += part_len;
        length -= part_len;
    }
}

void print_recrom(void) {
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

void print_recram(void) {
    /* Transmit the whole RECRAM content */
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
    ble_ntransmit(recram_buff, RECRAM_LENGTH, true);
    ble_safe_delay();

    /* Transmit the decoded NR and FS values */
    char text[32];
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);
    sprintf(text, "NR=%u, FS=%u\r\n", num_recs, free_slot);
    ble_send_string(text);
}

void get_record(unsigned int position, char *record) {
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH);

    /* Read the record from RECROM */
    recrom_read(position * RECORD_LENGTH + offset, RECORD_LENGTH, record);
}

void put_record(unsigned int position, char *record) {
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH);

    /* Make the write to RECROM */
    recrom_write(RECORD_LENGTH * position + offset, RECORD_LENGTH, record);
}

bool save_record(char *record) {
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);

    /* Does the RECROM still have free space? */
    if (free_slot < RECROM_MAX_RECS) {
        /* Write the record into memory */
        put_record(free_slot, record);

        /* Update metadata in RECRAM */
        return recram_update(++num_recs, ++free_slot);
    }
    return false;
}

void destroy_record(unsigned int position) {
    unsigned char offset = (unsigned char) (get_recrom_shift() + RECROM_HEADER_LENGTH);
    char recrom_buff[2];
    memset(recrom_buff, RECORD_DEL_CHAR, 2);

    /* Don't destroy the record if it's already invalid */
    recrom_read(RECORD_LENGTH * position + offset + RECORD_ROTATE_DEL_OFFSET, 2, recrom_buff);
    if (recrom_buff[0] == RECORD_DEL_CHAR || recrom_buff[1] == RECORD_DEL_CHAR) {
        return;
    }

    /* Destroy the record by writing DEL character in proper byte */
    recrom_buff[0] = RECORD_DEL_CHAR;
    recrom_write(RECORD_LENGTH * position + offset + RECORD_DEL_OFFSET, RECORD_DEL_LEN, recrom_buff);

    /* Read the current metadata */
    unsigned int num_recs, free_slot;
    recram_read(&num_recs, &free_slot);

    /* Decrease the metadata about number of records in RECMEM */
    recram_update(num_recs - 1, free_slot);
}

void _update_timestamp(void) {
    char rtc_buff[TIMERAM_LENGTH];
    memset(rtc_buff, 0x00, TIMERAM_LENGTH);
    rtc_read(TIMERAM_OFFSET, TIMERAM_LENGTH, rtc_buff);
    memcpy((void *) _timeram_buff, rtc_buff, TIMERAM_LENGTH);
    compress_time();
}

/**
 * Resets the RECRAM to default settings.
 * Writes magic and initializes the variables to zero.
 */
void recram_reset(void) {
    char recram_buff[RECRAM_LENGTH];
    memcpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
    memset(recram_buff + NUMREC_LOFFSET, 0x00, NUMREC_LENGTH);
    memset(recram_buff + FREESLOT_LOFFSET, 0x00, FREESLOT_LENGTH);
    rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
}

/**
 * Destroy every record by writing DEL character into proper bytes.
 *
 * @param shift Header shift which should be used.
 */
void destroy_all_records(unsigned char shift) {
    char recrom_buff[2];
    memset(recrom_buff, RECORD_DEL_CHAR, 2);
    unsigned int i = shift + RECROM_HEADER_LENGTH + RECORD_ROTATE_DEL_OFFSET;
    while (i < RECROM_BYTES) {
        /* Don't destroy the record if it's already invalid */
        recrom_read(i, 2, recrom_buff);
        if (recrom_buff[0] != RECORD_DEL_CHAR && recrom_buff[1] != RECORD_DEL_CHAR) {
            recrom_buff[0] = RECORD_DEL_CHAR;
            recrom_write(i, RECORD_DEL_LEN, recrom_buff);
        }

        i += RECORD_LENGTH;
    }
}

void recmem_rotate(void) {
    /* Read current record offset */
    char recrom_header_buff[RECROM_HEADER_LENGTH];
    recrom_read(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_header_buff);
    char header = recrom_header_buff[0];
    unsigned char offset = (unsigned char) (header & RECROM_HEADER_SHIFT_MASK);

    /* Next RECMEM write will start with new offset */
    offset = (unsigned char) (((offset + 1) & RECROM_HEADER_SHIFT_MASK) % RECORD_LENGTH);
    recrom_header_buff[0] = offset;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_header_buff);

    /* Destroy every record in RECROM */
    destroy_all_records(offset);

    /* Reset the pointers position to default */
    recram_reset();
}

void recmem_reset(void) {
    /* Next RECMEM write will start with default shift 0 */
    char recrom_buff[RECROM_HEADER_LENGTH];
    recrom_buff[0] = 0x00;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);

    /* Destroy every record in RECROM */
    destroy_all_records(0);

    /* Reset the pointers position to default */
    recram_reset();
}

void recmem_purge(void) {
    /* Set the whole RECROM to zeros */
    char recrom_buff[RECROM_PAGE_LENGTH];
    memset(recrom_buff, RECORD_DEL_CHAR, RECROM_PAGE_LENGTH);
    unsigned int i = 0;
    while (i < RECROM_BYTES) {
        recrom_write(i, RECROM_PAGE_LENGTH, recrom_buff);
        i += RECROM_PAGE_LENGTH;
    }

    /* Reset the pointers position to default */
    recram_reset();
}

void recram_purge(void) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0xFF, RECRAM_LENGTH);
    rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
}
