/* This file along with the receiver_test.c file tests the functionality of the zetaplus
 * radio module. The test is simple - an intended data byte of '6' is used to test if the
 * transmitter and receiver radio modules can pick up the byte.
 */

#include <Proj_library/h_files/t1_util.h>   //system set up (pins, functions etc.)
#include <Proj_library/h_files/t1_zeta.h>   //radio functions

int main(void)
{
    // Disable the GPIO power-on default high-impedance mode on configured port settings.
    PM5CTL0 &= ~(LOCKLPM5);

    //  stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //  Initialise system
    io_init();
    clock_init();
    spi_init();
    zeta_init();

    while(1){
        // Turning off Comparator Interrupt to prevent spiraling into isr_trap.asm
        P4IE &= ~(EXT_COMP);

        // Set zeta operating mode 2 (ATM Ready)
        zeta_select_mode(0x2);

        uint8_t data = 0x6;
        uint8_t data_out = data + 0x21;

        /* ZetaRadio communicates in ascii, therefore uint8_t value '6' will provide an
            acknowledgement. Special characters up to decimal 32 (exception decimal 127) &
            decimal 33 is '!'. Therefore an idea is to offset 6 by 33 and then fix this at
            the receiver side to transmit 4-bit binary values. */

        //uint8_t write_out[1u] = {data_out + '!'};
        uint8_t write_out[1u] = {data_out};
        while(1){
            // For debugging, Indicate transmission function is running with LED P1.1.
            P1OUT |= BIT1;

            // Wait 1 second to indicate SPI transfer on and complete on msp430 board.
            wait_one_second();

            // Transmit Data packet
            zeta_send_open(CHANNEL,1u);
            zeta_write_byte(data);      // replace 'data' with 'write_out[0]'
            zeta_send_close();

            // Display data sent
            led_set(data);

            // Wait 1 second to indicate SPI transfer on and complete on msp430 board.
            wait_one_second();

            // Clear led output
            led_clear();
        }
    }
}
