#ifndef NRF24L01_PORT_H
#define NRF24L01_PORT_H

#include <stdint.h>

void nrf24l01_port_delay(uint32_t delay);

// TODO: these selects should actually chose a radio in the future, right?
void nrf24l01_port_select(void);

void nrf24l01_port_deselect(void);

void nrf24l01_port_set_ce_low(void);

void nrf24l01_port_set_ce_high(void);

void nrf24l01_port_transmit(const uint8_t *data, uint16_t len);

void nrf24l01_port_transmit_byte(uint8_t data);

void nrf24l01_port_transmit_receive_byte(uint8_t tx_data, uint8_t *rx_data);

#endif /* NRF24L01_PORT_H */
