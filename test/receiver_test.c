/* This file along with the transmitter_test.c file tests the functionality of the zetaplus
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

    // Set zeta operating mode 1 (ATM Rx)
    zeta_select_mode(0x1);

    while(1){
        //Indicate receiver function is running
        P1OUT |= BIT1;

        uint8_t incoming_packet[1u + 4u] = {0};
        uint8_t data_in = 0;

        // Receive mode: ATR - Channel, Packet Length
        zeta_rx_mode(CHANNEL, sizeof(incoming_packet) - 4u);

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

        //comment / uncomment this line for more testing.
        power_off();
    }
}
