

;####################################################################;
;# PROCEDURES FOR USART COMMUNICATION
;# ---------------------------------------------------------------- #;
;#  - used only for communication with BLE module
;####################################################################;


;;
; ----------------------------------------------------------------------------
; void ble_send_next_char(char *byte[Z], char len[LENGTH], bool strict[CHAR]);
; ----------------------------------------------------------------------------
;
; Reads next character from SRAM under pointer 'Z' and sends it.
; The procedure ends when 'LENGTH' characters are transmitted,
; or the 'strict' mode is disabled and 0x0A='\n' character was transmitted,
; interrupt for transmission is then disabled, flag is cleared.
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
; -----------------------------------------------------------
; void ble_ntransmit(char *buffer, char length, bool strict);
; -----------------------------------------------------------
;
; Transmits first 'length' characters from 'buffer'.
; If 'strict' is true(!=0x00), then exactly 'length' characters will be transmitted,
; otherwise the transmission ends after first 0x0A='\n' character is transmitted.
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
; ---------------------------------
; void ble_receive_next_char(void);
; ---------------------------------
;
; Reads received character and stores it in '_usart_rb' under current
; '_usart_rbw' position, which is then incremented.
;;
ble_receive_next_char:
    PUSH  CHAR
    PUSH  TEMP
    IN    TEMP, SREG
    PUSH  TEMP
    PUSH  ZL
    PUSH  ZH
    PUSH  XL
    PUSH  XH

    ; Read the character
    LDS   CHAR, UDR0

    ; Sets 'Z' to point into the buffer
    LDI   ZH, hi8(_usart_rb)
    LDI   ZL, lo8(_usart_rb)

    ; Sets 'X' to contain the write location
    LDI   XH, hi8(_usart_rbw)
    LDI   XL, lo8(_usart_rbw)

    ; Load current write position and point 'Z' to it
    LD    TEMP, X
    CLR   r1
    ADD   ZL, TEMP
    ADC   ZH, r1

    ; Store the character into buffer
    ST    Z, CHAR

    ; Increase the RBW value and store it
    INC   TEMP
    ST    X, TEMP

    POP   XH
    POP   XL
    POP   ZH
    POP   ZL
    POP   TEMP
    OUT   SREG, TEMP
    POP   TEMP
    POP   CHAR
RET


;;
; --------------------------------------------
; void ble_nreceive(char length, bool strict);
; --------------------------------------------
;
; Reads characters transmitted by BLE module and stores them into '_usart_rb'
; buffer until 'length' characters is read, or 'strict' is false(=0x00)
; and 0x0A='\n' character was received.
;;
.global ble_nreceive
ble_nreceive:
    PUSH  r0

    ; Set the number of characters to transmit
    MOV   r0, r24

    ; Store the 'strict' boolean
    MOV   CHAR, r22

    ; LENGTH is located at r22!
    MOV   LENGTH, r0

    ; Wait until conditions are met
ble_nreceive_loop:
    ; Check if the length is reached
    LDI   ZH, hi8(_usart_rbw)
    LDI   ZL, lo8(_usart_rbw)
    LD    r1, Z
    MOV   r0, r1
    LDI   XH, hi8(_usart_rbr)
    LDI   XL, lo8(_usart_rbr)
    LD    TEMP, X
    SUB   r1, TEMP
    CP    r1, LENGTH
    BRSH  ble_nreceive_loop_end

    ; If nothing was read yet, go directly to sleep mode - wait for a character
    CPI   LENGTH, 0
    BREQ  ble_nreceive_loop_sleep

    ; Check strict mode
    CPI   CHAR, 0x00
    BRNE  ble_nreceive_loop_sleep

    ; Check if the '\n' character was read
    LDI   ZH, hi8(_usart_rb)
    LDI   ZL, lo8(_usart_rb)
    ; Load the last read character
    DEC   r0
    CLR   r1
    ADD   ZL, r0
    ADC   ZH, r1
    LD    TEMP, Z
    ; Do the check
    CPI   TEMP, 0x0A
    BREQ  ble_nreceive_loop_end
ble_nreceive_loop_sleep:
    RCALL delay_10ms
    RJMP  ble_nreceive_loop
ble_nreceive_loop_end:

    CLR   r1
    POP   r0
RET


;;
; -----------------------------
; void ble_allow_receive(void);
; -----------------------------
;
; Allows interrupt-driven receive of characters from USART
; into '_usart_rb'.
;;
.global ble_allow_receive
ble_allow_receive:
    ; Allow interrupt on received character
    LDS   TEMP, UCSR0B
    ORI   TEMP, (1 << RXCIE0)
    STS   UCSR0B, TEMP
RET


;;
; --------------------------------
; void ble_disallow_receive(void);
; --------------------------------
;
; Disallows interrupt-driven receive of characters from USART
; into '_usart_rb'.
;;
.global ble_disallow_receive
ble_disallow_receive:
    ; Disallow interrupts on received character
    LDS   TEMP, UCSR0B
    ANDI  TEMP, ~(1 << RXCIE0)
    STS   UCSR0B, TEMP
RET
