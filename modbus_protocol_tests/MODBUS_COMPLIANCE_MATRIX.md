# Modbus Standard Compliance Matrix (Current Project)

## Scope

This matrix evaluates conformance for the current server implementation.

Implemented public function codes:
- 0x01 Read Coils
- 0x02 Read Discrete Inputs
- 0x03 Read Holding Registers
- 0x04 Read Input Registers
- 0x05 Write Single Coil
- 0x06 Write Single Register
- 0x0F Write Multiple Coils
- 0x10 Write Multiple Registers

Not implemented in server (must return exception 0x01):
- 0x07, 0x08, 0x0B, 0x0C, 0x11, 0x16, 0x17

## Covered by Automated Regression (pytest)

- MBAP header correctness
- Transaction ID echo behavior
- Unit ID echo behavior
- Normal-path read/write for FC01/02/03/04/05/06/0F/10
- Quantity boundary checks
  - FC01/02: 1..2000
  - FC03/04: 1..125
  - FC0F: 1..1968
  - FC10: 1..123
- Address boundary checks (including end-address overflow)
- Exception mapping
  - Illegal Function (0x01)
  - Illegal Data Address (0x02)
  - Illegal Data Value (0x03)
- Malformed request length handling
- Byte count validation for FC0F/FC10
- RTU basic round-trip and CRC (optional, depends on serial environment)

## Out of Current Automated Scope

- Full serial line specification timing constraints (silent interval, frame timing)
- Broadcast semantics on serial line (address 0)
- Multi-client concurrency stress and soak tests
- Interoperability certification against official Modbus Conformance Test Tool
- Security hardening (non-standard to core Modbus spec)

## How to Interpret Coverage

- For implemented function codes in this project, protocol-level behavior is now broadly covered with boundary and exception tests.
- "100% Modbus standard coverage" for the entire standard is not realistic unless all optional/extended function groups and serial timing requirements are implemented and certified.
- This suite is suitable as a strong regression gate for the currently implemented protocol surface.
