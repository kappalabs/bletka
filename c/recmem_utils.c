
#include "recmem_utils.h"


void init_recmanager(void) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    if (strncmp(recram_buff, RECRAM_MAGIC, MAGIC_LENGTH) == 0) {
        //TODO: ok
        return;
    } else {
        strncpy(recram_buff, RECRAM_MAGIC, MAGIC_LENGTH);
        //TODO: spočítat průchodem RECROM
        memset(recram_buff + NUMREC_LOFFSET, 0x00, NUMREC_LENGTH);
        memset(recram_buff + FREESLOT_LOFFSET, 0x00, FREESLOT_LENGTH);
        rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
    }
}

void print_eeprom(void) {
    char eeprom_buff[MAX_BLE_MSG_LENGTH];
    int i;
    unsigned int position = 0x00;
    ble_send_string("RECROM bytes:\r\n");
//    for (i = 0; i < RECRAM_BYTES / MAX_BLE_MSG_LENGTH; ++i) {
    for (i = 0; i < 5; ++i) {
        recrom_read(position, MAX_BLE_MSG_LENGTH, eeprom_buff);
        ble_ntransmit(eeprom_buff, MAX_BLE_MSG_LENGTH);
        position += MAX_BLE_MSG_LENGTH;
        delay_100ms();
    }
}

void put_record(char *record, unsigned int position) {
    char recrom_buff[RECORD_LENGTH];
    recrom_read(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);
    speaker_beep();
    char header = recrom_buff[0];
    unsigned char offset = (unsigned char) ((header & 0x03) + RECROM_HEADER_LENGTH);

    recrom_write(RECORD_LENGTH * position + offset, RECORD_LENGTH, record);
    speaker_beep();

    print_eeprom();
}

bool save_record(char *record) {
    char recram_buff[RECRAM_LENGTH];
    memset(recram_buff, 0x00, RECRAM_LENGTH);
    rtc_read(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);

    if (strncmp(recram_buff, RECRAM_MAGIC, MAGIC_LENGTH) == 0) {
        /* Decode the metadata from RTC registers (2B H:L) */
        unsigned int num_recs = (unsigned int) (
                ((unsigned char) recram_buff[NUMREC_LOFFSET] << 8)
                | (unsigned char) recram_buff[NUMREC_LOFFSET + 1]);
        unsigned int free_slot = (unsigned int) (
                ((unsigned char) recram_buff[FREESLOT_LOFFSET] << 8)
                | (unsigned char) recram_buff[FREESLOT_LOFFSET + 1]);

        char text[32];
        sprintf(text, "NR=%u, FS=%u\r\n", num_recs, free_slot);
        ble_send_string(text);

        /* Does the RECROM still have free space? */
        if (free_slot < RECRAM_MAX_RECS - RECORD_LENGTH) {
//            //TODO: odstranit!
//            memset(record, num_recs, RECORD_LENGTH);

            /* Write the record into memory */
            put_record(record, free_slot);

            /* Update metadata in RECRAM */
            recram_buff[NUMREC_LOFFSET] = (char) ((++num_recs >> 8) & 0xFF);
            recram_buff[NUMREC_LOFFSET + 1] = (char) (num_recs & 0xFF);
            recram_buff[FREESLOT_LOFFSET] = (char) ((++free_slot >> 8) & 0xFF);
            recram_buff[FREESLOT_LOFFSET + 1] = (char) (free_slot & 0xFF);
            rtc_write(NUMREC_OFFSET, NUMREC_LENGTH, recram_buff + NUMREC_LOFFSET);
            rtc_write(FREESLOT_OFFSET, FREESLOT_LENGTH, recram_buff + FREESLOT_LOFFSET);
        } else {
            //TODO: error, nedostatek místa!
            speaker_err();
        }
    } else {
        strncpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
        //TODO: spočítat průchodem RECROM
        memset(recram_buff + NUMREC_LOFFSET, 0x00, NUMREC_LENGTH);
        memset(recram_buff + FREESLOT_LOFFSET, 0x00, FREESLOT_LENGTH);
        rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
    }

    return false;
}

void remove_record(unsigned int position) {
    char recrom_buff[RECROM_HEADER_LENGTH];
    recrom_read(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);
    char header = recrom_buff[0];
    unsigned char offset = (unsigned char) ((header & 0x03) + RECROM_HEADER_LENGTH + RECORD_DEL_OFFSET);
    char record[RECORD_DEL_LEN];
    memset(record, RECORD_DEL_CHAR, RECORD_DEL_LEN);

    /* Check RECMEM consistency */
    init_recmanager();

    /* Delete the record from RECROM */
    recrom_write(RECORD_LENGTH * position + offset, RECORD_DEL_LEN, record);
    speaker_beep();

    /* */
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

    ble_ntransmit(recram_buff, RECRAM_LENGTH);
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
    strncpy(_timeram_buff, rtc_buff, TIMERAM_LENGTH);
    compress_time();
}

void recram_reset() {
    char recram_buff[RECRAM_LENGTH];
    strncpy(recram_buff + MAGIC_LOFFSET, RECRAM_MAGIC, MAGIC_LENGTH);
    memset(recram_buff + NUMREC_LOFFSET, 0x00, NUMREC_LENGTH);
    memset(recram_buff + FREESLOT_LOFFSET, 0x00, FREESLOT_LENGTH);
    rtc_write(RECRAM_OFFSET, RECRAM_LENGTH, recram_buff);
}

void recmem_reset() {
    //TODO: vymazání všech záznamů z RECROMky, tj. 0x00 na 3. byty

    char recrom_buff[RECROM_HEADER_LENGTH];
    recrom_buff[0] = 0x00;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);

    recram_reset();

    print_recram();
    print_eeprom();
}

void recmem_purge(void) {
    //TODO: vymazání celé RECROMky, tj. všechno na 0x00

    char recrom_buff[RECROM_HEADER_LENGTH];
    recrom_buff[0] = 0x00;
    recrom_write(RECROM_HEADER_OFFSET, RECROM_HEADER_LENGTH, recrom_buff);

    recram_reset();

    print_recram();
    print_eeprom();
}
