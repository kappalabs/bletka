

;####################################################################;
;# PROCEDURES FOR HANDLING INTERRUPTS
;# ---------------------------------------------------------------- #;
;####################################################################;


;;
; -----------------------
; void handle_udre(void);
; -----------------------
;
; Interrupt handler of empty USART buffer.
;;
.global handle_udre
handle_udre:
    PUSH  TEMP
    IN    TEMP, SREG
    PUSH  TEMP

    RCALL ble_send_next_char

    POP   TEMP
    OUT   SREG, TEMP
    POP   TEMP
RETI


;;
; -----------------------
; void handle_urxc(void);
; -----------------------
;
; Interrupt handler of received character on USART.
;;
.global handle_urxc
handle_urxc:
    PUSH  TEMP
    IN    TEMP, SREG
    PUSH  TEMP

    RCALL ble_receive_next_char

    POP   TEMP
    OUT   SREG, TEMP
    POP   TEMP
RETI


;;
; ----------------------
; void handle_twi(void);
; ----------------------
;
; Interrupt handler for I2C/TWI comunication.
;;
.global handle_twi
handle_twi:
;;TODO: zpracování jednotlivých možností: switch
RETI


;;
; -----------------------
; void handle_int0(void);
; -----------------------
;
; Handles button press interrupt.
;;
.global handle_int0
handle_int0:
    PUSH  TEMP
    IN    TEMP, SREG
    PUSH  TEMP
    PUSH  MFLAGS
    PUSH  A
    PUSH  B
    PUSH  C
    PUSH  r24
    PUSH  ZH
    PUSH  ZL

    ; Set Z to point to the 'mint_flags' variable
    LDI   ZH, hi8(mint_flags)
    LDI   ZL, lo8(mint_flags)

    ; Temporarily load 'mint_flags' variable
    LD    MFLAGS, Z

    ; Check if any interrupt flag is present
    RCALL is_activate_flagged
    CPI   r24, 0x00
    BRNE  handle_int0_end
    RCALL is_button_flagged
    CPI   r24, 0x00
    BRNE  handle_int0_end

    ; Debounce after button is pressed
    RCALL delay_button

    ; Pressed button corresponds with 0, released with 1
    SBIS  PINB, PB0
    SBR   MFLAGS, (1 << MF_B0)
    SBIS  PINB, PB1
    SBR   MFLAGS, (1 << MF_B1)
    SBIS  PINB, PB2
    SBR   MFLAGS, (1 << MF_B2)
    SBIS  PINB, PB3
    SBR   MFLAGS, (1 << MF_B3)
    SBIS  PINB, PB4
    SBR   MFLAGS, (1 << MF_B4)

    LDI   TEMP, 0x00

handle_int0_release:
    RCALL delay_50ms
    INC   TEMP

    ; Check if all buttons are released
    SBIS  PINB, PINB0
    RJMP  handle_int0_release
    SBIS  PINB, PINB1
    RJMP  handle_int0_release
    SBIS  PINB, PINB2
    RJMP  handle_int0_release
    SBIS  PINB, PINB3
    RJMP  handle_int0_release
    SBIS  PINB, PINB4
    RJMP  handle_int0_release

    ; Check if the press was long enough
    CPI   TEMP, 10
    BRLO  handle_int0_end
    SBR   MFLAGS, (1 << MF_A)

handle_int0_end:
    ; Save the result into the interrupt flags variable
    ST    Z, MFLAGS

    ; Debounce release of the button
    RCALL delay_button

    POP   ZL
    POP   ZH
    POP   r24
    POP   C
    POP   B
    POP   A
    POP   MFLAGS
    POP   TEMP
    OUT   SREG, TEMP
    POP   TEMP
RETI
