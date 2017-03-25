

#ifndef BLETKA_DEFINES_S
#define BLETKA_DEFINES_S

//####################################################################
//# CONSTANT AND VARIABLE DEFINITIONS
//# ---------------------------------------------------------------- #
//####################################################################


#define LED_PORT      PORTD
#define LED_PORTPIN   PORTD7
#define LED_DDR       DDRD
#define LED_DDRPIN    DDD7
#define SPK_PORT      PORTC
#define SPK_PORTPIN   PORTC0
#define SPK_DDR       DDRC
#define SPK_DDRPIN    DDC0

#define MF_R      0
#define MF_T      1
#define MF_B0     2
#define MF_B1     3
#define MF_B2     4
#define MF_B3     5
#define MF_B4     6
#define MF_A      7

#define MF_B_MSK  ((1 << MF_B0) | (1 << MF_B1) | (1 << MF_B2) | (1 << MF_B3) | (1 << MF_B4))

#define TEMP      r25
#define CHAR      r24
// --------------------------------------
// | A | B4 | B3 | B2 | B1 | B0 | T | R |
// --7--------------------------------0--
#define MFLAGS    r23
#define LENGTH    r22
#define DATA      r21
#define A         r20
#define B         r19
#define C         r18


#endif // BLETKA_DEFINES_S
