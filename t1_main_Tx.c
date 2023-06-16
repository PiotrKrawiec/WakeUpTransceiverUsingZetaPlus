#include <Proj_library/hibernus/hibernation_5994.h> //hibernus by D.Balsamo (1)
#include <Proj_library/h_files/t1_util.h>   //system set up (pins, functions etc.)
#include <Proj_library/h_files/t1_zeta.h>   //radio functions

/* (1) Hibernus was originally implemented on the msp430fr5739 platform, this
 * implementation is now available on the msp430fr5994 platform, coded by P. Krawiec.
 * This is a full implementation of hibernus - General Purpose Registers, RAM and MCU
 * Core Registers are all saved to FRAM.
 *
 * Transmitter functionality:
 *
 *      Set-up Transmitter
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
 *                  Then proceed to the inactive operation
 *              }
 *
 *      If there isn't enough voltage input at the comparator,
 *      then run the inactive (transmitting) operation:
 *
 *              Set Zeta Radio to Transmitt mode{
 *                  Transmit a dummy packet to wake up the receiver
 *                  Wait enough time for the receiver to wake up
 *                  Transmit the intended data packet to receiver
 *                  Turn off the power to MCU.
 *              }
 *
 *      At this point, wait until there is again enough charge across the Energy storage
 *      supplied by the EH until the Comparator turns the MCU back on. And the process
 *      restarts again.
 */

uint8_t i;
uint8_t j;
//***** Active operation ***********************************************************
void active_operation(void)
{
    // Hibernus
    //__bis_SR_register(GIE);
    Hibernus();

    // "Active operation" Indicated by LED count in Binary using Port 8 Pins 0,1,2,3.
    while(COMPARATOR_ON){
        for(i=0;i<16;i++){
            led_set(i);
            wait_one_second();
        }
    }
}

// ***** Transmit Packet ***********************************************************
void transmit_packet(void){
    //initialise radio.
    zeta_init();

    // Turning off Comparator Interrupt to prevent isr_trap.asm
    P4IE &= ~(EXT_COMP);

    // For debugging, Indicate function is running by setting P1.0 on.
    P1OUT |= BIT1;
    led_clear();    // clear previous active operation LEDs.

    //wait 5 seconds
    for(i=0;i<5;i++){
        wait_one_second();
    }

    // Set zeta operating mode 2 for transmitting (ATM READY)
    zeta_select_mode(0x2);

    uint8_t data = 0x1;

    for(i=0;i<16;i++){

        /* Transmit dummy packet with data value 0 (i.e. nothing important) in it as a wake up signal to Rx!
         * Transmit Mode: ATS - specify the channel & Packet Length
         * (Packet length is in 8 bit bytes - i.e PL(1) = 0xFF, PL(2) = 0xFFFF, etc) */
        zeta_send_open(CHANNEL,1u);
        zeta_write_byte('0');
        zeta_send_close();
        led_set(0x0F);

        // Wait 2 seconds for wake up packet to turn on & configure MCU-radio for packet to be received.
        wait_one_second();
        led_clear();
        wait_one_second();

        // Prepare data packet to be sent
        uint8_t data_out = data + 0x21;         // offset by hex 21 for ascii format
        uint8_t write_out[1u] = {data_out};

        // Transmit Data packet
        zeta_send_open(CHANNEL,1u);
        zeta_write_byte(write_out[0]);
        zeta_send_close();
        led_set(data);

        // Wait 10 seconds to indicate if packet received and to shut down Rx.
        for(j=0;j<2;j++){
            wait_one_second();
        }

        led_clear();

        for(j=0;j<8;j++){
            wait_one_second();
        }

        // Clear led output
        data = data + 0x01;
    }

    if(!COMPARATOR_ON){
        power_off();
    }
}

// ***** Main Program **************************************************************

int main(void)
{
    // Disable the GPIO power-on default high-impedance mode on configured port settings.
    PM5CTL0 &= ~(LOCKLPM5);

    //  stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //  Initialise system.
    io_init();
    clock_init();
    spi_init();

    active_operation();

    if(!COMPARATOR_ON){
        transmit_packet();
    }
}
