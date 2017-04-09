#ifndef BLETKA_INTERRUPTS_H
#define BLETKA_INTERRUPTS_H

/**
 * ####################################################################
 *  INTERRUPT SERVICE ROUTINES
 * --------------------------------------------------------------------
 *  - in general, they use direct RJMP instruction -> no implicit
 *    PUSH/POP instructions!
 * ####################################################################
 */


/**
 * Interrupt Service Routine for
 * USART Rx Complete
 */
ISR(USART_RX_vect, ISR_NAKED;) {
    asm ("jmp handle_urxc"::);
}

/**
 * Interrupt Service Routine for
 * USART Data Register Empty
 */
ISR(USART_UDRE_vect, ISR_NAKED) {
    asm ("jmp handle_udre"::);
}

/**
 * Interrupt Service Routine for
 * 2-wire Serial Interface
 */
ISR(TWI_vect, ISR_NAKED) {
    asm ("jmp handle_twi"::);
}

/**
 * Interrupt Service Routine for
 * External Interrupt 0
 */
ISR(INT0_vect, ISR_NAKED) {
    asm ("jmp handle_int0"::);
}


#endif //BLETKA_INTERRUPTS_H
