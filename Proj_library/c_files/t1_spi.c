/**
 * @Acknowledgement: Special thanks to Rhys Thomas <rt8g15@soton.ac.uk> for
 * providing the github repository files (open source) for the zetaplus radio
 * module in C language to use with msp430 devices.
 *
 * @ github repository -> https://github.com/rhthomas/pt3p-zeta
 *
 * @modified by Piotr Krawiec <p.j.krawiec1@newcastle.ac.uk>
 * @date 02/12/2022
**/

#include <Proj_Library/h_files/t1_spi.h>

void spi_init(void){

    /* PORT 5 - SPI
     * Pin 0 - [Zeta+] SDI  [MOSI]      (input)
     * Pin 1 - [Zeta+] SDO  [MISO]      (output)
     * Pin 2 - [Zeta+] SCLK [SPI CLK]   (output)
     * Pin 3 - [Zeta+] nSEL [CS]        (output)
     */

    P5DIR |= (MOSI + CS + SCLK);
    P5DIR &= ~MISO;

    P5REN |= MISO;
    P5OUT |= MISO;

    P5SEL0 |= (MOSI | MISO | SCLK);

    // Set automatic CS pin.
#ifndef MANUAL
    P5OUT |= CS;
    P5SEL0 |= CS;
#endif // MANUAL

    // Reset state-machine.
    UCB1CTLW0 |= UCSWRST;

    /* SPI configuration:
     * 1. Master 3-pin mode.
     * 2. Synchronous.
     * 3. MSB first.
     * 4. Data captured on first clock edge and changed on following.
     * x. UCB1STE (P5.3) as CS pin. (not used, now manual)
     * x. Active low. (not used, now manual)
     * 5. SMCLK as source. */

    UCB1CTLW0 |= (UCMST | UCSYNC | UCMSB | UCCKPH);
#ifndef MANUAL
    UCB1CTLW0 |= (UCMODE1 | UCSTEM);
#endif // MANUAL
    // Running the SPI clk at 1MHz (@SMCLK SPEED).
    UCB1CTLW0 |= UCSSEL_2;
    // initialise the state-machine
    UCB1CTLW0 &= ~UCSWRST;
}

uint8_t spi_xfer(uint8_t byte)
{
    while (!(UCB1IFG & UCTXIFG0))
        ; // Wait until Tx buffer is ready.
    UCB1TXBUF = byte;
    while (!(UCB1IFG & UCRXIFG0))
        ; // Wait until Rx buffer is ready.
    return UCB1RXBUF;
}

inline void spi_cs_high(void){
    P5OUT |= CS;
}

inline void spi_cs_low(void){
    P5OUT &= ~CS;
}
