

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
; ---------------------------------------------------
; rtc_read(char position, char length, char *buffer);
; ---------------------------------------------------
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
; ---------------------------------------------------------
; void rtc_write(char position, char length, char *buffer);
; ---------------------------------------------------------
;
; Writes given 'buffer' of given 'length' characters into RTC module
; beginning at register 'position'.
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
; ----------------------------------------
; void rtc_set_pointer(char position[XL]);
; ----------------------------------------
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
; ------------------------------------------
; void rtc_reset_pointer(char position[XL]);
; ------------------------------------------
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


;;
; -----------------------------------
; char twi_read(bool ack[Carry bit]);
; -----------------------------------
;
; Reads single character over TWI. Sends ACK if Carry bit
; is set, otherwise sends NACK.
;;
twi_read:
    ; Prepare reading with NACK
    LDI   TEMP, (1 << TWINT) | (1 << TWEN)

    ; Decide based on Carry bit
    BRCC  twi_read_nack

    ; Change request for NACK to ACK
    ORI   TEMP, (1 << TWEA)

twi_read_nack:
    ; Apply prepared request on TWI
    STS   TWCR, TEMP

    ; Wait for required data
    RCALL twi_wait

    ; Store received data
    LDS   DATA, TWDR
RET


;;
; -------------------------------------------
; bool[Carry bit] twi_write(char byte[DATA]);
; -------------------------------------------
;
; Sends 1 byte from 'DATA' over TWI/I2C.
; Sets 'Carry' bit to 1 on success, 0 on fail.
;;
twi_write:
    ; Disable TWI for a while
    LDI   TEMP, (0 << TWINT) | (1 << TWEN)
    STS   TWCR, TEMP

    ; Put the data into transmission buffer
    STS   TWDR, DATA

    ; Set request to send the data
    LDI   TEMP, (1 << TWINT) | (1 << TWEN)
    STS   TWCR, TEMP

    ; Wait for the data to be send
    RCALL twi_wait

    ; Check transmisison success
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8
    ; SLA+R has been transmitted; ACK has been received?
    CPI   TEMP, 0x40
    BREQ  twi_write_ok
    ; Data byte has been transmitted; ACK has been received?
    CPI   TEMP, 0x28
    BREQ  twi_write_ok
    ; SLA+R has been transmitted; ACK has been received?
    CPI   TEMP, 0x40
    BREQ  twi_write_ok
    ; Data byte has been received; ACK has been returned?
    CPI   TEMP, 0x50
    BREQ  twi_write_ok

    ; Clear the carry bit to notify about failure
    CLC
    RET

twi_write_ok:
    ; Set the carry bit to notify about success
    SEC
RET



;====================================================================;
;=  I2C EEPROM UTILITY
;= ---------------------------------------------------------------- =;
;====================================================================;


;;
; ----------------------------------------------------------
; void recrom_read(int position, char length, char *buffer);
; ----------------------------------------------------------
;
; Reads 'length' characters into 'buffer' from given 'position'
; in RTC EEPROM memory.
;;
.global recrom_read
recrom_read:
    ; Store the 'position' value
    MOV   XH, r25
    MOV   XL, r24

    ; Store the 'length' of the data to be read
    MOV   LENGTH, r22

    ; Store the 'buffer' address
    MOV   ZH, r21
    MOV   ZL, r20

    ; { DUMMY send address - sets address position to be read from
        ; Send START condition
        RCALL twi_start

        ; Send EEPROM address
        LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_WRITE)
        RCALL twi_write

        ; Send MSB of the memory address
        MOV   DATA, XH
        RCALL twi_write

        ; Send LSB of the memory address
        MOV   DATA, XL
        RCALL twi_write
    ; }

    ; Send START condition
    RCALL twi_start

    ; Send memory I2C address with request to read
    LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_READ)
    RCALL twi_write

    ; Read data send by RTC
recrom_read_loop:
    ; Check the length of received string
    DEC   LENGTH
    BREQ  recrom_read_clc
    SEC
    RJMP  recrom_read_receive
recrom_read_clc:
    CLC
recrom_read_receive:
    ; Read one byte and send ACK or NACK based on previous condition
    RCALL twi_read

    ; Store received data
    ST    Z+, DATA

    ; Check if receive was successfull
    LDS   TEMP, TWSR
    ANDI  TEMP, 0xF8
    ; Data byte has been received; ACK has been returned
    CPI   TEMP, 0x50
    BREQ  recrom_read_loop
    ; Data byte has been received; NOT ACK has been returned
    CPI   TEMP, 0x58
    BRNE  recrom_read_loop

recrom_read_receive_end:
    ; Stop the comunication
    RCALL twi_stop
RET


;;
; -----------------------------------------------------------
; void recrom_write(int position, char length, char *buffer);
; -----------------------------------------------------------
;
; Writes 'length' characters from given 'buffer' starting at 'position'
; into RTC module EEPROM.
;;
.global recrom_write
recrom_write:
    ; Store the address in EEPROM to write to
    MOV   XH, r25
    MOV   XL, r24

    ; Store the length of sequence to be transmitted
    MOV   LENGTH, r22

    ; Store the pointer to 'buffer' containing data
    MOV   ZH, r21
    MOV   ZL, r20

    ; Send START condition
    RCALL twi_start

    ; Send EEPROM I2C address with request to write
    LDI   DATA, (EEPROM_ADDRESS << TWA0) | (TWI_WRITE)
    RCALL twi_write

    ; Send MSB of the memory address
    MOV   DATA, XH
    RCALL twi_write

    ; Send LSB of the memory address
    MOV   DATA, XL
    RCALL twi_write

    ; Check if there is someting to be send
    CPI   LENGTH, 0x00
    BREQ  memory_write_loop_end

    ; Send the whole sequence byte by byte
memory_write_loop:
    ; Send next byte from the buffer
    LD    DATA, Z+
    RCALL twi_write

    ; Check length of transmitted string
    DEC   LENGTH
    BRNE  memory_write_loop

memory_write_loop_end:
    ; Stop the comunication
    RCALL twi_stop

    ; Delay according the documentation when on >=2.5-volt
    RCALL delay_10ms
RET
