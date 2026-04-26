from __future__ import annotations

import struct


def build_mbap(transaction_id: int, unit_id: int, pdu: bytes) -> bytes:
    protocol_id = 0
    length = len(pdu) + 1
    return struct.pack(
        ">HHHB",
        transaction_id & 0xFFFF,
        protocol_id,
        length,
        unit_id & 0xFF,
    )


def parse_mbap(frame: bytes) -> tuple[int, int, int, int, bytes]:
    if len(frame) < 7:
        raise ValueError("MBAP frame too short")

    transaction_id, protocol_id, length, unit_id = struct.unpack(
        ">HHHB", frame[:7]
    )
    pdu = frame[7:7 + length - 1]
    return transaction_id, protocol_id, length, unit_id, pdu


def build_rtu_frame(address: int, pdu: bytes) -> bytes:
    payload = bytes([address & 0xFF]) + pdu
    crc = modbus_crc16(payload)
    return payload + struct.pack("<H", crc)


def split_rtu_frame(frame: bytes) -> tuple[int, bytes, int]:
    if len(frame) < 4:
        raise ValueError("RTU frame too short")

    address = frame[0]
    pdu = frame[1:-2]
    crc = struct.unpack("<H", frame[-2:])[0]
    return address, pdu, crc


def modbus_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def bits_to_bytes(bits: list[bool]) -> bytes:
    packed = bytearray((len(bits) + 7) // 8)
    for index, bit in enumerate(bits):
        if bit:
            packed[index // 8] |= 1 << (index % 8)
    return bytes(packed)


def bytes_to_bits(payload: bytes, quantity: int) -> list[bool]:
    bits: list[bool] = []
    for index in range(quantity):
        bits.append(bool(payload[index // 8] & (1 << (index % 8))))
    return bits