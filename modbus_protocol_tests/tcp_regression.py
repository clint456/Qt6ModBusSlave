from __future__ import annotations

import argparse
import logging
import socket
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from modbus_protocol_tests.frames import build_mbap, bits_to_bytes, parse_mbap, bytes_to_bits


LOGGER = logging.getLogger("modbus.protocol.tcp")


FC_READ_COILS = 0x01
FC_READ_DISCRETE_INPUTS = 0x02
FC_READ_HOLDING_REGISTERS = 0x03
FC_READ_INPUT_REGISTERS = 0x04
FC_WRITE_SINGLE_COIL = 0x05
FC_WRITE_SINGLE_REGISTER = 0x06
FC_WRITE_MULTIPLE_COILS = 0x0F
FC_WRITE_MULTIPLE_REGISTERS = 0x10


EX_ILLEGAL_FUNCTION = 0x01
EX_ILLEGAL_DATA_ADDRESS = 0x02
EX_ILLEGAL_DATA_VALUE = 0x03


@dataclass
class TcpConfig:
    host: str = "127.0.0.1"
    port: int = 502
    unit_id: int = 1
    timeout: float = 2.0


class ModbusTcpTester:
    def __init__(self, config: TcpConfig):
        self.config = config
        self.transaction_id = 1
        self.socket: socket.socket | None = None

    def __enter__(self):
        LOGGER.info("连接 TCP %s:%s，单元标识 unit_id=%s", self.config.host, self.config.port, self.config.unit_id)
        self.socket = socket.create_connection(
            (self.config.host, self.config.port), timeout=self.config.timeout
        )
        self.socket.settimeout(self.config.timeout)
        return self

    def __exit__(self, exc_type, exc, tb):
        if self.socket:
            self.socket.close()
        LOGGER.info("断开 TCP %s:%s", self.config.host, self.config.port)

    def request(self, pdu: bytes) -> bytes:
        assert self.socket is not None
        tx_tid = self.transaction_id
        frame = build_mbap(tx_tid, self.config.unit_id, pdu) + pdu
        LOGGER.info("发送请求 tid=%d unit=%d pdu=%s", tx_tid, self.config.unit_id, pdu.hex(" ").upper())
        self.socket.sendall(frame)
        self.transaction_id = (self.transaction_id + 1) & 0xFFFF

        header = self._recv_exact(7)
        _, _, length, _, = struct.unpack(">HHHB", header)
        body = self._recv_exact(length - 1)
        _, _, _, rx_unit_id = struct.unpack(">HHHB", header)
        LOGGER.info("接收响应 tid=%d unit=%d pdu=%s", tx_tid, rx_unit_id, body.hex(" ").upper())
        return header + body

    def _recv_exact(self, size: int) -> bytes:
        assert self.socket is not None
        data = bytearray()
        while len(data) < size:
            chunk = self.socket.recv(size - len(data))
            if not chunk:
                raise ConnectionError("server closed the connection")
            data.extend(chunk)
        return bytes(data)


def assert_ok(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def is_exception_response(pdu: bytes, function_code: int, exception_code: int) -> bool:
    return len(pdu) >= 2 and pdu[0] == (function_code | 0x80) and pdu[1] == exception_code


def run_tests(config: TcpConfig) -> None:
    with ModbusTcpTester(config) as client:
        test_protocol_header(client, config)
        test_read_and_write_coils(client)
        test_read_and_write_registers(client)
        test_batch_operations(client)
        test_exception_cases(client)


def test_protocol_header(client: ModbusTcpTester, config: TcpConfig) -> None:
    pdu = bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1)
    frame = client.request(pdu)
    transaction_id, protocol_id, length, unit_id, response_pdu = parse_mbap(frame)
    assert_ok(transaction_id == 1, "Transaction ID should echo the first request id")
    assert_ok(protocol_id == 0, "Protocol ID must be 0 for Modbus TCP")
    assert_ok(unit_id == config.unit_id, "Unit ID must echo request Unit ID")
    assert_ok(response_pdu[0] == FC_READ_HOLDING_REGISTERS, "Function code should echo")
    assert_ok(length == len(response_pdu) + 1, "MBAP length must match PDU length")


def test_read_and_write_coils(client: ModbusTcpTester) -> None:
    response = client.request(bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0xFF00))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[:5] == bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0xFF00), "Write single coil echo mismatch")

    response = client.request(bytes([FC_READ_COILS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[0] == FC_READ_COILS, "Read coils function code mismatch")
    assert_ok(pdu[1] == 1, "Read coils byte count should be 1 for one bit")
    bits = bytes_to_bits(pdu[2:], 1)
    assert_ok(bits[0] is True, "Coil 0 should be set after write")


def test_read_and_write_registers(client: ModbusTcpTester) -> None:
    response = client.request(bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 1234))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[:5] == bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 1234), "Write single register echo mismatch")

    response = client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[0] == FC_READ_HOLDING_REGISTERS, "Read holding registers function code mismatch")
    assert_ok(pdu[1] == 2, "Read holding registers byte count should be 2 for one register")
    value = struct.unpack(">H", pdu[2:4])[0]
    assert_ok(value == 1234, f"Holding register 0 should be 1234, got {value}")


def test_batch_operations(client: ModbusTcpTester) -> None:
    coil_bits = [True, False, True, True, False, False, True, False, True]
    coil_bytes = bits_to_bytes(coil_bits)
    request = bytes([FC_WRITE_MULTIPLE_COILS]) + struct.pack(">HHB", 10, len(coil_bits), len(coil_bytes)) + coil_bytes
    response = client.request(request)
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[:5] == bytes([FC_WRITE_MULTIPLE_COILS]) + struct.pack(">HH", 10, len(coil_bits)), "Write multiple coils response mismatch")

    response = client.request(bytes([FC_READ_COILS]) + struct.pack(">HH", 10, len(coil_bits)))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(bytes_to_bits(pdu[2:], len(coil_bits)) == coil_bits, "Batch coil values should round-trip")

    register_values = [100, 200, 300]
    payload = b"".join(struct.pack(">H", value) for value in register_values)
    request = bytes([FC_WRITE_MULTIPLE_REGISTERS]) + struct.pack(">HHB", 20, len(register_values), len(payload)) + payload
    response = client.request(request)
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(pdu[:5] == bytes([FC_WRITE_MULTIPLE_REGISTERS]) + struct.pack(">HH", 20, len(register_values)), "Write multiple registers response mismatch")

    response = client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 20, len(register_values)))
    _, _, _, _, pdu = parse_mbap(response)
    values = [struct.unpack(">H", pdu[i:i+2])[0] for i in range(2, len(pdu), 2)]
    assert_ok(values == register_values, f"Batch register values should round-trip, got {values}")


def test_exception_cases(client: ModbusTcpTester) -> None:
    response = client.request(bytes([0x7F, 0x00, 0x00, 0x00, 0x00]))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(is_exception_response(pdu, 0x7F, EX_ILLEGAL_FUNCTION), "Illegal function should return exception 0x01")

    response = client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 9999, 2))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(is_exception_response(pdu, FC_READ_HOLDING_REGISTERS, EX_ILLEGAL_DATA_ADDRESS), "Out-of-range address should return 0x02")

    response = client.request(bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0x1234))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(is_exception_response(pdu, FC_WRITE_SINGLE_COIL, EX_ILLEGAL_DATA_VALUE), "Invalid coil value should return 0x03")

    response = client.request(bytes([FC_WRITE_MULTIPLE_REGISTERS]) + struct.pack(">HHB", 0, 2, 2) + struct.pack(">HH", 1, 2))
    _, _, _, _, pdu = parse_mbap(response)
    assert_ok(is_exception_response(pdu, FC_WRITE_MULTIPLE_REGISTERS, EX_ILLEGAL_DATA_VALUE), "Invalid byte count should return 0x03")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Modbus TCP regression test suite")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=502)
    parser.add_argument("--unit-id", type=int, default=1)
    args = parser.parse_args()

    run_tests(TcpConfig(host=args.host, port=args.port, unit_id=args.unit_id))
    print("TCP protocol regression tests passed.")