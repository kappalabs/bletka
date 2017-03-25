

;####################################################################;
;# PROCEDURES FOR ACTIVE WAITING
;# ---------------------------------------------------------------- #;
;#  - designed for 4MHz
;#  - includes 7 cycles for RCALL a RET instructions
;####################################################################;


;;
; ------------------------
; void delay_button(void);
; ------------------------
;
; Active waiting for purposes of button debounce.
;;
delay_button:
    RCALL delay_50ms
RET


;;
; ------------------------
; void delay_1136us(void);
; ------------------------
;
; Exactly 1136us, 4544 - 7 cycles (for RCALL & RET).
; Can be used to generate A tone (440Hz).
;;
delay_1136us:
    LDI   A, 6
    LDI   B, 228
delay_1136us_loop:
    DEC   B
    BRNE  delay_1136us_loop
    DEC   A
    BRNE  delay_1136us_loop
RET


;;
; ----------------------
; void delay_50ms(void);
; ----------------------
;
; Exactly 50ms, 0.2M - 7 cycles (for RCALL & RET).
;;
.global delay_50ms
delay_50ms:
    LDI   A, 2
    LDI   B, 4
    LDI   C, 185
delay_50ms_loop:
    DEC   C
    BRNE  delay_50ms_loop
    DEC   B
    BRNE  delay_50ms_loop
    DEC   A
    BRNE  delay_50ms_loop
RET


;;
; -----------------------
; void delay_100ms(void);
; -----------------------
;
; Exactly 100ms, 0.4M - 7 cycles (for RCALL & RET).
;;
.global delay_100ms
delay_100ms:
    LDI   A, 3
    LDI   B, 8
    LDI   C, 117
delay_100ms_loop:
    DEC   C
    BRNE  delay_100ms_loop
    DEC   B
    BRNE  delay_100ms_loop
    DEC   A
    BRNE  delay_100ms_loop
    NOP
    NOP
RET


;;
; -----------------------
; void delay_500ms(void);
; -----------------------
;
; Exactly 500ms, 2M - 7 cycles (for RCALL & RET).
;;
.global delay_500ms
delay_500ms:
    LDI   A, 11
    LDI   B, 38
    LDI   C, 92
delay_500ms_loop:
    DEC   C
    BRNE  delay_500ms_loop
    DEC   B
    BRNE  delay_500ms_loop
    DEC   A
    BRNE  delay_500ms_loop
    NOP
RET


;;
; --------------------
; void delay_1s(void);
; ---------------------
;
; Exactly 1s, 4M - 7 cycles (for RCALL & RET).
;;
.global delay_1s
delay_1s:
    LDI   A, 21
    LDI   B, 75
    LDI   C, 189
delay_1s_loop:
    DEC   C
    BRNE  delay_1s_loop
    DEC   B
    BRNE  delay_1s_loop
    DEC   A
    BRNE  delay_1s_loop
RET
