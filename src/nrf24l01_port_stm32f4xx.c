#include "main.h"
#include "nrf24l01_port.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

void nrf24l01_port_delay(uint32_t delay) { HAL_Delay(delay); }

void nrf24l01_port_select(void) {
    HAL_GPIO_WritePin(SPI1_nCS1_GPIO_Port, SPI1_nCS1_Pin, GPIO_PIN_RESET);
}

void nrf24l01_port_deselect(void) {
    HAL_GPIO_WritePin(SPI1_nCS1_GPIO_Port, SPI1_nCS1_Pin, GPIO_PIN_SET);
}

void nrf24l01_port_set_ce_low(void) {
    HAL_GPIO_WritePin(SPI1_CE1_GPIO_Port, SPI1_CE1_Pin, GPIO_PIN_RESET);
}

void nrf24l01_port_set_ce_high(void) {
    HAL_GPIO_WritePin(SPI1_CE1_GPIO_Port, SPI1_CE1_Pin, GPIO_PIN_SET);
}

void nrf24l01_port_transmit(const uint8_t *data, uint16_t len) {
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

void nrf24l01_port_transmit_byte(uint8_t data) {
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

void nrf24l01_port_transmit_receive_byte(uint8_t tx_data, uint8_t *rx_data) {
    HAL_SPI_TransmitReceive(&hspi1, &tx_data, rx_data, 1, HAL_MAX_DELAY);
}
