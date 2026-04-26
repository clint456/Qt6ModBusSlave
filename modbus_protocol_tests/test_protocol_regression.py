from __future__ import annotations

import struct
import time
import sys
from pathlib import Path

import pytest

try:
    import serial
except ModuleNotFoundError:
    serial = None

if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from modbus_protocol_tests.frames import bits_to_bytes, bytes_to_bits, build_rtu_frame, modbus_crc16, parse_mbap, split_rtu_frame
from modbus_protocol_tests.tcp_regression import (
    EX_ILLEGAL_DATA_ADDRESS,
    EX_ILLEGAL_DATA_VALUE,
    EX_ILLEGAL_FUNCTION,
    FC_READ_COILS,
    FC_READ_DISCRETE_INPUTS,
    FC_READ_HOLDING_REGISTERS,
    FC_READ_INPUT_REGISTERS,
    FC_WRITE_MULTIPLE_COILS,
    FC_WRITE_MULTIPLE_REGISTERS,
    FC_WRITE_SINGLE_COIL,
    FC_WRITE_SINGLE_REGISTER,
    is_exception_response,
)


TEST_COVERAGE_MAP = {
    "test_tcp_protocol_header": [
        "MBAP 头字段(transaction/protocol/length/unit-id)校验",
        "FC03 响应结构合法性",
    ],
    "test_tcp_read_and_write_coils": [
        "FC01 读线圈",
        "FC05 写单线圈",
        "写后读一致性验证",
    ],
    "test_tcp_read_discrete_inputs": [
        "FC02 读离散输入",
        "位打包/解包一致性",
    ],
    "test_tcp_read_and_write_registers": [
        "FC03 读保持寄存器",
        "FC06 写单寄存器",
        "寄存器写后读回一致性",
    ],
    "test_tcp_read_input_registers": [
        "FC04 读输入寄存器",
        "寄存器字节序解析校验",
    ],
    "test_tcp_batch_operations": [
        "FC0F 批量写线圈",
        "FC10 批量写保持寄存器",
        "批量写后读回一致性",
    ],
    "test_tcp_exception_cases": [
        "非法功能码异常(0x01)",
        "非法地址异常(0x02)",
        "非法数据值异常(0x03)",
    ],
    "test_rtu_single_register_roundtrip": [
        "RTU FC06/FC03 往返验证",
        "RTU CRC16 校验",
        "RTU 从站地址一致性",
    ],
    "test_tcp_transaction_id_echo_sequence": [
        "MBAP Transaction ID 回显一致性",
        "多请求顺序一致性",
    ],
    "test_tcp_unsupported_standard_function_codes": [
        "未实现标准功能码返回非法功能异常(0x01)",
    ],
    "test_tcp_read_quantity_limits": [
        "FC01/02/03/04 读数量上下限校验",
        "非法数量返回异常(0x03)",
    ],
    "test_tcp_read_address_boundaries": [
        "FC01/02/03/04 地址边界校验",
        "越界地址返回异常(0x02)",
    ],
    "test_tcp_write_single_address_boundaries": [
        "FC05/06 写单点地址边界校验",
        "写地址越界返回异常(0x02)",
    ],
    "test_tcp_write_multi_address_boundaries": [
        "FC0F/10 批量写地址边界校验",
        "批量写越界返回异常(0x02)",
    ],
    "test_tcp_write_multi_limits_and_byte_count": [
        "FC0F/10 数量限制与字节计数一致性",
        "非法数量/字节数返回异常(0x03)",
    ],
    "test_tcp_short_request_pdu_returns_illegal_data_value": [
        "请求 PDU 长度不足处理",
        "长度错误返回异常(0x03)",
    ],
}


TEST_EXPECTATION_MAP = {
    "test_tcp_protocol_header": "正常响应",
    "test_tcp_read_and_write_coils": "正常响应",
    "test_tcp_read_discrete_inputs": "正常响应",
    "test_tcp_read_and_write_registers": "正常响应",
    "test_tcp_read_input_registers": "正常响应",
    "test_tcp_batch_operations": "正常响应",
    "test_tcp_exception_cases": "异常响应（预期异常）",
    "test_rtu_single_register_roundtrip": "正常响应",
    "test_tcp_transaction_id_echo_sequence": "正常响应",
    "test_tcp_unsupported_standard_function_codes": "异常响应（预期异常）",
    "test_tcp_read_quantity_limits": "边界混合（正常+异常）",
    "test_tcp_read_address_boundaries": "边界混合（正常+异常）",
    "test_tcp_write_single_address_boundaries": "边界混合（正常+异常）",
    "test_tcp_write_multi_address_boundaries": "边界混合（正常+异常）",
    "test_tcp_write_multi_limits_and_byte_count": "边界混合（正常+异常）",
    "test_tcp_short_request_pdu_returns_illegal_data_value": "异常响应（预期异常）",
}


def _rtu_recv_frame(port: serial.Serial, timeout: float = 2.0) -> bytes:
    deadline = time.time() + timeout
    buffer = bytearray()
    while time.time() < deadline:
        chunk = port.read(256)
        if chunk:
            buffer.extend(chunk)
            if len(buffer) >= 5:
                return bytes(buffer)
    raise TimeoutError("RTU response timeout")


def test_tcp_protocol_header(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
    transaction_id, protocol_id, length, unit_id, pdu = parse_mbap(response)

    assert transaction_id == 1
    assert protocol_id == 0
    assert unit_id == 1
    assert pdu[0] == FC_READ_HOLDING_REGISTERS
    assert length == len(pdu) + 1


def test_tcp_read_and_write_coils(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([FC_READ_COILS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[0] == FC_READ_COILS
    assert pdu[1] == 1
    assert bytes_to_bits(pdu[2:], 1) == [True]

    response = modbus_tcp_client.request(bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0x0000))
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0x0000)

    response = modbus_tcp_client.request(bytes([FC_READ_COILS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert bytes_to_bits(pdu[2:], 1) == [False]


def test_tcp_read_discrete_inputs(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([FC_READ_DISCRETE_INPUTS]) + struct.pack(">HH", 0, 8))
    _, _, _, _, pdu = parse_mbap(response)

    assert pdu[0] == FC_READ_DISCRETE_INPUTS
    assert pdu[1] == 1
    assert bytes_to_bits(pdu[2:], 8) == [False] * 8


def test_tcp_read_and_write_registers(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert struct.unpack(">H", pdu[2:4])[0] == 4321

    response = modbus_tcp_client.request(bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 1234))
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 1234)

    response = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert struct.unpack(">H", pdu[2:4])[0] == 1234


def test_tcp_read_input_registers(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([FC_READ_INPUT_REGISTERS]) + struct.pack(">HH", 0, 4))
    _, _, _, _, pdu = parse_mbap(response)

    assert pdu[0] == FC_READ_INPUT_REGISTERS
    assert pdu[1] == 8
    values = [struct.unpack(">H", pdu[i:i + 2])[0] for i in range(2, len(pdu), 2)]
    assert values == [0, 0, 0, 0]


def test_tcp_batch_operations(modbus_tcp_client):
    coil_bits = [True, False, True, True, False, False, True, False, True]
    coil_payload = bits_to_bytes(coil_bits)
    response = modbus_tcp_client.request(
        bytes([FC_WRITE_MULTIPLE_COILS])
        + struct.pack(">HHB", 10, len(coil_bits), len(coil_payload))
        + coil_payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([FC_WRITE_MULTIPLE_COILS]) + struct.pack(">HH", 10, len(coil_bits))

    response = modbus_tcp_client.request(bytes([FC_READ_COILS]) + struct.pack(">HH", 10, len(coil_bits)))
    _, _, _, _, pdu = parse_mbap(response)
    assert bytes_to_bits(pdu[2:], len(coil_bits)) == coil_bits

    register_values = [100, 200, 300]
    register_payload = b"".join(struct.pack(">H", value) for value in register_values)
    response = modbus_tcp_client.request(
        bytes([FC_WRITE_MULTIPLE_REGISTERS])
        + struct.pack(">HHB", 20, len(register_values), len(register_payload))
        + register_payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([FC_WRITE_MULTIPLE_REGISTERS]) + struct.pack(">HH", 20, len(register_values))

    response = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 20, len(register_values)))
    _, _, _, _, pdu = parse_mbap(response)
    values = [struct.unpack(">H", pdu[i:i + 2])[0] for i in range(2, len(pdu), 2)]
    assert values == register_values


def test_tcp_exception_cases(modbus_tcp_client):
    response = modbus_tcp_client.request(bytes([0x7F, 0x00, 0x00, 0x00, 0x00]))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, 0x7F, EX_ILLEGAL_FUNCTION)

    response = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 9999, 2))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, FC_READ_HOLDING_REGISTERS, EX_ILLEGAL_DATA_ADDRESS)

    response = modbus_tcp_client.request(bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0x1234))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, FC_WRITE_SINGLE_COIL, EX_ILLEGAL_DATA_VALUE)

    response = modbus_tcp_client.request(bytes([FC_WRITE_MULTIPLE_REGISTERS]) + struct.pack(">HHB", 0, 2, 2) + struct.pack(">HH", 1, 2))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, FC_WRITE_MULTIPLE_REGISTERS, EX_ILLEGAL_DATA_VALUE)


def test_tcp_transaction_id_echo_sequence(modbus_tcp_client):
    expected_tid_1 = modbus_tcp_client.transaction_id
    response_1 = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 0, 1))
    tid_1, _, _, _, _ = parse_mbap(response_1)
    assert tid_1 == expected_tid_1

    expected_tid_2 = modbus_tcp_client.transaction_id
    response_2 = modbus_tcp_client.request(bytes([FC_READ_HOLDING_REGISTERS]) + struct.pack(">HH", 1, 1))
    tid_2, _, _, _, _ = parse_mbap(response_2)
    assert tid_2 == expected_tid_2


@pytest.mark.parametrize("function_code", [0x07, 0x08, 0x0B, 0x0C, 0x11, 0x16, 0x17])
def test_tcp_unsupported_standard_function_codes(modbus_tcp_client, function_code: int):
    response = modbus_tcp_client.request(bytes([function_code, 0x00, 0x00, 0x00, 0x00]))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_FUNCTION)


@pytest.mark.parametrize(
    "function_code,max_quantity,valid_start,invalid_quantity",
    [
        (FC_READ_COILS, 2000, 0, 2001),
        (FC_READ_DISCRETE_INPUTS, 2000, 0, 2001),
        (FC_READ_HOLDING_REGISTERS, 125, 0, 126),
        (FC_READ_INPUT_REGISTERS, 125, 0, 126),
    ],
)
def test_tcp_read_quantity_limits(
    modbus_tcp_client,
    function_code: int,
    max_quantity: int,
    valid_start: int,
    invalid_quantity: int,
):
    response = modbus_tcp_client.request(bytes([function_code]) + struct.pack(">HH", valid_start, max_quantity))
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[0] == function_code

    response = modbus_tcp_client.request(bytes([function_code]) + struct.pack(">HH", 0, 0))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)

    response = modbus_tcp_client.request(bytes([function_code]) + struct.pack(">HH", 0, invalid_quantity))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)


@pytest.mark.parametrize(
    "function_code",
    [FC_READ_COILS, FC_READ_DISCRETE_INPUTS, FC_READ_HOLDING_REGISTERS, FC_READ_INPUT_REGISTERS],
)
def test_tcp_read_address_boundaries(modbus_tcp_client, function_code: int):
    response = modbus_tcp_client.request(bytes([function_code]) + struct.pack(">HH", 9999, 1))
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[0] == function_code

    response = modbus_tcp_client.request(bytes([function_code]) + struct.pack(">HH", 9999, 2))
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_ADDRESS)


@pytest.mark.parametrize(
    "function_code,valid_payload,invalid_payload",
    [
        (FC_WRITE_SINGLE_COIL, struct.pack(">HH", 9999, 0xFF00), struct.pack(">HH", 10000, 0xFF00)),
        (FC_WRITE_SINGLE_REGISTER, struct.pack(">HH", 9999, 0x1357), struct.pack(">HH", 10000, 0x1357)),
    ],
)
def test_tcp_write_single_address_boundaries(modbus_tcp_client, function_code: int, valid_payload: bytes, invalid_payload: bytes):
    response = modbus_tcp_client.request(bytes([function_code]) + valid_payload)
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([function_code]) + valid_payload

    response = modbus_tcp_client.request(bytes([function_code]) + invalid_payload)
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_ADDRESS)


@pytest.mark.parametrize(
    "function_code,valid_start,invalid_start,quantity,payload",
    [
        (FC_WRITE_MULTIPLE_COILS, 9998, 9999, 2, bytes([0b00000011])),
        (FC_WRITE_MULTIPLE_REGISTERS, 9998, 9999, 2, struct.pack(">HH", 0xAA55, 0x55AA)),
    ],
)
def test_tcp_write_multi_address_boundaries(
    modbus_tcp_client,
    function_code: int,
    valid_start: int,
    invalid_start: int,
    quantity: int,
    payload: bytes,
):
    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", valid_start, quantity, len(payload)) + payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([function_code]) + struct.pack(">HH", valid_start, quantity)

    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", invalid_start, quantity, len(payload)) + payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_ADDRESS)


@pytest.mark.parametrize(
    "function_code,overflow_quantity,max_quantity,valid_payload,bad_byte_count",
    [
        (FC_WRITE_MULTIPLE_COILS, 1969, 1968, bytes([0x00]), 2),
        (FC_WRITE_MULTIPLE_REGISTERS, 124, 123, struct.pack(">H", 0x1234), 1),
    ],
)
def test_tcp_write_multi_limits_and_byte_count(
    modbus_tcp_client,
    function_code: int,
    overflow_quantity: int,
    max_quantity: int,
    valid_payload: bytes,
    bad_byte_count: int,
):
    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", 0, 0, 0)
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)

    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", 0, overflow_quantity, len(valid_payload)) + valid_payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)

    if function_code == FC_WRITE_MULTIPLE_COILS:
        valid_byte_count = (max_quantity + 7) // 8
        payload = bytes(valid_byte_count)
    else:
        valid_byte_count = max_quantity * 2
        payload = bytes(valid_byte_count)

    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", 0, max_quantity, valid_byte_count) + payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert pdu[:5] == bytes([function_code]) + struct.pack(">HH", 0, max_quantity)

    response = modbus_tcp_client.request(
        bytes([function_code]) + struct.pack(">HHB", 0, 1, bad_byte_count) + valid_payload
    )
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)


@pytest.mark.parametrize(
    "function_code,short_payload",
    [
        (FC_READ_COILS, bytes([0x00, 0x00, 0x00])),
        (FC_READ_DISCRETE_INPUTS, bytes([0x00, 0x00, 0x00])),
        (FC_READ_HOLDING_REGISTERS, bytes([0x00, 0x00, 0x00])),
        (FC_READ_INPUT_REGISTERS, bytes([0x00, 0x00, 0x00])),
        (FC_WRITE_SINGLE_COIL, bytes([0x00, 0x00, 0x00])),
        (FC_WRITE_SINGLE_REGISTER, bytes([0x00, 0x00, 0x00])),
        (FC_WRITE_MULTIPLE_COILS, bytes([0x00, 0x00, 0x00, 0x00])),
        (FC_WRITE_MULTIPLE_REGISTERS, bytes([0x00, 0x00, 0x00, 0x00])),
    ],
)
def test_tcp_short_request_pdu_returns_illegal_data_value(modbus_tcp_client, function_code: int, short_payload: bytes):
    response = modbus_tcp_client.request(bytes([function_code]) + short_payload)
    _, _, _, _, pdu = parse_mbap(response)
    assert is_exception_response(pdu, function_code, EX_ILLEGAL_DATA_VALUE)


def test_rtu_single_register_roundtrip(rtu_port: str, rtu_baudrate: int, rtu_address: int):
    if not rtu_port:
        pytest.skip("RTU port not provided")
    if serial is None:
        pytest.skip("pyserial is not installed")

    with serial.Serial(rtu_port, baudrate=rtu_baudrate, timeout=0.2) as port:
        port.reset_input_buffer()

        write_request = build_rtu_frame(rtu_address, bytes([0x06]) + struct.pack(">HH", 0, 2468))
        port.write(write_request)
        response = _rtu_recv_frame(port)
        resp_address, resp_pdu, resp_crc = split_rtu_frame(response)
        assert resp_address == rtu_address
        assert resp_pdu[:5] == bytes([0x06]) + struct.pack(">HH", 0, 2468)
        assert resp_crc == modbus_crc16(response[:-2])

        read_request = build_rtu_frame(rtu_address, bytes([0x03]) + struct.pack(">HH", 0, 1))
        port.write(read_request)
        response = _rtu_recv_frame(port)
        resp_address, resp_pdu, resp_crc = split_rtu_frame(response)
        assert resp_address == rtu_address
        assert resp_pdu[0] == 0x03
        assert struct.unpack(">H", resp_pdu[2:4])[0] == 2468
        assert resp_crc == modbus_crc16(response[:-2])