/*
Originally Hibernus was created for use on MSP430FR5739.

This is modified by P. Krawiec for use on MSP430FR5994:

Hibernus: Software-based approach to intelligently hibernate and restore the system's state
in response to a power failure. This software exploits an external comparator.

Citation: If you are using this code for your research please cite:

[1] D. Balsamo, A. S. Weddell, G. V. Merrett, B. M. Al-Hashimi, D. Brunelli and L. Benini,
    "Hibernus: Sustaining Computation During Intermittent Supply for Energy-Harvesting Systems,"
    in IEEE Embedded Systems Letters, vol. 7, no. 1, pp. 15-18, March 2015.

[2] D. Balsamo; A. Weddell; A. Das; A. Arreola; D. Brunelli; B. Al-Hashimi; G. Merrett; L. Benini,
    "Hibernus++: A Self-Calibrating and Adaptive System for Transiently-Powered Embedded Devices,"
    in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems , vol.PP, no.99, pp.1-1

This research has been supported by the University of Southampton and the University of Bologna.
Copyright 2016, Domenico Balsamo, All rights reserved.
Date: 25/05/2016

Date modified by P. Krawiec, researching in Newcastle Univeristy: 07/04/2023.
*/

//******************************************************************************************************
#include <Proj_library/hibernus/hibernation_5994.h>
#include <Proj_library/h_files/t1_util.h>

#pragma SET_DATA_SECTION(".fram_vars")

unsigned long int *FRAM_write_ptr = (unsigned long int *) SAVING_RAM_LOCATION_START; //pointer for FRAM
unsigned long int *RAM_copy_ptr = (unsigned long int *) RAM_START; //pointer that points the RAM
unsigned long int *FLAG_interrupt = (unsigned long int *) INT; //Flag for Interrupt
unsigned long int *CC_Check = (unsigned long int *) CHECK; //Flag for Restoring

// These pointers and variable are used to set the PC
unsigned long int *PC_in_FRAM = (unsigned long int *) PROGRAM_COUNTER; //pointer for PC
unsigned long int* current_SP;

// Array to restore state of registers
unsigned int gpr_data[514];

unsigned int *Reg_copy_ptr;

int pro;
int t;

#pragma SET_DATA_SECTION()

const unsigned int gpr_locations[514] = {
    /*Special Function Registers*/
    0x100, 0x102, 0x104,
    /*PMM*/
    0x120, 0x12a, 0x130,
    /*FRAM controller A*/
    0x140, 0x144, 0x146,
    /*CRC16*/
    0x150, 0x152, 0x154, 0x156,
    /*RAM controller*/
    0x158,
    /*watchdog*/
    0x15c,
    /*CS registers*/
    0x160, 0x162, 0x164, 0x168, 0x16a, 0x16c,
    /*Sys registers*/
    0x180, 0x186, 0x188, 0x18a, 0x18c, 0x18e, 0x19a, 0x19c, 0x19e,
    /*Shared reference*/
    0x1b0,
    /*Port 1*/
    0x200, 0x202, 0x204, 0x206, 0x20a, 0x20c, 0x20e, 0x216, 0x218, 0x21a, 0x21c,
    /*Port 2*/
    0x201, 0x203, 0x205, 0x207, 0x20b, 0x20d, 0x217, 0x21e, 0x219, 0x21b, 0x21d,
    /*Port 3*/
    0x220, 0x222, 0x224, 0x226, 0x22a, 0x22c, 0x22e, 0x236, 0x238, 0x23a, 0x23c,
    /*Port 4*/
    0x221, 0x223, 0x225, 0x227, 0x22b, 0x22d, 0x237, 0x23e, 0x239, 0x23b, 0x23d,
    /*Port 5*/
    0x240, 0x242, 0x244, 0x246, 0x24a, 0x24c, 0x24e, 0x256, 0x258, 0x25a, 0x25c,
    /*Port 6*/
    0x241, 0x243, 0x245, 0x247, 0x24b, 0x24d, 0x257, 0x25e, 0x259, 0x25b, 0x25d,
    /*Port 7*/
    0x260, 0x262, 0x264, 0x266, 0x26a, 0x26c, 0x26e, 0x276, 0x278, 0x27a, 0x27c,
    /*Port 8*/
    0x261, 0x263, 0x265, 0x267, 0x26b, 0x26d, 0x277, 0x27e, 0x279, 0x27b, 0x27d,
    /*Port J*/
    0x320, 0x322, 0x324, 0x326, 0x32a, 0x32c, 0x336,
    /*TA0*/
    0x340, 0x342, 0x344, 0x346, 0x350, 0x352, 0x354, 0x356, 0x360, 0x36e,
    /*TA1*/
    0x380, 0x382, 0x384, 0x386, 0x390, 0x392, 0x394, 0x396, 0x3a0, 0x3ae,
    /*TB0*/
    0x3c0, 0x3c2, 0x3c4, 0x3c6, 0x3c8, 0x3ca, 0x3cc, 0x3ce, 0x3d0, 0x3d2, 0x3d4,
    0x3d6, 0x3d8, 0x3da, 0x3dc, 0x3de, 0x3e0, 0x3ee,
    /*TA2*/
    0x400, 0x402, 0x404, 0x410, 0x412, 0x420, 0x42e,
    /*Capacitive Touch I/O 0 registers*/
    0x430,
    /*TA3*/
    0x440, 0x442, 0x444, 0x450, 0x452, 0x454, 0x460, 0x46e,
    /*Capacitive Touch I/O 1 registers*/
    0x470,
    /*RTC_C Registers*/
    0x4a0, 0x4a1, 0x4a2, 0x4a3, 0x4a4, 0x4a6, 0x4a8, 0x4aa, 0x4ac, 0x4ad, 0x4ae,
    0x450, 0x451, 0x452, 0x453, 0x454, 0x455, 0x456, 0x458, 0x459, 0x45a, 0x45b,
    0x45c, 0x45e,
    /*32-Bit Hardware Multiplier*/
    0x4c0, 0x4c2, 0x4c4, 0x4c6, 0x4c8, 0x4ca, 0x4cc, 0x4ce, 0x4d0, 0x4d2, 0x4d4,
    0x4d6, 0x4d8, 0x4da, 0x4dc, 0x4de, 0x4e0, 0x4e2, 0x4e4, 0x4e6, 0x4e8, 0x4ea,
    0x4ec,
    /*DMA general control*/
    0x500, 0x502, 0x504, 0x506, 0x508, 0x50e,
    /*DMA Channel 0*/
    0x510, 0x512, 0x514, 0x516, 0x518, 0x51a,
    /*DMA Channel 1*/
    0x520, 0x522, 0x524, 0x526, 0x528, 0x52a,
    /*DMA Channel 2*/
    0x530, 0x532, 0x534, 0x536, 0x538, 0x53a,
    /*DMA Channel 3*/
    0x540, 0x542, 0x544, 0x546, 0x548, 0x54a,
    /*DMA Channel 4*/
    0x550, 0x552, 0x554, 0x556, 0x558, 0x55a,
    /*DMA Channel 5*/
    0x560, 0x562, 0x564, 0x566, 0x568, 0x56a,
    /*MPU Control Registers*/
    0x5a0, 0x5a2, 0x5a4, 0x5a6, 0x5a8, 0x5aa, 0x5ac, 0x5ae,
    /*eUSCI_A0 Registers*/
    0x5c0, 0x5c2, 0x5c6, 0x5c7, 0x5c8, 0x5ca, 0x5cc, 0x5ce, 0x5d0, 0x5d2, 0x5d3,
    0x5da, 0x5dc, 0x5de,
    /*eUSCI_A1 Registers*/
    0x5e0, 0x5e2, 0x5e6, 0x5e7, 0x5e8, 0x5ea, 0x5ec, 0x5ee, 0x5f0, 0x5f2, 0x5f3,
    0x5fa, 0x5fc, 0x5fe,
    /*eUSCI_A2 Registers*/
    0x600, 0x602, 0x606, 0x607, 0x608, 0x60a, 0x60c, 0x60e, 0x610, 0x612, 0x613,
    0x61a, 0x61c, 0x61e,
    /*eUSCI_A3 Registers*/
    0x620, 0x622, 0x626, 0x627, 0x628, 0x62a, 0x62c, 0x62e, 0x630, 0x632, 0x633,
    0x63a, 0x63c, 0x63e,
    /*eUSCI_B0 Registers*/
    0x640, 0x642, 0x646, 0x647, 0x648, 0x64a, 0x64c, 0x64e, 0x654, 0x656, 0x658,
    0x65a, 0x65c, 0x65e, 0x660, 0x66a, 0x66c, 0x66e,
    /*eUSCI_B1 Registers*/
    0x680, 0x682, 0x686, 0x687, 0x688, 0x68a, 0x68c, 0x68e, 0x694, 0x696, 0x698,
    0x69a, 0x69c, 0x69e, 0x6a0, 0x6aa, 0x6ac, 0x6ae,
    /*eUSCI_B2 Registers*/
    0x6c0, 0x6c2, 0x6c6, 0x6c7, 0x6c8, 0x6ca, 0x6cc, 0x6ce, 0x6d4, 0x6d6, 0x6d8,
    0x6da, 0x6dc, 0x6de, 0x6e0, 0x6ea, 0x6ec, 0x6ee,
    /*eUSCI_B3 Registers*/
    0x700, 0x702, 0x706, 0x707, 0x708, 0x70a, 0x70c, 0x70e, 0x714, 0x716, 0x718,
    0x71a, 0x71c, 0x71e, 0x720, 0x72a, 0x72c, 0x72e,
    /*TA4*/
    0x7c0, 0x7c2, 0x7c4, 0x7d0, 0x7d2, 0x7d4, 0x7e0, 0x7ee,
    /*ADC12_B Registers*/
    0x800, 0x802, 0x804, 0x806, 0x808, 0x80a, 0x80c, 0x80e,
    0x810, 0x812, 0x814, 0x816, 0x818,
    0x820, 0x822, 0x824, 0x826, 0x828, 0x82a, 0x82c, 0x82e,
    0x830, 0x832, 0x834, 0x836, 0x838, 0x83a, 0x83c, 0x83e,
    0x840, 0x842, 0x844, 0x846, 0x848, 0x84a, 0x84c, 0x84e,
    0x850, 0x852, 0x854, 0x856, 0x858, 0x85a, 0x85c, 0x85e,
    0x860, 0x862, 0x864, 0x866, 0x868, 0x86a, 0x86c, 0x86e,
    0x870, 0x872, 0x874, 0x876, 0x878, 0x87a, 0x87c, 0x87e,
    0x880, 0x882, 0x884, 0x886, 0x888, 0x88a, 0x88c, 0x88e,
    0x890, 0x892, 0x894, 0x896, 0x898, 0x89a, 0x89c, 0x89e,
    /*Comparator_E Registers*/
    0x8c0, 0x8c2, 0x8c4, 0x8c6, 0x8cc, 0x8ce,
    /*CRC32 Registers - Where 'reserved' in datasheet, value not copied.*/
    0x980, 0x986, 0x988, 0x98a, 0x99c, 0x99e, 0x9a0, 0x9a6, 0x9a8, 0x9ae,
    /*AES Accelerator Registers - Where 'reserved' in datasheet, value not copied.*/
    0x9c0, 0x9c4, 0x9c6, 0x9c8, 0x9ca, 0x9cc, 0x9ce
};

unsigned int i; // used in loops.

//******************************************************************************************************
/*
 * In this project, an external comparator is used to determine whether the system
 * has enough volts to complete an active operation task (such as sampling data),
 * or if it should transmit/wait to receive packets of data. Interrupts are used to
 * achieve this, triggering on rising/falling edges when the output of the external
 * comparator changes.
 *
 * Input:
 * P4.1 - External comparator.
 *
 * Debug LEDs:
 * P1.1 - Indicates System active (Initialising, then running main code).
 *
 * Active Operation LEDs (counting up in binary):
 * P8.0 - LED0
 * P8.1 - LED1
 * P8.2 - LED2
 * P8.3 - LED3
 */

//******************************************************************************************************

void Hibernus(void){
    //For debugging: Hibernus/Interrupt Active
    P1OUT |= BIT0;

    /* If the System has been run more than once, but has nothing to recover, indicate error,
    * for a few seconds, then force system to set up the interrupt to hibernate. */
    if(*CC_Check == 0){
        //For debugging: Indicating restoring error with Port 8 LEDs (5 times flash).
        P8OUT   &=  0x0F;
        for (i = 0; i < 5; i++) {
            led_flash();
        }
        *CC_Check = 2;             // Forcing system to get ready for hibernating procedure.
    }

    /* Get ready for hibernating. Conditions triggered:
    *   - On first time boot there's nothing to restore, *CC_Check is set to 0xFFFF.
    *   - When the system incorrectly saves the state, *CC_Check is set to 0x0010 */
    if ((*CC_Check != 0) && (*CC_Check != 1)){
        *CC_Check = 0;

        if (!COMPARATOR_ON){
                *FLAG_interrupt = 1;
                Set_interrupt_restore();
                __bis_SR_register(LPM4_bits+GIE);   //Enter LPM4 with interrupts enabled (for debug)
                __no_operation();                   // For debug
            }

        *FLAG_interrupt = 2;
        Set_interrupt_hibernate();
        __bis_SR_register(GIE);        // Set interrupt
        __no_operation();
    }

    // If there is a previous state, then restore
    if (*CC_Check == 1){
        Restore();
    }

    //For debugging: Hibernus/Interrupt Inactive
    P1OUT &= ~BIT0;
}

//******************************************************************************************************

void Set_interrupt_restore (void)
{
    P4IFG   =   0;              // Clear P4 interrupt flag.
    P4IES   &=  ~(BIT1);        // Flag is set from low to high transition (rising edge).
    P4IE    |=  BIT1;           // P4.1 interrupt enabled.
    P4IFG   =   0;              // Clear P4 interrupt flag.
}

//******************************************************************************************************

void Set_interrupt_hibernate (void)
{
    P4IFG   =   0;              // Clear P4 interrupt flag.
    P4IES   |=  BIT1;           // Trigger P4.1 interrupt on high to low transition (falling edge)
    P4IE    |=  BIT1;           // P4.1 interrupt enabled.
    P4IFG   =   0;              // Clear P4 interrupt flag.
}

void Disable_interrupt_flag (void){
    P4IFG   =   0;
    P4IE    &=  ~BIT1;
}

//******************************************************************************************************

void Hibernate (void){

	*CC_Check=0;

    // Save Core registers to FRAM
    // These increment in 4 bytes. The first register R0 is actually the PC.
	asm(" MOVA R1,&0x600C");
    asm(" MOVA R2,&0x6010");
    asm(" MOVA R3,&0x6014");
    asm(" MOVA R4,&0x6018");
    asm(" MOVA R5,&0x601C");
    asm(" MOVA R6,&0x6020");
    asm(" MOVA R7,&0x6024");
    asm(" MOVA R8,&0x6028");
    asm(" MOVA R9,&0x602C");
    asm(" MOVA R10,&0x6030");
    asm(" MOVA R11,&0x6034");
    asm(" MOVA R12,&0x6038");
    asm(" MOVA R13,&0x603C");
    asm(" MOVA R14,&0x6040");
    asm(" MOVA R15,&0x6044");

    // Saving Program Counter (PC)
    current_SP = (void*) _get_SP_register();
    *PC_in_FRAM= *current_SP;

    // Copy all the RAM and Registers onto the FRAM
    Save_RAM();

    pro=0;

    Save_GPR();

    *CC_Check = 1;
}

//******************************************************************************************************

void Save_RAM (void){

	FRAM_write_ptr= (unsigned long int *) SAVING_RAM_LOCATION_START;
	RAM_copy_ptr= (unsigned long int *) RAM_START;

	// copy all RAM onto the FRAM
	while(RAM_copy_ptr < (unsigned long int *) (RAM_END)){
	    *FRAM_write_ptr++ = *RAM_copy_ptr++;
	}
}

//******************************************************************************************************

void Save_GPR(void)
{
    for (i = 0; i < 514; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        gpr_data[i] = *Reg_copy_ptr;
    }
}

//******************************************************************************************************

void Restore (void){

    Restore_GPR();

    // Restore Core Registers
    asm(" MOVA &0x600C,R1");
    asm(" MOVA &0x6010,R2");
    asm(" MOVA &0x6014,R3");
    asm(" MOVA &0x6018,R4");
    asm(" MOVA &0x601C,R5");
    asm(" MOVA &0x6020,R6");
    asm(" MOVA &0x6024,R7");
    asm(" MOVA &0x6028,R8");
    asm(" MOVA &0x602C,R9");
    asm(" MOVA &0x6030,R10");
    asm(" MOVA &0x6034,R11");
    asm(" MOVA &0x6038,R12");
    asm(" MOVA &0x603C,R13");
    asm(" MOVA &0x6040,R14");
    asm(" MOVA &0x6044,R15");

    *current_SP = *PC_in_FRAM;

    Restore_RAM();

    /* If debugging reaches this next line, it will mean the restoration was not successful.
     * In this case all that is required is to set CC_Check to 0 to indicate that it is not
     * the first time hibernus has ran and the previous restoration was unsuccessful, set pro
     * to 1 to indicate unsuccessful restoration and setting the flag interrupt to 2 so that
     * the node can set the next intterupt to be for hibernation (from high to low trigger).*/

    // Setting interrupt for next hibernation.
    *FLAG_interrupt=2;
    Set_interrupt_hibernate();

    *CC_Check=0;
    pro=1;

    __bis_SR_register(GIE);     //inetrrupts enabled
    __no_operation();           // For debug

}

//******************************************************************************************************

void Restore_GPR(void)
{
    /* Some register addresses MUST BE skipped during restoring, these are:
     * PMM0CTRL0    (0x120)     gpr_loc[3]
     * FRAMCTRL0    (0x140)     gpr_loc[6]
     * Watchdog     (0x15c)     gpr_loc[14]
     * CSCTL0       (0x160)     gpr_loc[15]
     * MPUCTRL0     (0x5a0)     gpr_loc[270]
     *
     * This is because these registers unlock writting capability using the higher bits of the register.
     * If these registers are written with any other value, a PUC is generated (Power-up Clear, i.e. a
     * reset), therefore these values should be manually written and not copied and paste-d!).
     *
     * Other registers that are skipped are:
     * P3OUT        (0x222)     gpr_loc[54]
     * P3DIR        (0x224)     gpr_loc[55]
     *
     * P3 is dedicated to Zeta+ pins, these must not be altered so that zeta+ doesn't get triggered.
     */

    // Unlock registers.
    MPUCTL0_H = 0xA5;
    PMMCTL0_H = 0xA5;
    FRCTL0_H = 0xA5;
    CSCTL0_H = 0xA5;

    // Restore registers.
    for (i = 0; i < 3; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    for (i = 4; i < 6; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    for (i = 7; i < 14; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    for (i = 16; i < 54; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    for (i = 56; i < 270; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    for (i = 271; i < 514; i++) {
        Reg_copy_ptr = (unsigned int *)gpr_locations[i];
        *Reg_copy_ptr = gpr_data[i];
    }

    // Lock registers.
    MPUCTL0_H = 0x01;
    PMMCTL0_H = 0x01;
    FRCTL0_H = 0x01;
    CSCTL0_H = 0x01;
}

//******************************************************************************************************

void Restore_RAM (void){

    FRAM_write_ptr= (unsigned long int *) SAVING_RAM_LOCATION_START;
    RAM_copy_ptr= (unsigned long int *) RAM_START;

    //Copy RAM values in FRAM back into RAM.
     while(RAM_copy_ptr < (unsigned long int *) (RAM_END)) {

         *RAM_copy_ptr++=*FRAM_write_ptr++;
     }
}

//******************************************************************************************************

#pragma vector=PORT4_VECTOR
__interrupt void PORT4_ISR(void)
{
    switch (__even_in_range(P4IV, P4IV_P4IFG1)) {
    case P4IV_P4IFG1:

        //For debugging: Hibernus/Interrupt Inactive
        P1OUT |= BIT0;

        Disable_interrupt_flag();

        //Hibernate interrupt
        if (*FLAG_interrupt == 2){
            Hibernate();

            if(pro==0){
                t = COMPARATOR_ON;
                if (t == 0){
                    *FLAG_interrupt = 4;
                    /* If tx, comment the rest of the if statement, if rx, uncomment.
                     * LPM4 is used for debugging/demonstrating! */
                    //power_off();

                    //__bis_SR_register(LPM4_bits);   // Set interrupt and enter LPM4.
                    //__no_operation();
                }
                else{
                    *FLAG_interrupt = 2;
                    Set_interrupt_hibernate();
                    /* There's a chance the interrupt was stuck on the active operation
                     * loop. Must initialise the counting of the timer again to prevent
                     * being stuck forever in the loop. */

                    // Stop timerB.
                    TB0CTL = MC_0; // Stop counting.
                    TB0R = 0;      // Reset counter.
                    __bic_SR_register(GIE);

                    timerB_start();
                    __bis_SR_register(GIE);             // Set interrupt
                    __no_operation();
                }
            }
        }

        pro=0;
        //for debug
        __bic_SR_register_on_exit(LPM4_bits);

        //For debugging: Hibernus/Interrupt Inactive
        P1OUT &= ~BIT0;
        break;
    default:
        break;
    }
}
