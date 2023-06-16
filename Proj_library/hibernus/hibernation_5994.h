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

Date modified by P. Krawiec: 07/04/2023.
*/

//**************************************************************************************************************
#include <msp430.h>
#include <Proj_library/h_files/t1_util.h>
//Interrupt and Restoring
// In lnk_msp430fr5994, FRAM starts at 0x4000 & ends at 0xFF80, FRAM2 starts at 0x10000 & ends at 0x40000.
// First available address to safely write data to FRAM is at 0x4150.
#define INT 0x6000
#define CHECK 0x6004

//Program Counter (PC)
#define PROGRAM_COUNTER 0x6008

//RAM to FRAM saving location
#define SAVING_RAM_LOCATION_START 0x6050    // First address to save RAM to after saving SP + MCU core.

//Location of RAM
#define RAM_END 0x2C00  // In lnk_msp430fr5994.cmd, RAM_START = 0x1C00, RAM_LENGTH = 0x1000, so RAM_END = 0x2C00.

// there is also LEA_RAM, but LEAN is not used or concerned in this project.


// Function Declarations
void Hibernus(void);
void Set_interrupt_restore (void);
void Set_interrupt_hibernate (void);
void Disable_interrupt_flag (void);
void Hibernate (void);
void Save_RAM (void);
void Save_GPR(void);
void Restore (void);
void Restore_GPR(void);
void Restore_RAM (void);

