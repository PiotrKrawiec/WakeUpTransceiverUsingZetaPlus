/* For the SPI bus test, we are seeing if the SPI bus operates as expected.
 * Therefore, we can write that we are transmitting a packet of length 1 with
 * binary value of 15 for example. When inspecting the outcome, it should be
 * observed that there are 5 instances of when the nSSEL pin goes low - during
 * this time, data is being transfered at the Serial data output pin of the MSP430.
 *
 * The SPI pins on the MSP430 investigated are:
 * MOSI - P5.0
 * SCLK - P5.2
 * (CS) - P5.3
 * GPIO trigger to capture Osciloscope wave: P7.3
 */

#include <Proj_library/h_files/t1_util.h>   //system set up (pins, functions etc.)
#include <Proj_library/h_files/t1_zeta.h>   //radio functions

int main (void){

    // Disable the GPIO power-on default high-impedance mode on configured port settings.
    PM5CTL0 &= ~(LOCKLPM5);

    //  stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //  Initialise system.
    io_init();
    clock_init();
    spi_init();
    zeta_init();

    // For debugging, Indicate SPI transmission will occur by setting P1.1 on.
     P1OUT |= BIT1;
    // Osciliscope triggered to start by P7 BIT3 (rising edge)
    P7OUT |= BIT3;

    // Set zeta operating mode 2 for transmitting (ATM READY)
    zeta_select_mode(0x2);

    __delay_cycles(8e6);

    // Data packet
    zeta_send_open(CHANNEL,1);
    zeta_write_byte(0x6);
    zeta_send_close();

    // Osciliscope triggered to stop by P7 BIT3 (rising edge)
    P7OUT &= BIT3;

    // Wait 1 second to indicate SPI transfer on and complete on msp430 board.
    __delay_cycles(8e6);

    P1OUT &= ~BIT1;
}
