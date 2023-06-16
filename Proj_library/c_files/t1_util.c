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

#include <Proj_Library/h_files/t1_util.h>
#include <Proj_Library/h_files/t1_zeta.h>
#include <Proj_Library/h_files/t1_spi.h>

// Only initialised when flashing.
#pragma PERSISTENT (mailbox)
buffer_t mailbox = {0};

volatile uint8_t timerB_exit = 0;

//*************************************************************************************
void io_init(void)
{
    /*
     * PxDIR sets pin to output/input
     * PxIn reads the individual pin values (0/1)
     * PxOut sets the value of the pin (0/1)
     * PxSEL(0/1/2) sets the mode of the pin (see datasheet)
     * PxREN enables resistor on output (pull up/down)
     * Setting which pins are being used and in which mode.
     */

    /* PORT 1 - Node Status
     * Pin 0 - Hibernus Active
     * Pin 1 - Transmitting/Receiving packet function active */
    P1OUT   =   0x00;
    P1DIR   =   (BIT1+BIT0);


    /* PORT 2
     * Pin 5 - UB20M PGO    [NLATCH]    (input) */
    P2DIR = 0xFF;
    P2OUT &= ~PS_LATCH;

    /* PORT 3
     * This port is defined later in SPI.H */

    /* PORT 4
     * Pin 1 - external comparator  [EXT_COMP]  (input) */
    P4DIR = 0xFF & ~(EXT_COMP);
    P4OUT = 0;
    P4REN = 0;
    P4IE |= EXT_COMP;

    /* PORT 8 - Active Operation LEDs
     * Pin 0 - LED 0 (output)
     * Pin 1 - LED 1 (output)
     * Pin 2 - LED 2 (output)
     * Pin 3 - LED 3 (output) */
    P8DIR = 0x0F;
    P8OUT = ~(0x0F);

    /* The remaining ports are left unused */
}

//*************************************************************************************
void clock_init(void)
{
    // Unlock CS registers.
    CSCTL0_H = 0xA5;                        // 0xAH unlocks register, see family user guide

    // Set DCORSEL = 0.
    CSCTL1 &= ~(DCORSEL);

    // Set DCO to 8MHz.
    CSCTL1 |=  DCOFSEL_6;                   // if DCOR = 0, config DCOF_6 = 8MHz.

    // ACLK = VLO, SMCLK = MCLK = DCO
    CSCTL2 |= SELA_1 + SELS_3 + SELM_3;     // SELx_1 uses VLOCLK, SELx_3 use DCOCLK.

    // ACLK/1, SMCLK/8, MCLK/1
    CSCTL3 |= DIVA_0 + DIVS_3 + DIVM_0;     // DIVx_0 = divide by 1, DIVx_3 divides by 8.

    // Power down clocks if not used by ACLK, MCLK or SMCLK.
    CSCTL4 |= HFXTOFF_1 + LFXTOFF_1;

    // Lock clock registers.
    CSCTL0_H = 0;
}

//*************************************************************************************
void timer_start(void)
{
    // ACLK, upmode, clear.
    TA0CCR0 = 0x6978; // ~3s delay. ACLK running at 9kHz (VLO),
    //so 1/f = T for 1 increment in bits. 3 seconds/T = 27000 = 4E20 in hex.
    TA0CTL |= (TASSEL__ACLK + MC_1);
    TA0CCTL0 = CCIE; // CCR0 interrupt enabled.
    __bis_SR_register(GIE); // Enable interrupts for timeout.
}

void timer_stop(void)
{
    TA0CTL = MC_0; // Stop counting.
    TA0R = 0;      // Reset counter.
    __bic_SR_register(GIE);
}

void timerB_start (void){
    TB0CCR0 = 0x2650; // ~1s delay. ACLK running at 9kHz (VLO),
    //so 1/f = T for 1 increment in bits. 1 seconds/T = 18000 =  in hex.
    TB0CTL |= (TBSSEL__ACLK + MC_1);
    TB0CCTL0 = CCIE; // CCR0 interrupt enabled.
    __bis_SR_register(GIE);
}

void timerB_stop (void){
    TB0CTL = MC_0; // Stop counting.
    TB0R = 0;      // Reset counter.
    __bic_SR_register(GIE);
}

void wait_one_second(void){

    TB0CCR0 = 0x2650; // ~1s delay. ACLK running at 9kHz (VLO),
    //so 1/f = T for 1 increment in bits. 1 seconds/T = 18000 =  in hex.
    TB0CTL |= (TBSSEL__ACLK + MC_1);
    TB0CCTL0 = CCIE; // CCR0 interrupt enabled.
    __bis_SR_register(GIE);

    while(!timerB_exit){
        ;   // do nothing
    }
    timerB_exit = 0;
}

//*************************************************************************************
void led_set(uint8_t byte)
{
    P8OUT = (byte & 0x0F);
}

void led_clear(void)
{
    P8OUT = 0;
}

void led_flash(void)
{
    P8OUT   &=  ~(BIT1+BIT2);
    P8OUT   |=  (BIT0+BIT3);
    __delay_cycles(4e5);
    P8OUT   &=  ~(BIT0+BIT3);
    P8OUT   |=  (BIT1+BIT2);
    __delay_cycles(4e5);

}
//*************************************************************************************
inline void enter_lpm5(void)
{
    // Unlock PMM registers.
    PMMCTL0_H = 0xA5;
    // Disable SVS module.
    PMMCTL0_L &= ~(SVSHE);
    // Enter LPM4.5
    PMMCTL0_L |= PMMREGOFF;
    // Lock PMM registers.
    PMMCTL0_H = 0;
    // Enter LPM4.5
    __bis_SR_register(LPM4_bits + GIE);
}

inline void power_off(void)
{
    // Because PMOS GPIO is Active-low, power off occurs when GPIO is high.
     P2OUT |= PS_LATCH;
}

//*************************************************************************************
error_t mailbox_push(uint8_t new)
{
    uint8_t next = (mailbox.head + 1) % BUFFER_SIZE;
    if (next != mailbox.tail) {
        mailbox.buffer[mailbox.head] = new;
        mailbox.head = next;
        return ERROR_OK;
    }
    return ERROR_NOBUFS;
}

error_t mailbox_pop(uint8_t *out)
{
    if (mailbox.head == mailbox.tail) {
        return ERROR_NOBUFS;
    }
    uint8_t next = (mailbox.tail + 1) % BUFFER_SIZE;
    *out = mailbox.buffer[mailbox.tail];
    mailbox.tail = next;
    return ERROR_OK;
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
    /* Timer to count 1 seconds */

    timerB_stop();  // Stop & reset timer.
    timerB_exit = 1; // Assert exit flag.
}
