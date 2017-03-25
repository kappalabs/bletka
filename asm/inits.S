

;####################################################################;
;# PROCEDURES FOR INITIALIZATION
;# ---------------------------------------------------------------- #;
;#  - sets the clock speed to 4MHz
;#  - initializes ports, communication protocols
;####################################################################;


;;
; ------------------------
; void init_library(void);
; ------------------------
;
; Makes all initializations needed for this device.
;;
.global init_library
init_library:
    ; Port configurations
    RCALL init_ports

    ; Change MCU frequency to 8MHz(RC)/2(prescaler) = 4MHz
    LDI   TEMP,   (1 << CLKPCE)
    STS   CLKPR,  TEMP
    LDI   TEMP,   0x01
    STS   CLKPR,  TEMP

    ; USART configurations
    RCALL init_usart

    ; TWI/I2C configurations
    RCALL init_twi

    ; Allow general interrupts
    SEI
RET


;;
; ----------------------
; void init_ports(void);
; ----------------------
;
; Initialization of input/output ports.
;;
init_ports:
    ; Output pin for LED
    LDI   TEMP,   (1 << LED_DDRPIN)
    OUT   LED_DDR,TEMP

    ; Output pin for speaker
    LDI   TEMP,   (1 << SPK_DDRPIN)
    OUT   SPK_DDR,TEMP

    ; Input pins for buttons B[0-4]
    LDI   TEMP,   0xE0 ; 0xFF ^ ((1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4))
    OUT   DDRB,   TEMP
    ; Allow pull-up
    LDI   TEMP,   0x1F ; (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4)
    OUT   PORTB,  TEMP

    ; Allow external interrupt from buttons
    LDI   TEMP,   (1 << INT0)
    OUT   EIMSK,  TEMP

    ; Set INT0 as input pin with allowed pull-up resistor
    IN    TEMP,   DDRD
    ANDI  TEMP,   0xFD
    OUT   DDRD,   TEMP
    IN    TEMP,   PORTD
    ORI   TEMP,   (1 << PD2)
    OUT   PORTD,  TEMP
RET


;;
; ----------------------
; void init_usart(void);
; ----------------------
;
; Define comunication protocol of USART
;;
init_usart:
    ; Set baud rate 9600 [UBRR = 4MHz/(16*(9600))-1 ~ 25]
    LDI   TEMP,   hi8(25)
    STS   UBRR0H, TEMP
    LDI   TEMP,   lo8(25)
    STS   UBRR0L, TEMP

    ; Standard comunication speed
    LDI   TEMP,   0x00
    STS   UCSR0A, TEMP

    ; Allow reception and transmission
    LDI   TEMP,   (1 << RXEN0) | (1 << TXEN0)
    STS   UCSR0B, TEMP

    ; Asynchronous transmission, without parite bits, 1 stop bit, 8 bits in frame
    LDI   TEMP,   (1 << USBS0) | (3 << UCSZ00)
    STS   UCSR0C, TEMP
RET


;;
; --------------------
; void init_twi(void);
; --------------------
;
; Define comunication protocole of I2C/TWI.
;;
init_twi:
    ; Set divider to get 100kHz communication speed
    LDI   TEMP, (3 << TWBR0)
    STS   TWBR, TEMP
    LDI   TEMP, (0 << TWPS0)
    STS   TWSR, TEMP
RET
