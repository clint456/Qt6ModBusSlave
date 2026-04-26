from __future__ import annotations

import argparse
import struct
import time

import serial

from frames import build_rtu_frame, modbus_crc16, split_rtu_frame


FC_READ_HOLDING_REGISTERS = 0x03
FC_WRITE_SINGLE_REGISTER = 0x06


def recv_frame(port: serial.Serial, timeout: float = 2.0) -> bytes:
    deadline = time.time() + timeout
    buffer = bytearray()
    while time.time() < deadline:
        chunk = port.read(256)
        if chunk:
            buffer.extend(chunk)
            if len(buffer) >= 5:
                return bytes(buffer)
    raise TimeoutError("RTU response timeout")


def assert_ok(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def run_tests(port_name: str, baudrate: int, address: int = 1) -> None:
    with serial.Serial(port_name, baudrate=baudrate, timeout=0.2) as port:
        port.reset_input_buffer()

        write_request = build_rtu_frame(address, bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 4321))
        port.write(write_request)
        response = recv_frame(port)
        resp_address, resp_pdu, resp_crc = split_rtu_frame(response)
        assert_ok(resp_address == address, "RTU address should echo request address")
        assert_ok(resp_pdu[:5] == bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 4321), "RTU write response mismatch")
        assert_ok(resp_crc == modbus_crc16(response[:-2]), "RTU CRC mismatch for write response")

        read_request = build_rtu_frame(address, bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
        port.write(read_request)
        response = recv_frame(port)
        resp_address, resp_pdu, resp_crc = split_rtu_frame(response)
        assert_ok(resp_address == address, "RTU address should echo request address")
        assert_ok(resp_pdu[0] == FC_READ_HOLDING_REGISTERS, "RTU read function code mismatch")
        assert_ok(struct.unpack(">H", resp_pdu[2:4])[0] == 4321, "RTU holding register 0 should equal 4321")
        assert_ok(resp_crc == modbus_crc16(response[:-2]), "RTU CRC mismatch for read response")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Modbus RTU regression test suite")
    parser.add_argument("--port", required=True, help="Serial port path, for example /dev/ttyUSB0 or COM3")
    parser.add_argument("--baudrate", type=int, default=9600)
    parser.add_argument("--address", type=int, default=1)
    args = parser.parse_args()

    run_tests(args.port, args.baudrate, args.address)
    print("RTU protocol regression tests passed.")