/**
 * @Acknowledgement: Special thanks to Rhys Thomas <rt8g15@soton.ac.uk> for
 * providing the github repository files (open source) for the zetaplus radio
 * module in C language to use with msp430 devices.
 *
 * @ github repository -> https://github.com/rhthomas/pt3p-zeta
 *
 * @modified by Piotr Krawiec <p.j.krawiec1@newcastle.ac.uk>
 * @date 02/12/2022
 *
 * @brief Setup functions to initialise general peripherals.
 *
 * Pin-out:
 * MSP430FR5994
 * -------+
 *        |
 *    P4.1|<- External comparator
 *    P2.5|-> UB20M power-gated output
 *        |
 * -------+
 *
 */

#ifndef UTIL_H
#define UTIL_H


#include <msp430.h>
#include <stdint.h>


// Connection definitions.
#define EXT_COMP (BIT1) ///< External comparator output connects to P4.1.
#define PS_LATCH (BIT5) ///< Power-supply latch output on P2.5.


// State definitions.
#define COMPARATOR_ON (P4IN & EXT_COMP) ///< Tests the state of the comparator output.
#define BUFFER_SIZE 10u ///< Number of bytes in mailbox buffer.

//*************************************************************************************

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head, tail;
} buffer_t;

typedef enum {
    ERROR_OK = 0, ERROR_NOBUFS, ERROR_TIMEOUT
} error_t;

//*************************************************************************************

/**
 * @defgroup init Initialisation
 * @brief Peripheral initialisation.
 * @{
 */
/**
 * @brief Initialise IO pins.
 *
 * Power-supply latch is driven high to keep the supply of power to the node.
 *
 */
void io_init(void);


/**
 * @brief Initialise system clock.
 *
 * | Clock | Module | Freq. |
 * |-------|--------|-------|
 * | MCLK  |   DCO  |  8MHz |
 * | SMCLK |   DCO  |  1MHz |
 * | ACLK  |   VLO  | 10kHz |
 *
 */
void clock_init(void);

//*************************************************************************************

/**
 * @defgroup timers Timers
 * @brief Control of timer module for timeout protection.
 * @{
 */

/**
 * @brief Start running the timer.
 */
inline void timer_start(void);


/**
 * @brief Stop and reset the timer.
 */
inline void timer_stop(void);

/**
 * @brief Control of timer module for 0.1 second delay packet transmission.
 */

/**
 * @brief Start running timer B
 */
inline void timerB_start (void);

/**
 * @brief Stop and reset the timer.
 */
inline void timerB_stop (void);

/**
 * @brief Delay code by 1 second (TimerB start/stop).
 */
inline void wait_one_second(void);

//*************************************************************************************

/**
 * @defgroup led LEDs
 * @brief Functions for controlling on-board LEDs.
 * @{
 */
/**
 * @brief Write byte to Port 8 LEDs.
 *
 * @param byte : Value to display.
 */
void led_set(uint8_t byte);


/**
 * @brief Clear all the on-board LEDs.
 */
void led_clear(void);


/**
 * @brief Flash the LEDs to indicate a timeout has occured.
 */
void led_flash(void);

//*************************************************************************************

/**
 * @brief Sets the necessary registers to enter LPMx.5.
 */
inline void enter_lpm5(void);


/**
 * @brief Release latch to power-supply.
 *
 * @note The latch is driven high in io_init() and is the first setup function
 * that is called, regardless of what mode of operation is running.
 */
inline void power_off(void);

//*************************************************************************************

/**
 * @defgroup mailbox Mailbox
 * @brief Functions for reading/writing data to the mailbox.
 * @{
 */
/**
 * @brief Push new data into the box.
 *
 * @param[in] new - Data to be added.
 * @return Error status.
 * @retval ERROR_OK - No errors, packet added to buffer.
 * @retval ERROR_NOBUFS - Buffer full, packet lost.
 */
error_t mailbox_push(uint8_t new);


/**
 * @brief Pop oldest data from the box.
 *
 * @param[out] out - Address to write popped data to.
 * @return Error status.
 * @retval ERROR_OK - No errors, packet popped from buffer.
 * @retval ERROR_NOBUFS - Buffer empty, Nothing to pop.
 */
error_t mailbox_pop(uint8_t *out);


#endif // UTIL_H
