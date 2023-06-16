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
 * @brief Simple SPI library for MSP430FR5994 MCU.
 *
 * Pin-out:
 * MSP430FR5994
 * -------+
 *        |
 *    P5.0|-> Data Out (to SDI)     (UCB1SIMO)
 *    P5.1|<- Data In (to SDO)      (UCB1SOMI)
 *    P5.2|-> Serial Clock Out      (UCB1CLK)
 *    P5.3|-> nSEL (Chip select)    (UCB1STE)
 *        |
 * -------+
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <msp430.h>

//#define MANUAL  ///< "Uncomment" for manual CS toggling.

#define MOSI (BIT0) ///< Master-out/Slave-in (P5.0).
#define MISO (BIT1) ///< Master-in/Slave-out (P5.1).
#define SCLK (BIT2) ///< SPI clock (P5.2).
#define CS   (BIT3) ///< Chip select pin for SPI (P5.3).

/**
 * @brief Initialises the SPI peripheral on eUSCI B1.
 *
 * SPI configuration:
 * 1. Master 3-pin mode (CS is handled manually).
 * 2. Synchronous.
 * 3. MSB first.
 * 4. Data captured on first clock edge and changed on following.
 * 5. SMCLK @ 1MHz as source.
 *
 * @ingroup init
 * @note ZETAPLUS has a max. SPI frequency of 1.4MHz, that's why
 * SMCLK is set t0 1MHz.
 */
void spi_init(void);


/**
 * @brief Begin SPI transfer.
 *
 * @param byte : Data to transmit to slave.
 * @return Data received from slave.
 */
uint8_t spi_xfer(uint8_t byte);


/**
 * @brief Set the chip select pin high.
 */
inline void spi_cs_high(void);


/**
 * @brief Set the chip select pin low.
 */
inline void spi_cs_low(void);


#endif // SPI_H
