# nRF24L01 Driver

Stateless, low level, hardware accurate, polling based driver for the nRF24L01 family 2.4 GHz wireless transceiver.

Based on https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf product specification, and tested againts the Si24R1 IC (nRF24L01+ clone).

## TODO

- Implement IRQ/interrupt-driven operation.
- Support all 6 RX pipes.
- Support configurable fixed payload sizes cleanly across all pipes.
- Support dynamic payload length.
- Support ACK payloads.
- Return richer status/error information than plain `bool`.
- Expose more typed register/status readback helpers where useful.
- Add explicit FIFO/status helpers.
- Support variable address width instead of keeping it hardcoded to 5 bytes.

## Current limitations

- Polling only, no IRQ support yet.
- Only 2 RX pipes are currently exposed.
- Address width is currently hardcoded to 5 bytes.
- The API is designed around fixed-size payloads.
