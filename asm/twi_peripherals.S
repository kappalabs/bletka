

;####################################################################;
;# METHODS SUPPPORTING WORK OVER TWI/I2C INTERFACE
;# ---------------------------------------------------------------- #;
;#  - designated for work with real time clock module
;####################################################################;


#define RTC_ADDRESS     0b01101000
#define EEPROM_ADDRESS  0b01010000

#define TWI_WRITE       0
#define TWI_READ        1


;;
; -------------------------------------------------------------------
; char rtc_write(char *position, unsigned char length, char *buffer);
; -------------------------------------------------------------------
;
; Writes given 'buffer' of given 'length' characters into RTC module
; begining at register 'position'.
;;
.global rtc_write
rtc_write:
    ; Store the 'position' into the low byte of 'X' register
    MOV   XL, r24

    ; Store the number of bytes to read
    MOV   LENGTH, r22

    ; Point the 'Z' register into 'buffer'
    MOV   ZH, r21
    MOV   ZL, r20

    ; Send START condition
    RCALL twi_start

    ; Send real clock address
    LDI   DATA, (RTC_ADDRESS << TWA0) | (TWI_WRITE)
    RCALL twi_write

    ; Send requested pointer position
    MOV   DATA, XL
    RCALL twi_write

    ; Send 'LENGTH' characters from array under 'Z'
rtc_write_loop:
    ; Send next character
    LD    DATA, Z+
    RCALL twi_write

    ; Check the next loop iteration
    DEC   LENGTH
    BRNE  rtc_write_loop

    ; Stop the comunication
    RCALL twi_stop
RET


;;
; ------------------------------------------------------------------
; char rtc_read(char *position, unsigned char length, char *buffer);
; ------------------------------------------------------------------
;
; Reads and stores registers of real time clock module from given
; 'position' of given 'length' into given 'buffer'.
;;
.global rtc_read
rtc_read:
    ; Store the 'position' into the low byte of 'X' register
    MOV   XL, r24

    ; Store the number of bytes to read
    MOV   LENGTH, r22

    ; Point the 'Z' register into 'buffer'
    MOV   ZH, r21
    MOV   ZL, r20

    ; Set the pointer in real time clock to 'position' register
    RCALL rtc_set_pointer

    ; Reads 'LENGTH' registers from real time module, stores them under 'Z' register
    RCALL rtc_read_registers
RET


;;
; --------------------------------------------
; void rtc_set_pointer(char *position[XL]);
; --------------------------------------------
;
; Sets the pointer inside real time clock module to point
; on given register address/position.
;;
rtc_set_pointer:
    ; Send START condition
    RCALL twi_start

    ; Send real clock address
    LDI   DATA, (RTC_ADDRESS << TWA0) | (TWI_WRITE)
    RCALL twi_write

    ; Send requested pointer position
    MOV   DATA, XL
    RCALL twi_write

    ; Stop the comunication
    RCALL twi_stop
RET

;;
; ----------------------------------------------
; void rtc_reset_pointer(char *position[XL]);
; ----------------------------------------------
;
; Sets the pointer inside real time clock module to point
; on first register.
;;
rtc_reset_pointer:
    LDI   XL, 0x00
    RCALL rtc_set_pointer
RET


;;
; -----------------------------------------
; void rtc_read_registers(char *buffer[Z]);
; -----------------------------------------
;
; Reads 'LENGTH' characters into buffer under 'Z' pointer.
;;
rtc_read_registers:
    ; Send START condition
    RCALL twi_start

    ; Send RTC device address and request for read
    LDI   DATA, (RTC_ADDRESS << TWA0) | (TWI_READ)
    RCALL twi_write

    ; Read bytes send back from RTC
rtc_read_registers_receive:
    DEC   LENGTH
    BREQ  rtc_read_registers_receive_clc
    SEC
    RJMP  rtc_read_registers_receive_read
rtc_read_registers_receive_clc:
    CLC

rtc_read_registers_receive_read:
    ; Read character on TWI
    RCALL twi_read

    ; Save received data
    ST    Z+, DATA

    ; Check if comunication was succesfull
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8

    ; Data byte has been received; ACK has been returned
    CPI   TEMP, 0x50
    BREQ  rtc_read_registers_receive

    ; Data byte has been received; NOT ACK has been returned
    CPI   TEMP, 0x58
    BRNE  rtc_read_registers_receive

rtc_read_registers_receive_end:
    ; Stop the comunication
    RCALL twi_stop
RET


;;
; --------------------------------
; bool[Carry bit] twi_start(void);
; --------------------------------
;
; Sends a START condition over TWI/I2C.
; Sets 'Carry' bit to 1 on success, 0 on fail.
;;
twi_start:
    ; Sets request to send the START condition
    LDI   TEMP, (1 << TWINT) | (1 << TWSTA) | (1 << TWEN)
    STS   TWCR, TEMP

    ; Waits for the condition to be send
    RCALL	twi_wait

    ; Checks success of the operation
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8

    ; A START condition has been transmitted?
    CPI   TEMP, 0x08
    BREQ  twi_start_free

    ; A repeated START condition has been transmitted?
    CPI   TEMP, 0x10
    BRNE  twi_start_exit

    ; START condition send without a problem
twi_start_free:
    SEC
    RET

    ; A problem occured
twi_start_exit:
    CLC
RET


;;
; --------------------
; void twi_wait(void);
; --------------------
;
; Waits until the TWI is ready again.
;;
twi_wait:
    LDS   TEMP, TWCR
    SBRS  TEMP, TWINT
    RJMP  twi_wait
RET


;;
; --------------------
; void twi_stop(void);
; --------------------
;
; Sends a STOP condition over TWI/I2C.
;;
twi_stop:
    ; Sets request to send a STOP condition
    LDI   TEMP, (1 << TWINT) | (1 << TWEN) | (1 << TWSTO)
    STS   TWCR, TEMP

    ; Waits until the condition is send
twi_stop_wait:
    LDS   TEMP, TWCR
    ANDI  TEMP, 0x10
    BRNE  twi_stop_wait
RET


//TODO: naformátovat, zkontrolovat

;;
; char twi_read(void);
;
; Přečte jeden znak poslaný přes TWI. Odešle ACK
; pokud je Carry bit nastaven, jinak odešle NACK.
;;
twi_read:
    ; Povolení čtení zatím s NACK
    LDI   TEMP, (1 << TWINT) | (1 << TWEN)

    ; Na základě hodnoty Carry bitu rozhodni o hodnotě ACK
    BRCC  twi_read_nack

    ; Požádej o ACK
    ORI   TEMP, (1 << TWEA)

twi_read_nack:
    ; Nastav požadavek na TWI
    STS   TWCR, TEMP

    ; Vyčkej na data
    RCALL twi_wait

    ; Ulož přijatá data
    LDS   DATA, TWDR
RET


;;
; bool[Carry bit] twi_write(data [DATA]);
;
; Odešle 1 byte v registru 'DATA' po TWI/I2C.
; Do 'Carry' bitu nastaví 1 při úspěchu, 0 při neúspěchu.
;;
twi_write:
    ; Pozastavení TWI
    LDI   TEMP, (0 << TWINT) | (1 << TWEN)
    STS   TWCR, TEMP

    ; Vložení dat do odesílacího bufferu
    STS   TWDR, DATA

    ; Nastavení požadavku na odeslání dat
    LDI   TEMP, (1 << TWINT) | (1 << TWEN)
    STS   TWCR, TEMP

    ; Počkání na odeslání
    RCALL twi_wait

    ; Zkontroluje úspěšnost odeslání
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8
    CPI   TEMP, 0x40  ; SLA+R has been transmitted; ACK has been received?
    BREQ  twi_write_ok
    CPI   TEMP, 0x28  ; Data byte has been transmitted; ACK has been received?
    BREQ  twi_write_ok
    CPI   TEMP, 0x40  ; SLA+R has been transmitted; ACK has been received?
    BREQ  twi_write_ok
    CPI   TEMP, 0x50  ; Data byte has been received; ACK has been returned?
    BREQ  twi_write_ok

    ; Oznámíme neúspěšné odeslání
    CLC
    RET
twi_write_ok:
    SEC
RET


;########################;
;#  I2C EEPROM UTILITY  #;
;########################;


;;
; void recrom_read(unsigned int position, unsigned char length, char *buffer);
;
; 
;;
.global recrom_read
recrom_read:
    PUSH  r0
    PUSH  r28
    PUSH  r29

    ; Dočasné uložení adresy
    MOV   XH, r25
    MOV   XL, r24

    ; Uchování délky čtené sekvence
    MOV   LENGTH, r22

    ; Nastavení adresy bufferu
    MOV   ZH, r21
    MOV   ZL, r20

    ; { DUMMY nastavení adresy
        ; Odešli START sekvenci
        RCALL twi_start

        ; Odešli adresu EEPROM paměti
        LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_WRITE)
        RCALL twi_write

        ; Odešle MSB adresy v paměti
        MOV   DATA, XH
        RCALL twi_write

        ; Odešle LSB adresy v paměti
        MOV   DATA, XL
        RCALL twi_write
    ; }

    ; Odešli START sekvenci
    RCALL twi_start

    ; Odešli adresu paměti + požadavek na čtení
    LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_READ)
    RCALL twi_write

; Čti jednotlivé byty zasílané pamětí
memory_read_registers_receive:
    ; Kontrola délky čteného řetězce
    DEC   LENGTH
    BREQ  memory_read_registers_receive_clc
    SEC
    RJMP  memory_read_registers_receive_read
memory_read_registers_receive_clc:
    CLC

memory_read_registers_receive_read:
    ; Přečtení jednoho bytu a odeslání (N)ACK dle předchozí podmínky
    RCALL twi_read

    ; Ulož přečtená data
    ST    Z+, DATA

    ; Zkontroluje úspěšnost odeslání
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8

    ; Data byte has been received; ACK has been returned
    CPI   TEMP, 0x50
    BREQ  memory_read_registers_receive
    ; Data byte has been received; NOT ACK has been returned
    CPI   TEMP, 0x58
    BRNE  memory_read_registers_receive

memory_read_registers_receive_end:
    ; Ukončení komunikace
    RCALL twi_stop

    POP   r29
    POP   r28
    CLR   r1
    POP   r0
RET


;;
; bool[Carry] recrom_write(unsigned int position, unsigned char length, char *buffer);
;
; Zapíše do paměti na danou adresu daný byte.
;;
.global recrom_write
recrom_write:
    PUSH  r0
    PUSH  r28
    PUSH  r29

    ; Dočasné uložení adresy EEPROM paměti
    MOV   XH, r25
    MOV   XL, r24

    ; Uchování délky sekvence k zápisu
    MOV   LENGTH, r22

    ; Uložení adresy pole dat k zápisu
    MOV   ZH, r21
    MOV   ZL, r20

    ; Odešli START sekvenci
    RCALL twi_start

    ; Odešli adresu EEPROM paměti
    LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_WRITE)
    RCALL twi_write

    ; Odešle MSB adresy v paměti
    MOV   DATA, XH
    RCALL twi_write

    ; Odešle LSB adresy v paměti
    MOV   DATA, XL
    RCALL twi_write

    ; Kontrola nenulovosti délky řetězce k odeslání
    CPI   LENGTH, 0x00
    BREQ  memory_write_loop_end

; Odesílej jednotlivé byty
memory_write_loop:
    ; Odešli další byte z řetězce
    LD    DATA, Z+
    RCALL twi_write

    ; Kontrola délky odeslaného řetězce
    DEC   LENGTH
    BRNE  memory_write_loop

memory_write_loop_end:

    ; Ukončení komunikace
    RCALL twi_stop

    POP   r29
    POP   r28
    CLR   r1
    POP   r0
RET
