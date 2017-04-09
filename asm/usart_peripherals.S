

;####################################################################;
;# PROCEDURES FOR USART COMMUNICATION
;# ---------------------------------------------------------------- #;
;#  - used only for communication with BLE module
;####################################################################;


;;
; ----------------------------------------------------------------------------
; void ble_send_next_char(char *byte[Z], char len[LENGTH], char strict[CHAR]);
; ----------------------------------------------------------------------------
;
; Reads next character from SRAM under pointer 'Z' and sends it.
; The procedure ends when 'LENGTH' characters is transmitted,
; interrupt is then disabled, flag is cleared.
; //TODO
;;
ble_send_next_char:
    ; Read the next character
    LD    TEMP, Z+

    ; Put the character into transmission buffer
    STS   UDR0, TEMP

    ; Check if the number of characters to send is reached
    DEC   LENGTH
    BREQ  ble_send_next_char_last

    ; Check if the end character should be ignored
    CPI   CHAR, 0x00
    BRNE  ble_send_next_char_end

    ; Check for the end of string
    CPI   TEMP, 0x0A
    BRNE  ble_send_next_char_end

ble_send_next_char_last:
    ; Disallow interrupts on empty USART buffer
    LDS   TEMP, UCSR0B
    ANDI  TEMP, ~(1 << UDRIE0)
    STS   UCSR0B, TEMP

    ; Notify end of transmission by clearing the flag
    CBR   MFLAGS, (1 << MF_T)

ble_send_next_char_end:
RET


;;
; ---------------------------------------------------------
; void ble_ntransmit(char *data, char length, char strict);
; ---------------------------------------------------------
;
; Transmits first 'length' characters of given string.
; //TODO
;;
.global ble_ntransmit
ble_ntransmit:
    PUSH  r0

    ; Set the 'Z' pointer to point into given data buffer
    MOV   ZH, r25
    MOV   ZL, r24

    ; Set the number of characters to transmit
    MOV   LENGTH, r22

    ; Store the 'strict' boolean
    MOV   CHAR, r20

    ; Set the flag to indicate transmission
    SBR   MFLAGS, (1 << MF_T)

    ; Allows IDLE sleep mode
    LDI   TEMP, (1 << SE)
    OUT   SMCR, TEMP

    ; Allow interrupt on empty USART buffer
    LDS   TEMP, UCSR0B
    ORI   TEMP, (1 << UDRIE0)
    STS   UCSR0B, TEMP

    ; Sleep until next character can be transmitted
ble_ntransmit_loop:
    SBRC  MFLAGS, MF_T
    RJMP  ble_ntransmit_loop_sleep
    RJMP  ble_ntransmit_loop_end
ble_ntransmit_loop_sleep:
    SLEEP
    RJMP  ble_ntransmit_loop
ble_ntransmit_loop_end:

    ; Clear the sleep register as recomended
    LDI   TEMP, 0x00
    OUT   SMCR, TEMP

    CLR   r1
    POP   r0
RET


;;
; ------------------------------------------
; void ble_receive_next_char(char *byte[Z]);
; ------------------------------------------
;
; Reads received character and stores it on 'Z' pointer location,
; which it will post-increment.
; After reading 'LENGTH' characters, or when 0x0A='\n' is received,
; receiving procedure ends.
; //TODO
;;
ble_receive_next_char:
    ; Read the character
    LDS   TEMP, UDR0
    ST    Z+, TEMP

    ; Check the number of remaining characters
    DEC   LENGTH
    BREQ  ble_receive_next_char_last

    ; Check if the end character should be ignored
    CPI   CHAR, 0x00
    BRNE  ble_receive_next_char_end

    ; Check for the end of string
    CPI   TEMP, 0x0A
    BRNE  ble_receive_next_char_end

    ; Stop receiving more characters
ble_receive_next_char_last:
    ; Disallow interrupts on received character
    LDS   TEMP, UCSR0B
    ANDI  TEMP, ~(1 << RXCIE0)
    STS   UCSR0B, TEMP

    ; Notify end of receiving by clearing the flag
    CBR   MFLAGS, (1 << MF_R)

ble_receive_next_char_end:
RET


;;
; ----------------------------------------------------------
; void ble_nreceive(char *buffer, char length, char strict);
; ----------------------------------------------------------
;
; Receives and stores bytes into given buffer until it receives 0x0A='\n'
; or limit for number of characters 'length' is reached.
; //TODO
;;
.global ble_nreceive
ble_nreceive:
    PUSH  r0

    ; Set the 'Z' pointer to point into given buffer
    MOV   ZH, r25
    MOV   ZL, r24

    ; Maximum length of received string is limited
    MOV   LENGTH, r22

    ; Store the 'strict' boolean
    MOV   CHAR, r20

    ; Set the flag to indicate receiving
    SBR   MFLAGS, (1 << MF_R)

    ; Allows IDLE sleep mode
    LDI   TEMP, (1 << SE)
    OUT   SMCR, TEMP

    ; Allow interrupt on received character
    LDS   TEMP, UCSR0B
    ORI   TEMP, (1 << RXCIE0)
    STS   UCSR0B, TEMP

    ; Sleep until next character is received
ble_receive_loop:
    SBRC  MFLAGS, MF_R
    RJMP  ble_receive_loop_sleep
    RJMP  ble_receive_loop_end
ble_receive_loop_sleep:
    SLEEP
    RJMP  ble_receive_loop
ble_receive_loop_end:

    ; Clear the sleep register as recomended
    LDI   TEMP, 0x00
    OUT   SMCR, TEMP

    CLR   r1
    POP   r0
RET
