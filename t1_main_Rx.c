#include <Proj_library/hibernus/hibernation_5994.h> //hibernus by D.Balsamo (1)
#include <Proj_library/h_files/t1_util.h>   //system set up (pins, functions etc.)
#include <Proj_library/h_files/t1_zeta.h>   //radio functions

/* (1) Hibernus was originally implemented on the msp430fr5739 platform, this
 * implementation is now available on the msp430fr5994 platform, coded by P. Krawiec.
 * This is a full implementation of hibernus - General Purpose Registers, RAM and MCU
 * Core Registers are all saved to FRAM.
 *
 * Receiver functionality:
 *
 *      Set-up Receiver
 *
 *      If there is enough voltage input at the comparator (the transmitter switches on),
 *      then run the active operation:
 *
 *              Call hibernus to{
 *                  Recover previous state (if one exists) of active operation
 *                  Set up interrupt for when comparator output goes low next.
 *              }
 *
 *              Proceed with active operation until comparator output is low.
 *              When comparator output is low{
 *                  Save state of active operation.
 *                  Turn off the latching supply to the MCU.
 *              }
 *
 *      If a wake-up packet has been sent by transmitter,
 *      then run the inactive (receiving) operation:
 *
 *              Set Zeta Radio to Receive mode{
 *              Wait to receive packet
 *              Save it to mailbox (does this save directly to NVM?!)
 *              Save this to NVM before power cuts.
 *              }
 *
 *      At this point, wait until there is again enough charge across the Energy storage
 *      supplied by the EH until the Comparator turns the MCU back on. And the process
 *      restarts again.
 */

uint8_t i;
uint8_t j;

// ***** Receive Packet ************************************************************
void receive_packet(void){
    //initialise radio
    zeta_init();

    // Set zeta operating mode 2 (ATM Ready)
    zeta_select_mode(0x2);

    //Indicate receiver function is running
    P1OUT |= BIT1;

    uint8_t incoming_packet[1u + 4u] = {0};
    uint8_t data_in = 0;

    // Receive mode: ATR - Channel, Packet Length
    zeta_rx_mode(CHANNEL, 1u);

    __delay_cycles(100);

    if(zeta_rx_packet(incoming_packet)){
        ;
    }
    else{
        mailbox_push(incoming_packet[4]);
        mailbox_pop(&data_in);
        led_set(data_in);

        // Take data contents of the packet and display them
        wait_one_second();

        // clear output
        led_clear();
    }

    if(!COMPARATOR_ON){
        power_off();
    }
}

// ***** Main Program *******************************************************************

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

    if(!COMPARATOR_ON){
        receive_packet();
    }
}
