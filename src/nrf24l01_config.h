#ifndef NRF24L01_CONFIG_H
#define NRF24L01_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief nRF24L01-family public configuration types.
 */

// RADIO LEVEL CONFIG

#define NRF24L01_ADDRESS_WIDTH 5

// typedef enum {
//     NRF24L01_ADDRESS_WIDTH_3_BYTES = 0x01,
//     NRF24L01_ADDRESS_WIDTH_4_BYTES = 0x02,
//     NRF24L01_ADDRESS_WIDTH_5_BYTES = 0x03,
// } nrf24l01_address_width_t;

/**
 * @brief Supported radio variants.
 */
typedef enum {
    NRF24L01_VARIANT_BASE,
    NRF24L01_VARIANT_PLUS,
} nrf24l01_variant_t;

/**
 * @brief Supported air data rates.
 *
 * @warning `NRF24L01_DATARATE_250KBPS` requires a plus-compatible device.
 */
typedef enum {
    NRF24L01_DATARATE_250KBPS,
    NRF24L01_DATARATE_1MBPS,
    NRF24L01_DATARATE_2MBPS
} nrf24l01_datarate_t;

/**
 * @brief CRC configuration.
 */
typedef enum {
    NRF24L01_CRC_DISABLED,
    NRF24L01_CRC_8,
    NRF24L01_CRC_16,
} nrf24l01_crc_t;

/**
 * @brief RF power amplifier level.
 */
typedef enum {
    NRF24L01_PA_MIN,
    NRF24L01_PA_LOW,
    NRF24L01_PA_HIGH,
    NRF24L01_PA_MAX,
} nrf24l01_pa_level_t;

/**
 * @brief Auto-retransmit delay configuration in microseconds.
 */
typedef enum {
    NRF24L01_RETRANSMIT_DELAY_250,
    NRF24L01_RETRANSMIT_DELAY_500,
    NRF24L01_RETRANSMIT_DELAY_750,
    NRF24L01_RETRANSMIT_DELAY_1000,
    NRF24L01_RETRANSMIT_DELAY_1250,
    NRF24L01_RETRANSMIT_DELAY_1500,
    NRF24L01_RETRANSMIT_DELAY_1750,
    NRF24L01_RETRANSMIT_DELAY_2000,
    NRF24L01_RETRANSMIT_DELAY_2250,
    NRF24L01_RETRANSMIT_DELAY_2500,
    NRF24L01_RETRANSMIT_DELAY_2750,
    NRF24L01_RETRANSMIT_DELAY_3000,
    NRF24L01_RETRANSMIT_DELAY_3250,
    NRF24L01_RETRANSMIT_DELAY_3500,
    NRF24L01_RETRANSMIT_DELAY_3750,
    NRF24L01_RETRANSMIT_DELAY_4000
} nrf24l01_retransmit_delay_t;

/**
 * @brief Global radio configuration used during initialization.
 *
 * This struct contains radio-wide settings. It does not describe RX pipe
 * configuration.
 *
 */
typedef struct {
    // nrf24l01_address_width_t address_width;
    nrf24l01_variant_t variant;
    uint8_t channel;
    nrf24l01_datarate_t datarate;
    nrf24l01_crc_t crc;
    nrf24l01_pa_level_t pa_level;
    nrf24l01_retransmit_delay_t retransmit_delay;
    // 0..15
    uint8_t retransmit_count;
} nrf24l01_config_t;

// PIPE LEVEL CONFIG

/**
 * @brief Per-pipe receive configuration.
 *
 * @warning This struct does not enable the pipe. Use `nrf24l01_open_pipe()`
 * after configuring it.
 *
 * @warning Pipe 0 is special when auto-ack is enabled because its RX address
 * may be rewritten by `nrf24l01_set_tx_address()`.
 */
typedef struct {
    uint8_t address[NRF24L01_ADDRESS_WIDTH];
    // 1..32
    uint8_t payload_size;
    bool auto_ack;
} nrf24l01_pipe_config_t;

#endif /* NRF24L01_CONFIG_H */
