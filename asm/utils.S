

;####################################################################;
;# GENERAL PURPOSE UTILITIES
;# ---------------------------------------------------------------- #;
;####################################################################;


;====================================================================;
;= SLEEP UTILITIES
;= ---------------------------------------------------------------- =;
;====================================================================;


;;
; ----------------------
; void deep_sleep(void);
; ----------------------
;
; Makes the device go into power down sleep mode.
;;
.global deep_sleep
deep_sleep:
    ; Set the sleep mode to power down
    LDI   TEMP, (2 << SM0) | (1 << SE)
    OUT   SMCR, TEMP

    ; Start sleeping
    SLEEP

    ; Clear the sleep register as recomended
    LDI   TEMP, 0x00
    OUT   SMCR, TEMP
RET



;====================================================================;
;= INDICATORS CONTROL
;= ---------------------------------------------------------------- =;
;====================================================================;


;;
; ------------------
; void led_on(void);
; ------------------
;
; Turns the LED on.
;;
.global led_on
led_on:
    LDI   TEMP, (1 << LED_PORTPIN)
    OUT   LED_PORT, TEMP
RET


;;
; -------------------
; void led_off(void);
; -------------------
;
; Turns the LED off.
;;
.global led_off
led_off:
    LDI   TEMP, (0 << LED_PORTPIN)
    OUT   LED_PORT, TEMP
RET


;;
; ----------------------
; void speaker_on(void);
; ----------------------
;
; Makes the speaker membrane to move up.
;;
.global speaker_on
speaker_on:
    LDI   TEMP, (1 << SPK_PORTPIN)
    OUT   SPK_PORT, TEMP
RET


;;
; -----------------------
; void speaker_off(void);
; -----------------------
;
; Makes the speaker membrane to move down.
;;
.global speaker_off
speaker_off:
    LDI   TEMP, (0 << SPK_PORTPIN)
    OUT   SPK_PORT, TEMP
RET


;;
; ------------------------
; void speaker_beep(void);
; ------------------------
;
; Makes a beep sound.
;;
.global speaker_beep
speaker_beep:
    LDI   C, 16
speaker_beep_loop:
    RCALL speaker_on
    RCALL delay_1136us
    RCALL speaker_off
    RCALL delay_1136us
    DEC   C
    BRNE  speaker_beep_loop
RET



;====================================================================;
;= FLAG CHECKS
;= ---------------------------------------------------------------- =;
;====================================================================;


;;
; -------------------------------
; char is_activate_flagged(void);
; -------------------------------
;
; Checks if the Activate flag is set.
; Returns 1 if set, 0 if cleared.
;;
.global is_activate_flagged
is_activate_flagged:
    LDI   ZH, hi8(mint_flags)
    LDI   ZL, lo8(mint_flags)
    LD    TEMP, Z
    ANDI  TEMP, (1 << MF_A)
    CPI   TEMP, 0x00
    BRNE  is_activate_flagged_true
    LDI   r24, 0x00
    RET
is_activate_flagged_true:
    LDI   r24, 0x01
RET


;;
; -----------------------------
; char is_button_flagged(void);
; -----------------------------
;
; Checks if the Activate flag is set.
; Returns 1 if set, 0 if cleared.
;;
.global is_button_flagged
is_button_flagged:
    LDI   ZH, hi8(mint_flags)
    LDI   ZL, lo8(mint_flags)
    LD    TEMP, Z
    ANDI  TEMP, MF_B_MSK
    CPI   TEMP, 0x00
    BRNE  is_button_flagged_true
    LDI   r24, 0x00
    RET
is_button_flagged_true:
    LDI   r24, 0x01
RET



;====================================================================;
;= TIME ENCODING
;= ---------------------------------------------------------------- =;
;====================================================================;


;;
; ----------------------------------------------------------
; char *[_timeram_buff{4B}]compress_time(char *[_timeram_buff{7B}]);
; ----------------------------------------------------------
;
; Makes a compression of 7B in '_timeram_buff' retrieved from
; RTC module into 4B.
;;
.global compress_time
compress_time:
    PUSH  r0
    PUSH  r2
    PUSH  r3
    PUSH  r4
    PUSH  r5
    PUSH  r6

    ; Sets 'Z' to point into the buffer with data for compression
    LDI   ZH, hi8(_timeram_buff)
    LDI   ZL, lo8(_timeram_buff)

    ; 1. byte (seconds) [6 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0x7F
    RCALL weird_to_binary
    MOV   r2, TEMP

    ; 2. byte (minutes) [6 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0x7F
    RCALL weird_to_binary
    MOV   r3, TEMP

    ; 3. byte (hours) [5 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0x3F
    RCALL weird_to_binary
    MOV   r4, TEMP

    ; 4. byte (day in week) - we do not care about this one
    LD    TEMP, Z+

    ; 5. byte (day) [5 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0x3F
    RCALL weird_to_binary
    MOV   r5, TEMP

    ; 6. byte (month) [4 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0x1F
    RCALL weird_to_binary
    MOV   r6, TEMP

    ; 7. byte (year) [6 bits]
    LD    TEMP, Z+
    ANDI  TEMP, 0xFF
    RCALL weird_to_binary
    ; Align and delete the MSbit, we only want 6 less significant ones
    LSL   TEMP
    LSL   TEMP

    ;# Composition of prepared 6B into 4B

    ; Resets the 'Z' position to the start of preprocessed buffer
    LDI   ZH, hi8(_timeram_buff)
    LDI   ZL, lo8(_timeram_buff)

    ; Aligns month and year to day
    ROL   TEMP
    ROL   r6
    ROL   TEMP
    ROL   r6
    ROL   TEMP
    ROL   r6
    ROL   TEMP
    ROL   r6
    ; Aligns month, year and day to hours
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   TEMP
    ROL   r6
    ROL   r5
    ; Aligns month, year, day and hours to minutes
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ; Aligns month, year, day, hours and minutes to seconds
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   r3
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   r3
    ; Aligns everything to the left fringe
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   r3
    ROL   r2
    ROL   TEMP
    ROL   r6
    ROL   r5
    ROL   r4
    ROL   r3
    ROL   r2

    ; Stores the compressed data back into given buffer
    ST    Z+, r2
    ST    Z+, r3
    ST    Z+, r4
    ST    Z+, r5
    ST    Z+, r6
    ST    Z+, TEMP

    POP   r6
    POP   r5
    POP   r4
    POP   r3
    POP   r2
    CLR   r1
    POP   r0
RET


;;
; --------------------------------------------
; char[TEMP] weird_to_binary(char byte[TEMP]);
; --------------------------------------------
;
; Makes conversion of given byte from pseudo-binary version
; from RTC module into standard form.
;
; RTC pseudo-binary form:
;   each nibble encodes binary one decimal numeral
;   -> decimal number has pseudo-binary form equal to
;      its decimal form (but not the other way around)
;;
weird_to_binary:
    PUSH  r16
    PUSH  r17

    ; Multiply the more significant nibble by 10
    MOV   r16, TEMP
    SWAP  r16
    ANDI  r16, 0x0F
    LDI   r17, 10
    MUL   r16, r17

    ; Add the less significant nibble
    ANDI  TEMP, 0x0F
    ADD   TEMP, r0

    POP   r17
    POP   r16
RET
