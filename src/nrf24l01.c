#include "nrf24l01.h"
#include "nrf24l01_port.h"

// COMMANDS
// Datasheet section 8.3.1: SPI commands
#define NRF24_CMD_R_REGISTER 0x00 // 000X XXXX
#define NRF24_CMD_W_REGISTER 0x20 // 001X XXXX
#define NRF24_CMD_R_RX_PAYLOAD 0x61
#define NRF24_CMD_W_TX_PAYLOAD 0xA0
#define NRF24_CMD_FLUSH_TX 0xE1
#define NRF24_CMD_FLUSH_RX 0xE2
#define NRF24_CMD_NOP 0xFF

// REGISTERS
// Datasheet section 9.1: Register map table

// REGISTER ADDRESSES
typedef enum
{
    NRF24_REG_CONFIG = 0x00,
    NRF24_REG_EN_AA = 0x01,
    NRF24_REG_EN_RXADDR = 0x02,
    NRF24_REG_SETUP_AW = 0x03,
    NRF24_REG_SETUP_RETR = 0x04,
    NRF24_REG_RF_CH = 0x05,
    NRF24_REG_RF_SETUP = 0x06,
    NRF24_REG_STATUS = 0x07,
    NRF24_REG_OBSERVE_TX = 0x08,
    NRF24_REG_CD = 0x09,
    NRF24_REG_RX_ADDR_P0 = 0x0A,
    NRF24_REG_RX_ADDR_P1 = 0x0B,
    // NRF24_REG_RX_ADDR_P2 = 0x0C,
    // NRF24_REG_RX_ADDR_P3 = 0x0D,
    // NRF24_REG_RX_ADDR_P4 = 0x0E,
    // NRF24_REG_RX_ADDR_P5 = 0x0F,
    NRF24_REG_TX_ADDR = 0x10,
    NRF24_REG_RX_PW_P0 = 0x11,
    NRF24_REG_RX_PW_P1 = 0x12,
    // NRF24_REG_RX_PW_P2 = 0x13,
    // NRF24_REG_RX_PW_P3 = 0x14,
    // NRF24_REG_RX_PW_P4 = 0x15,
    // NRF24_REG_RX_PW_P5 = 0x16,
    NRF24_REG_FIFO_STATUS = 0x17,
    // NRF24_REG_DYNPD = 0x1C,
    NRF24_REG_FEATURE = 0x1D,
} nrf24l01_register_map_address_t;

// CONFIG REGISTER BIT MASK
#define NRF24_REG_CONFIG_MASK_RX_DR (1 << 6)
#define NRF24_REG_CONFIG_MASK_TX_DS (1 << 5)
#define NRF24_REG_CONFIG_MASK_MAX_RT (1 << 4)
#define NRF24_REG_CONFIG_EN_CRC (1 << 3)
#define NRF24_REG_CONFIG_CRCO (1 << 2)
#define NRF24_REG_CONFIG_PWR_UP (1 << 1)
#define NRF24_REG_CONFIG_PRIM_RX 1

// EN_AA REGISTER BIT MASK
// #define NRF24_REG_ENAA_P5 (1 << 5)
// #define NRF24_REG_ENAA_P4 (1 << 4)
// #define NRF24_REG_ENAA_P3 (1 << 3)
// #define NRF24_REG_ENAA_P2 (1 << 2)
#define NRF24_REG_ENAA_P1 (1 << 1)
#define NRF24_REG_ENAA_P0 1

// EN_RXADDR ADDRESS BIT MASK
// #define NRF24_REG_ERX_P5 (1 << 5)
// #define NRF24_REG_ERX_P4 (1 << 4)
// #define NRF24_REG_ERX_P3 (1 << 3)
// #define NRF24_REG_ERX_P2 (1 << 2)
#define NRF24_REG_ERX_P1 (1 << 1)
#define NRF24_REG_ERX_P0 1

// SETUP_AW REGISTER BIT MASK
#define NRF24_REG_SETUP_AW_MASK 0x03
// TODO: it's hardcoded, remove as part of feature "support variable address
// width". probably enum?
#define NRF24_REG_SETUP_AW_5_BYTES_VALUE 0x03

// SETUP_RETR REGISTER BIT MASK
#define NRF24_REG_SETUP_RETR_ARD_MASK (0x0F << 4)
#define NRF24_REG_SETUP_RETR_ARC_MASK 0x0F

// RF_CH REGISTER BIT MASK
#define NRF24_REG_RF_CH_MASK 0x7F

// RF_SETUP REGISTER BIT MASK
// TODO: add contwave, part of "constant carrier wave", see Appendix C
#define NRF24_REG_RF_SETUP_RF_DR_LOW (1 << 5)
#define NRF24_REG_RF_SETUP_PLL_LOCK (1 << 4)
#define NRF24_REG_RF_SETUP_RF_DR_HIGH (1 << 3)
#define NRF24_REG_RF_SETUP_RF_PWR_MASK (0x03 << 1)
#define NRF24_REG_RF_SETUP_LNA_HCURR 1

// STATUS REGISTER BIT MASK
#define NRF24_REG_STATUS_RX_DR (1 << 6)
#define NRF24_REG_STATUS_TX_DS (1 << 5)
#define NRF24_REG_STATUS_MAX_RT (1 << 4)
#define NRF24_REG_STATUS_MASK_RX_P_NO (0x07 << 1)
#define NRF24_REG_STATUS_TX_FULL 1

// TODO: observe tx register, part of "observe register" feature to count lost
// packets and retransmissions

// TODO: rpd register, not sure for what

// RX_PW_PX REGISTER BIT MASK
// Same bitmask for RX_PW_P1 to RX_PW_P6 registers
#define NRF24_REG_RX_PW_PX_MASK 0x3F

// TODO: fifo status register bit mask, for feature "fifo status"

// TODO: dynamic payload register for feature "dynamic payload length"

// Maximum payload length in bytes for one "payload" in tx and rx
#define NRF24_PAYLOAD_MAX_SIZE 32U

// TODO: will be removed by feature "irq based"
// Maximum number of polls to the status register to check if message is sent
// (TX_DS) register.
#define NRF24_TX_STATUS_POLL_COUNT 100U

// Time CE is pulsed high to start a transmission.
#define NRF24_TX_PULSE_DELAY_MS 1U

static bool nrf24l01_flush_tx()
{
    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_FLUSH_TX);
    nrf24l01_port_deselect();

    return true;
}

static bool nrf24l01_flush_rx()
{
    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_FLUSH_RX);
    nrf24l01_port_deselect();

    return true;
}

// CONFIG
// set config register by sending command 000A AAAA, where A AAAA is register
// address (CONFIG)
// and in the next byte, the content
static bool nrf24l01_set_register_config(bool en_crc, bool crco, bool pwr_up,
                                         bool prim_rx)
{
    uint8_t reg = 0;

    if (en_crc)
        reg |= NRF24_REG_CONFIG_EN_CRC;

    if (crco)
        reg |= NRF24_REG_CONFIG_CRCO;

    if (pwr_up)
        reg |= NRF24_REG_CONFIG_PWR_UP;

    if (prim_rx)
        reg |= NRF24_REG_CONFIG_PRIM_RX;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_CONFIG);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// EN_AA
static bool
nrf24l01_set_register_enable_autoack(bool autoack_p0, bool autoack_p1
                                     // bool autoack_p2, bool autoack_p3,
                                     // bool autoack_p4, bool autoack_p5
)
{
    uint8_t reg = 0;

    if (autoack_p0)
        reg |= NRF24_REG_ENAA_P0;

    if (autoack_p1)
        reg |= NRF24_REG_ENAA_P1;

    // if (autoack_p2)
    //     reg |= NRF24_REG_ENAA_P2;

    // if (autoack_p3)
    //     reg |= NRF24_REG_ENAA_P3;

    // if (autoack_p4)
    //     reg |= NRF24_REG_ENAA_P4;

    // if (autoack_p5)
    //     reg |= NRF24_REG_ENAA_P5;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_EN_AA);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

typedef struct
{
    bool pipe_0;
    bool pipe_1;
} nrf24l01_register_enable_autoack_t;

static bool nrf24l01_get_register_enable_autoack(
    nrf24l01_register_enable_autoack_t *autoack)
{
    uint8_t reg = 0;

    if (autoack == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_R_REGISTER | NRF24_REG_EN_AA);
    nrf24l01_port_transmit_receive_byte(NRF24_CMD_NOP, &reg);
    nrf24l01_port_deselect();

    autoack->pipe_0 = (reg & NRF24_REG_ENAA_P0) != 0U;
    autoack->pipe_1 = (reg & NRF24_REG_ENAA_P1) != 0U;

    return true;
}

// EN_RXADDR
static bool nrf24l01_set_register_enabled_rx(bool enable_p0, bool enable_p1
                                             //  bool enable_p2, bool enable_p3,
                                             //  bool enable_p4, bool enable_p5
)
{
    uint8_t reg = 0;

    if (enable_p0)
        reg |= NRF24_REG_ERX_P0;

    if (enable_p1)
        reg |= NRF24_REG_ERX_P1;

    // if (enable_p2)
    //     reg |= NRF24_REG_ERX_P2;

    // if (enable_p3)
    //     reg |= NRF24_REG_ERX_P3;

    // if (enable_p4)
    //     reg |= NRF24_REG_ERX_P3;

    // if (enable_p5)
    //     reg |= NRF24_REG_ERX_P5;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_EN_RXADDR);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

typedef struct
{
    bool pipe_0;
    bool pipe_1;
} nrf24l01_register_enabled_rx_t;

static bool
nrf24l01_get_register_enabled_rx(nrf24l01_register_enabled_rx_t *enabled_rx)
{
    uint8_t reg = 0;

    if (enabled_rx == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_R_REGISTER | NRF24_REG_EN_RXADDR);
    nrf24l01_port_transmit_receive_byte(NRF24_CMD_NOP, &reg);
    nrf24l01_port_deselect();

    enabled_rx->pipe_0 = (reg & NRF24_REG_ERX_P0) != 0U;
    enabled_rx->pipe_1 = (reg & NRF24_REG_ERX_P1) != 0U;

    return true;
}

// SETUP_AW
static bool nrf24l01_set_register_setup_aw(void)
{
    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_SETUP_AW);
    nrf24l01_port_transmit_byte(NRF24_REG_SETUP_AW_5_BYTES_VALUE);
    nrf24l01_port_deselect();

    return true;
}

// SETUP_RETR
static bool
nrf24l01_set_register_setup_retr(nrf24l01_retransmit_delay_t retransmit_delay,
                                 uint8_t retransmit_count)
{
    uint8_t reg;

    if (retransmit_count > 15U)
        return false;

    reg = ((uint8_t)retransmit_delay << 4) & NRF24_REG_SETUP_RETR_ARD_MASK;
    reg |= retransmit_count & NRF24_REG_SETUP_RETR_ARC_MASK;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_SETUP_RETR);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// RF_SETUP
static bool nrf24l01_set_register_rf_setup(nrf24l01_variant_t variant,
                                           nrf24l01_datarate_t datarate,
                                           nrf24l01_pa_level_t pa_level)
{
    uint8_t reg = 0U;

    switch (datarate)
    {
    case NRF24L01_DATARATE_250KBPS:
        if (variant != NRF24L01_VARIANT_PLUS)
            return false;
        reg |= NRF24_REG_RF_SETUP_RF_DR_LOW;
        break;
    case NRF24L01_DATARATE_1MBPS:
        break;
    case NRF24L01_DATARATE_2MBPS:
        reg |= NRF24_REG_RF_SETUP_RF_DR_HIGH;
        break;
    default:
        return false;
    }

    // TODO: dont magic values, have an enum with typed
    switch (pa_level)
    {
    case NRF24L01_PA_MIN:
        break;
    case NRF24L01_PA_LOW:
        reg |= (0x01 << 1);
        break;
    case NRF24L01_PA_HIGH:
        reg |= (0x02 << 1);
        break;
    case NRF24L01_PA_MAX:
        reg |= (0x03 << 1);
        break;
    default:
        return false;
    }

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RF_SETUP);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// TODO: statug reg

// TODO: observe tx reg

// TODO: observe tx reg

// TODO: cd reg

// RF_CH
static bool nrf24l01_set_register_rf_channel(uint8_t channel)
{
    uint8_t reg = channel & NRF24_REG_RF_CH_MASK;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RF_CH);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// RF_SETUP
// static bool nrf24l01_set_register_rf_setup() {}

typedef enum
{
    NRF24L01_REGISTER_STATUS_RX_PIPE_0 = 0,
    NRF24L01_REGISTER_STATUS_RX_PIPE_1 = 1,
    NRF24L01_REGISTER_STATUS_RX_PIPE_EMPTY = 7,
} nrf24l01_register_status_rx_pipe_t;

typedef struct
{
    bool rx_data_ready;                            // RX_DR
    bool tx_data_sent;                             // TX_DS
    bool max_retransmits;                          // MAX_RT
    nrf24l01_register_status_rx_pipe_t rx_pipe_no; // RX_P_NO
    bool tx_fifo_full;                             // RX_FULL
} nrf24l01_register_status_t;

// STATUS
static bool nrf24l01_get_register_status(nrf24l01_register_status_t *status)
{
    uint8_t reg;

    if (status == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_receive_byte(NRF24_CMD_NOP, &reg);
    nrf24l01_port_deselect();

    status->rx_data_ready = (reg & NRF24_REG_STATUS_RX_DR) != 0U;
    status->tx_data_sent = (reg & NRF24_REG_STATUS_TX_DS) != 0U;
    status->max_retransmits = (reg & NRF24_REG_STATUS_MAX_RT) != 0U;
    status->rx_pipe_no =
        (nrf24l01_register_status_rx_pipe_t)((reg &
                                              NRF24_REG_STATUS_MASK_RX_P_NO) >>
                                             1);
    status->tx_fifo_full = (reg & NRF24_REG_STATUS_TX_FULL) != 0U;

    return true;
}

static bool nrf24l01_set_register_status(bool clear_rx_data_ready,
                                         bool clear_tx_data_sent,
                                         bool clear_max_retransmits)
{
    uint8_t reg = 0U;

    if (clear_rx_data_ready)
        reg |= NRF24_REG_STATUS_RX_DR;

    if (clear_tx_data_sent)
        reg |= NRF24_REG_STATUS_TX_DS;

    if (clear_max_retransmits)
        reg |= NRF24_REG_STATUS_MAX_RT;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_STATUS);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// RX_ADDR_0
static bool nrf24l01_set_register_rx_addr_p0(
    const uint8_t address[NRF24L01_ADDRESS_WIDTH])
{
    if (address == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0);
    nrf24l01_port_transmit(address, NRF24L01_ADDRESS_WIDTH);
    nrf24l01_port_deselect();

    return true;
}

// RX_ADDR_1
static bool nrf24l01_set_register_rx_addr_p1(
    const uint8_t address[NRF24L01_ADDRESS_WIDTH])
{
    if (address == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P1);
    nrf24l01_port_transmit(address, NRF24L01_ADDRESS_WIDTH);
    nrf24l01_port_deselect();

    return true;
}

// TX_ADDR
static bool
nrf24l01_set_register_tx_addr(const uint8_t address[NRF24L01_ADDRESS_WIDTH])
{
    if (address == NULL)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_TX_ADDR);
    nrf24l01_port_transmit(address, NRF24L01_ADDRESS_WIDTH);
    nrf24l01_port_deselect();

    return true;
}

// RX_PW_P0
static bool nrf24l01_set_register_rx_payload_width_p0(uint8_t width)
{
    if (width > 32U)
        return false;

    uint8_t reg = width & NRF24_REG_RX_PW_PX_MASK;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RX_PW_P0);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

// RX_PW_P1
static bool nrf24l01_set_register_rx_payload_width_p1(uint8_t width)
{
    if (width > 32U)
        return false;

    uint8_t reg = width & NRF24_REG_RX_PW_PX_MASK;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_REGISTER | NRF24_REG_RX_PW_P1);
    nrf24l01_port_transmit_byte(reg);
    nrf24l01_port_deselect();

    return true;
}

bool nrf24l01_send(const uint8_t *payload, size_t length)
{
    uint8_t padded_payload[NRF24_PAYLOAD_MAX_SIZE] = {0};
    nrf24l01_register_status_t status;
    uint32_t polls_remaining = NRF24_TX_STATUS_POLL_COUNT;

    if (payload == NULL)
        return false;

    if (length > NRF24_PAYLOAD_MAX_SIZE)
        return false;

    for (size_t i = 0; i < length; ++i)
        padded_payload[i] = payload[i];

    nrf24l01_port_set_ce_low();
    nrf24l01_flush_tx();
    nrf24l01_set_register_status(true, true, true);

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_W_TX_PAYLOAD);
    nrf24l01_port_transmit(padded_payload, NRF24_PAYLOAD_MAX_SIZE);
    nrf24l01_port_deselect();

    nrf24l01_port_set_ce_high();
    nrf24l01_port_delay(NRF24_TX_PULSE_DELAY_MS);
    nrf24l01_port_set_ce_low();

    // TODO: will be removed by feature "irq based"
    while (polls_remaining-- > 0U)
    {
        if (!nrf24l01_get_register_status(&status))
            return false;

        if (status.tx_data_sent)
        {
            nrf24l01_set_register_status(false, true, false);
            return true;
        }

        if (status.max_retransmits)
        {
            nrf24l01_set_register_status(false, false, true);
            nrf24l01_flush_tx();
            return false;
        }

        nrf24l01_port_delay(1U);
    }

    return false;
}

bool nrf24l01_recv(uint8_t *buffer, size_t buffer_len)
{
    nrf24l01_register_status_t status;

    if (buffer == NULL)
        return false;

    if (buffer_len < NRF24_PAYLOAD_MAX_SIZE)
        return false;

    if (!nrf24l01_get_register_status(&status))
        return false;

    if (!status.rx_data_ready)
        return false;

    if (status.rx_pipe_no == NRF24L01_REGISTER_STATUS_RX_PIPE_EMPTY)
        return false;

    nrf24l01_port_select();
    nrf24l01_port_transmit_byte(NRF24_CMD_R_RX_PAYLOAD);
    for (size_t i = 0; i < NRF24_PAYLOAD_MAX_SIZE; ++i)
        nrf24l01_port_transmit_receive_byte(NRF24_CMD_NOP, &buffer[i]);
    nrf24l01_port_deselect();

    nrf24l01_set_register_status(true, false, false);

    return true;
}

bool nrf24l01_init(const nrf24l01_config_t *config)
{
    if (config == NULL)
        return false;

    nrf24l01_port_set_ce_low();
    nrf24l01_port_delay(5);

    if (!nrf24l01_flush_tx())
        return false;

    if (!nrf24l01_set_register_status(true, true, true))
        return false;

    if (!nrf24l01_set_register_enable_autoack(true, false))
        return false;

    if (!nrf24l01_set_register_enabled_rx(true, false))
        return false;

    if (!nrf24l01_set_register_setup_aw())
        return false;

    if (!nrf24l01_set_register_setup_retr(config->retransmit_delay,
                                          config->retransmit_count))
        return false;

    if (!nrf24l01_set_register_rf_channel(config->channel))
        return false;

    if (!nrf24l01_set_register_rf_setup(config->variant, config->datarate,
                                        config->pa_level))
        return false;

    if (!nrf24l01_set_register_rx_payload_width_p0(32U))
        return false;

    if (!nrf24l01_set_register_config(true, true, true, false))
        return false;

    nrf24l01_port_delay(5);

    return true;
}

bool nrf24l01_set_tx_address(const uint8_t *address)
{
    nrf24l01_register_enable_autoack_t autoack;

    if (address == NULL)
        return false;

    if (!nrf24l01_set_register_tx_addr(address))
        return false;

    if (!nrf24l01_get_register_enable_autoack(&autoack))
        return false;

    if (autoack.pipe_0)
        return nrf24l01_set_register_rx_addr_p0(address);

    return true;
}

bool nrf24l01_set_pipe_rx_address(nrf24l01_pipe_t pipe,
                                  const uint8_t address[NRF24L01_ADDRESS_WIDTH])
{
    if (address == NULL)
        return false;

    switch (pipe)
    {
    case NRF24L01_PIPE_0:
        return nrf24l01_set_register_rx_addr_p0(address);
    case NRF24L01_PIPE_1:
        return nrf24l01_set_register_rx_addr_p1(address);
    default:
        return false;
    }
}

bool nrf24l01_config_pipe(nrf24l01_pipe_t pipe,
                          const nrf24l01_pipe_config_t *config)
{
    nrf24l01_register_enable_autoack_t autoack;

    if (config == NULL)
        return false;

    if ((config->payload_size == 0U) || (config->payload_size > 32U))
        return false;

    // TODO: maybe refactor so we can address by pipe numbers instead of having
    // to switch
    switch (pipe)
    {
    case NRF24L01_PIPE_0:
        if (!nrf24l01_set_register_rx_addr_p0(config->address))
            return false;
        if (!nrf24l01_set_register_rx_payload_width_p0(config->payload_size))
            return false;
        break;
    case NRF24L01_PIPE_1:
        if (!nrf24l01_set_register_rx_addr_p1(config->address))
            return false;
        if (!nrf24l01_set_register_rx_payload_width_p1(config->payload_size))
            return false;
        break;
    default:
        return false;
    }

    if (!nrf24l01_get_register_enable_autoack(&autoack))
        return false;

    switch (pipe)
    {
    case NRF24L01_PIPE_0:
        autoack.pipe_0 = config->auto_ack;
        break;
    case NRF24L01_PIPE_1:
        autoack.pipe_1 = config->auto_ack;
        break;
    default:
        return false;
    }

    return nrf24l01_set_register_enable_autoack(autoack.pipe_0, autoack.pipe_1);
}

bool nrf24l01_open_pipe(nrf24l01_pipe_t pipe)
{
    nrf24l01_register_enabled_rx_t enabled_rx;

    if (!nrf24l01_get_register_enabled_rx(&enabled_rx))
        return false;

    switch (pipe)
    {
    case NRF24L01_PIPE_0:
        enabled_rx.pipe_0 = true;
        break;
    case NRF24L01_PIPE_1:
        enabled_rx.pipe_1 = true;
        break;
    default:
        return false;
    }

    return nrf24l01_set_register_enabled_rx(enabled_rx.pipe_0,
                                            enabled_rx.pipe_1);
}

bool nrf24l01_close_pipe(nrf24l01_pipe_t pipe)
{
    nrf24l01_register_enabled_rx_t enabled_rx;

    if (!nrf24l01_get_register_enabled_rx(&enabled_rx))
        return false;

    switch (pipe)
    {
    case NRF24L01_PIPE_0:
        enabled_rx.pipe_0 = false;
        break;
    case NRF24L01_PIPE_1:
        enabled_rx.pipe_1 = false;
        break;
    default:
        return false;
    }

    return nrf24l01_set_register_enabled_rx(enabled_rx.pipe_0,
                                            enabled_rx.pipe_1);
}

bool nrf24l01_set_mode_tx()
{
    nrf24l01_port_set_ce_low();
    nrf24l01_set_register_status(true, true, true);
    nrf24l01_set_register_config(true, true, true, false);

    return true;
}

bool nrf24l01_set_mode_rx()
{
    nrf24l01_port_set_ce_low();
    nrf24l01_set_register_status(true, true, true);
    nrf24l01_set_register_config(true, true, true, true);
    nrf24l01_port_set_ce_high();

    return true;
}

bool nrf24l01_available()
{
    nrf24l01_register_status_t status;

    if (!nrf24l01_get_register_status(&status))
        return false;

    if (!status.rx_data_ready)
        return false;

    return status.rx_pipe_no != NRF24L01_REGISTER_STATUS_RX_PIPE_EMPTY;
}

// // on by default, no need to use rn
// bool nrf24l01_set_autoack_all_pipes(nrf24l01_t *radio, bool autoack) {
//     (void)radio;

//     return nrf24l01_set_register_enable_autoack(autoack, autoack);
// }
