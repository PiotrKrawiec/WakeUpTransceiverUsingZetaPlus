/**
 * @Acknowledgement: Special thanks to Rhys Thomas <rt8g15@soton.ac.uk> for providing the github repository files (open source) for the zeta plus radio module in C language to use with msp430 devices.
 * @ github repository -> https://github.com/rhthomas/pt3p-zeta
 *
 * @modified by Piotr Krawiec <p.j.krawiec1@newcastle.ac.uk>
 * @date 02/12/2022
 *
 * @brief Library for interfacing with ZetaPlus radio module.
 *
 * Pin-out:
 * MSP430FR5994
 * -------+
 *        |
 * Zeta+: |
 *    P3.5|<- Interrupt request (nIRQ)
 *    P3.4|-> Shutdown          (SDN)
 *        |
 * -------+
 *
 */

#ifndef ZETA_H
#define ZETA_H

#include <stdint.h>
#include <msp430.h>
#include <Proj_Library/h_files/t1_spi.h>
#include <Proj_Library/h_files/t1_util.h>
/**
 * @brief Shutdown pin (P3.4).
 *
 * | SDN | Radio  |
 * |-----|--------|
 * | 0   | Active |
 * | 1   | Shutdown (no register retention) |
 */
#define SDN (BIT4)

/**
 * @brief Interrupt request input (P3.5).
 *
 * @note Active low.
 */
#define IRQ (BIT5)

/**
 * @brief Channel of operation, 250kHz increments.
 *
 * We can use this to move closer to a different frequency band that the UB20 antennas are
 * tuned to.
 *
 * frequency = 869.50\mbox{MHz} + (250\mbox{kHz} * Y), we don't require any change,
 * so leaving Y Value as 0.
 */
#define CHANNEL (0u)

/**
 * @brief Reverses the order of a byte.
 *
 * Called within zeta_sync_byte().
 *
 * @param[in] byte : Byte to flip.
 * @return Reversed byte.
 * @see zeta_sync_byte()
 */
uint8_t reverse(uint8_t byte);

/**
 * @brief Setup the device.
 *
 * 1. Sets up I/O pins. SDN low to keep the radio active.
 * 2. Waits for radio to be ready for data.
 * 3. Host baud = 57.6kbps
 * 4. RF baud = 500kbps
 * 5. Maximum RF output power.
 * 6. Set unused sync bytes `{AA, AA, AA, AA}`.
 *
 * @ingroup init
 * @pre SPI initialisation function (spi_init()) must be called before initialising the
 * radio.
 */
void zeta_init(void);

/**
 * @brief Wait for interrupt from CODEC (data ready).
 *
 * @return Any errors while polling nIRQ pin.
 * @retval ERROR_OK - No errors.
 * @retval ERROR_TIMEOUT - Receive timeout, perhaps false wake-up.
 */
error_t zeta_wait_irq(void);

/**
 * @brief Wait until device is ready for another command.
 */
void zeta_ready(void);

//***** ATx Commands ************************************************************************

/**
 * @defgroup config Radio Configuration
 * @brief Functions to configure the radio settings (i.e. channel, mode etc).
 * @{
 */

/**
 * @brief Set mode of the device [ATM].
 *
 * @param[in] mode : Operating mode of the radio.
 *  | Value | Name  | Description |
 *  |-------|-------|-------------|
 *  | 1     | RX    | ZetaPlus enters RX mode using last RX configured settings.     |
 *  | 2     | Ready | Low power awake state for fast switching between RX/TX (<1ms). |
 *  | 3     | Sleep | Low power sleep mode with register retention.                  |
 */
void zeta_select_mode(uint8_t mode);

/**
 * @brief Receive mode configuration [ATR].
 *
 * @param[in] ch : Channel.
 * @param[in] pLength : Packet length.
 */
void zeta_rx_mode(uint8_t ch, uint8_t pLength);

/**
 * @brief Set sync bytes and addressing after preamble [ATA].
 *
 * These appear directly after the preamble in the Tx'd RF data. Can be used as
 * a form of addressing. If the bytes are set, the radio will only pass on data
 * which contains the correct sync bytes.
 *
 * @param[in] sync1 : Byte 1.
 * @param[in] sync2 : Byte 2.
 * @param[in] sync3 : Byte 3.
 * @param[in] sync4 : Byte 4.
 * @note Set byte to `0xAA` if not required.
 * @note Reverse order bytes, i.e. `2B`=`D4`, `D4`=`2B`.
 */
void zeta_sync_byte(uint8_t sync1, uint8_t sync2, uint8_t sync3, uint8_t sync4);

/**
 * @brief Set baud-rate between radio and MCU [ATH].
 *
 * @param[in] baud : Data rate between ZetaPlus and MCU.
 *  | `baud` | Data Rate |
 *  |--------|-----------|
 *  | 0      | 9.6kbps   |
 *  | 1      | 19.2kbps (default) |
 *  | 2      | 28.8kbps  |
 *  | 3      | 38.4kbps  |
 *  | 4      | 57.6kbps  |
 */
void zeta_set_baud_host(uint8_t baud);

/**
 * @brief Set baud-rate of RF [ATB].
 *
 * @param[in] baud : RF data rate, must be at least double host baud.
 *  | `baud` | RF Rate   |
 *  |--------|-----------|
 *  | 1      | 4.8kbps   |
 *  | 2      | 9.6kbps   |
 *  | 3      | 38.4kbps  |
 *  | 4      | 128.0kbps |
 *  | 5      | 256.0kbps |
 *  | 6      | 500.0kbps |
 * @note SDN must be toggled high/low for setting to take effect (~15ms).
 * @warning Higher RF baud rate increases BER at longer range.
 */
void zeta_set_baud_rf(uint8_t baud);

/**
 * @brief Set RF output power [ATP].
 *
 * @param[in] pwr : RF TX output power (1-127).
 * @note Not a linear function. The output power varies depending on factors
 *  such as supply voltage, impedance miss-match etc. Adjustment resolution of
 *  the TX output power is very fine (step size < 0.1dB) when operating near
 *  max. power setting, but becomes coarser as output level is reduced.
 */
void zeta_set_rf_power(uint8_t pwr);

/**
 * @brief Enable data error checking [ATE].
 *
 * @param[in] en : Enable CRC error checking.
 *  | | |
 *  |-|-|
 *  | 0    | Disable error checking (default) |
 *  | 1    | Enable error checking            |
 */
void zeta_enable_crc(uint8_t en);

/**
 * @brief Reset the radio to its power on defaults [ATD].
 */
void zeta_reset_default(void);

/** @} */


/**
 * @brief Get RSSI from receiver [ATQ].
 *
 * @return Received signal strength (0-255).
 * @ingroup debug
 */
uint8_t zeta_get_rssi(void);


/**
 * @brief Get version of radio firmware.
 *
 * @note To see output, attach Saleae.
 * @warning Potential race condition with !(P1IN & IRQ) poll.
 * @ingroup debug
 */
void zeta_get_vers(void);


/**
 * @brief Get current radio configuration.
 *
 * Returns `#?12345678` where:
 * 1. Operating mode (Rx = 1, Ready = 2).
 * 2. RF baud rate (1-6).
 * 3. RF output power (1-127).
 * 4. Sync byte 1.
 * 5. Sync byte 2.
 * 6. Sync byte 3.
 * 7. Sync byte 4.
 * 8. Channel number (0-15).
 *
 * @param[out] settings - Address of array to store the read settings.
 * @ingroup debug
 */
void zeta_get_settings(uint8_t *settings);

//***** Tx Transmission Commands ***********************************************************

/**
 * @defgroup tx Transmission
 * @brief Main API calls for transmitting packets.
 * @{
 */
/**
 * @brief Transmit mode configuration [ATS].
 *
 * @param[in] ch : Channel to transmit on.
 * @param[in] pLength : Length of packet.
 * @see CHANNEL.
 */
void zeta_send_open(uint8_t ch, uint8_t pLength);


/**
 * @brief Write byte to TX buffer (to send).
 *
 * @param[in] data : Data byte to send.
 */
void zeta_write_byte(uint8_t data);


/**
 * @brief Send packet closing.
 */
void zeta_send_close(void);


/**
 * @brief Send byte packet over radio.
 *
 * @param[in] packet : Pointer to byte packet to send.
 * @param[in] len : Length of packet.
 */
void zeta_send_packet(uint8_t *packet, uint8_t len);

//***** Rx Transmission Commands ***********************************************************

/**
 * @defgroup rx Reception
 * @brief Main API calls for receiving packets.
 * @{
 */
/**
 * @brief Read one byte from FIFO only.
 *
 * @param[out] out - Address to write read byte to.
 * @return Any errors during read.
 * @retval ERROR_OK - No errors.
 * @retval ERROR_TIMEOUT - Receive timeout, perhaps false wake-up.
 */
error_t zeta_read_byte(uint8_t *out);


/**
 * @brief Read packet from FIFO loop until empty.
 *
 * * '#' - Shows the start of a new packet.
 * * 'R' - Shows the start of a new packet.
 * * Length - Length of received packet.
 * * RSSI - Received signal strength indicator.
 * * Packet[Length] - The received packet.
 *
 * @param[out] packet : Array to return Rx'd packet to.
 * @retval ERROR_OK - No errors.
 * @retval ERROR_TIMEOUT - Receive timeout, perhaps false wake-up.
 */
error_t zeta_rx_packet(uint8_t *packet);


#endif // ZETA_H
