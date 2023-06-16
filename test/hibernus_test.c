/* The test is to see if P8 OUT can keep counting in the same manner. This
 * is done by running the following code with the condition of hibernus in
 * the ISR to power off and go into LPM for debug purposes.
 *
 * Start by setting comparator input (see util.h for specific pin) high and
 * letting the code run freely. Once desired, turn comparator input off. At
 * this point, the ISR will run, save the MCU core, RAM and General Purpose
 * Registers to FRAM. The operation of the LEDs should freeze and P1 LED
 * should indicate that hibernus is still running. Remember the value of the
 * P8OUT LEDs (i.e. the pattern of LEDs on/off)
 *
 * Now click soft reset button. Once done, the software will soft reset back
 * to the beginning of main. At this point, click run. The code will freeze
 * at a point in hibernus, waiting for the input to go high again. When this
 * happens, observe the LEDs and increase the voltage until the previous LED
 * output before soft resetting are shown on the output.
 */
#include <Proj_library/hibernus/hibernation_5994.h> //hibernus by D.Balsamo (1)
#include <Proj_library/h_files/t1_util.h>   //system set up (pins, functions etc.)

volatile uint8_t timerB_exit = 0;

int main (void){

    // Disable the GPIO power-on default high-impedance mode on configured port settings.
    PM5CTL0 &= ~(LOCKLPM5);

    //  stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    //  Initialise system.
    io_init();
    clock_init();

    __bis_SR_register(GIE);
    Hibernus();

    uint8_t i;
    // "Active operation" Indicated by LED count in Binary using Port 8 Pins 0,1,2,3.
    while(1){
        for(i=0;i<16;i++){
            led_set(i);
            //__delay_cycles(8e5);

            // wait 1 second
            timerB_start();
            while(!timerB_exit){
                ;   // do nothing while timerB_exit = 0, exit on timerB_exit = 1.
            }
            timerB_exit = 0;

        }
    }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
    /* Timer to count 1 seconds */

    timerB_stop();  // Stop & reset timer.
    timerB_exit = 1; // Assert exit flag.
}
