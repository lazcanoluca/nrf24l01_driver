#ifndef NRF24L01_H
#define NRF24L01_H

#define NRF24L01_PIPE_COUNT 2

#include "nrf24l01_config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Supported RX pipes in the current driver.
 */
typedef enum {
    NRF24L01_PIPE_0,
    NRF24L01_PIPE_1,
} nrf24l01_pipe_t;

/**
 * @brief Radio operating mode.
 */
typedef enum {
    NRF24L01_MODE_RX,
    NRF24L01_MODE_TX,
} nrf24l01_mode_t;

/**
 * @brief Driver handle.
 *
 * The driver is intentionally stateless. The register file is treated as the
 * source of truth, so this handle is currently opaque and empty.
 */
typedef struct {
} nrf24l01_t;

/**
 * @brief Initialize the radio with a global configuration.
 *
 * This applies radio-wide settings such as channel, CRC, retransmit behavior,
 * RF setup, and the initial TX address.
 *
 * @param radio Driver handle.
 * @param config Global radio configuration.
 * @return `true` if initialization succeeds, `false` otherwise.
 *
 * @warning Call this before any other `nrf24l01_*` function.
 *
 * @code
 * nrf24l01_t radio;
 * nrf24l01_config_t config = {
 *     .variant = NRF24L01_VARIANT_PLUS,
 *     .channel = 108,
 *     .datarate = NRF24L01_DATARATE_250KBPS,
 *     .crc = NRF24L01_CRC_16,
 *     .pa_level = NRF24L01_PA_LOW,
 *     .retransmit_delay = NRF24L01_RETRANSMIT_DELAY_1500,
 *     .retransmit_count = 15,
 * };
 *
 * nrf24l01_init(&radio, &config);
 * @endcode
 */
bool nrf24l01_init(nrf24l01_t *radio, const nrf24l01_config_t *config);

/**
 * @brief Configure an RX pipe.
 *
 * This writes the pipe address, fixed payload width, and auto-ack setting.
 * Pipe enablement is handled separately by `nrf24l01_open_pipe()`.
 *
 * @param radio Driver handle.
 * @param pipe RX pipe to configure.
 * @param config Pipe configuration.
 * @return `true` if the configuration succeeds, `false` otherwise.
 *
 * @warning Pipe 0 is special when auto-ack is enabled.
 * Calling `nrf24l01_set_tx_address()` may also overwrite `RX_ADDR_P0` so ACK
 * behavior works correctly. If the application needs a stable local receive
 * address, prefer pipe 1. If pipe 0 is used as a normal RX pipe, its address
 * may need to be restored after changing the TX address.
 *
 * @code
 * nrf24l01_pipe_config_t pipe1 = {
 *     .address = {'N', 'O', 'D', 'E', '2'},
 *     .payload_size = 32,
 *     .auto_ack = true,
 * };
 *
 * nrf24l01_config_pipe(&radio, NRF24L01_PIPE_1, &pipe1);
 * nrf24l01_open_pipe(&radio, NRF24L01_PIPE_1);
 * @endcode
 */
bool nrf24l01_config_pipe(nrf24l01_t *radio, nrf24l01_pipe_t pipe,
                          const nrf24l01_pipe_config_t *config);

/**
 * @brief Enable a previously configured RX pipe.
 *
 * @param radio Driver handle.
 * @param pipe RX pipe to enable.
 * @return `true` if the pipe is enabled, `false` otherwise.
 *
 * @warning Configure the pipe before opening it.
 */
bool nrf24l01_open_pipe(nrf24l01_t *radio, nrf24l01_pipe_t pipe);

/**
 * @brief Disable an RX pipe.
 *
 * @param radio Driver handle.
 * @param pipe RX pipe to disable.
 * @return `true` if the pipe is disabled, `false` otherwise.
 */
bool nrf24l01_close_pipe(nrf24l01_t *radio, nrf24l01_pipe_t pipe);

/**
 * @brief Change the RX address of a single pipe.
 *
 * @param radio Driver handle.
 * @param pipe RX pipe whose address will be updated.
 * @param address New pipe address.
 * @return `true` if the address is written, `false` otherwise.
 *
 * @warning If pipe 0 auto-ack is enabled, later calls to
 * `nrf24l01_set_tx_address()` may overwrite its RX address again.
 */
bool nrf24l01_set_pipe_rx_address(
    nrf24l01_t *radio, nrf24l01_pipe_t pipe,
    const uint8_t address[NRF24L01_ADDRESS_WIDTH]);

/**
 * @brief Set the current transmit destination address.
 *
 * @param radio Driver handle.
 * @param address Destination address.
 * @return `true` if the address is written, `false` otherwise.
 *
 * @warning Call this before `nrf24l01_send()` if the application transmits.
 *
 * @warning If auto-ack is enabled on pipe 0, this function may also update
 * `RX_ADDR_P0` to keep ACK-related behavior working correctly.
 *
 * @code
 * const uint8_t remote[NRF24L01_ADDRESS_WIDTH] = {'N', 'O', 'D', 'E', '1'};
 * nrf24l01_init(&radio, &config);
 * nrf24l01_set_tx_address(&radio, remote);
 * @endcode
 */
bool nrf24l01_set_tx_address(nrf24l01_t *radio, const uint8_t *address);

/**
 * @brief Put the radio in TX mode.
 *
 * @param radio Driver handle.
 * @return `true` if the mode switch succeeds, `false` otherwise.
 *
 * @warning Call this before `nrf24l01_send()`.
 */
bool nrf24l01_set_mode_tx(nrf24l01_t *radio);

/**
 * @brief Put the radio in RX mode.
 *
 * @param radio Driver handle.
 * @return `true` if the mode switch succeeds, `false` otherwise.
 *
 * @warning Call this before using `nrf24l01_available()` or
 * `nrf24l01_recv()`.
 */
bool nrf24l01_set_mode_rx(nrf24l01_t *radio);

// /**
//  * @brief Enable or disable auto-ack on all currently supported pipes.
//  *
//  * @param radio Driver handle.
//  * @param autoack `true` to enable auto-ack, `false` to disable it.
//  * @return `true` if the register write succeeds, `false` otherwise.
//  *
//  * @warning Use this carefully if you are mixing broadcast-like traffic and
//  * reliable point-to-point traffic.
//  */
// bool nrf24l01_set_autoack_all_pipes(nrf24l01_t *radio, bool autoack);

/**
 * @brief Transmit one fixed-size payload.
 *
 * If `length` is smaller than the configured payload width, the remaining bytes
 * are zero-padded by the driver. If `length` is larger than the payload width,
 * transmission fails.
 *
 * @param radio Driver handle.
 * @param payload Payload buffer.
 * @param length Number of payload bytes to send.
 * @return `true` if the payload is transmitted successfully, `false`
 * otherwise.
 *
 * @warning Use this in TX mode.
 *
 * @code
 * const uint8_t message[32] = "Hello";
 * nrf24l01_set_mode_tx(&radio);
 * nrf24l01_send(&radio, message, sizeof(message));
 * @endcode
 */
bool nrf24l01_send(nrf24l01_t *radio, const uint8_t *payload, size_t length);

/**
 * @brief Receive one full fixed-size payload.
 *
 * The driver reads exactly one payload from the RX FIFO. If `buffer_len` is
 * smaller than the configured payload size, reception fails and no payload is
 * copied to the caller buffer.
 *
 * @param radio Driver handle.
 * @param buffer Destination buffer.
 * @param buffer_len Size of the destination buffer in bytes.
 * @return `true` if one payload is received, `false` otherwise.
 *
 * @warning Use this in RX mode and only after `nrf24l01_available()` returns
 * `true`.
 */
bool nrf24l01_recv(nrf24l01_t *radio, uint8_t *buffer, size_t buffer_len);

/**
 * @brief Check whether at least one payload is waiting in the RX FIFO.
 *
 * @param radio Driver handle.
 * @return `true` if at least one payload is available, `false` otherwise.
 *
 * @warning Use this in RX mode.
 *
 * @code
 * if (nrf24l01_available(&radio)) {
 *     uint8_t buffer[32];
 *     nrf24l01_recv(&radio, buffer, sizeof(buffer));
 * }
 * @endcode
 */
bool nrf24l01_available(nrf24l01_t *radio);

#endif /* NRF24L01_H */
