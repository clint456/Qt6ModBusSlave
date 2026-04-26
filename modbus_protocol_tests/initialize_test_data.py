from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from modbus_protocol_tests.tcp_regression import ModbusTcpTester, TcpConfig


FC_WRITE_SINGLE_COIL = 0x05
FC_WRITE_MULTIPLE_COILS = 0x0F
FC_WRITE_SINGLE_REGISTER = 0x06
FC_WRITE_MULTIPLE_REGISTERS = 0x10


def seed_protocol_data(host: str, port: int, unit_id: int) -> None:
    with ModbusTcpTester(TcpConfig(host=host, port=port, unit_id=unit_id)) as client:
        client.request(bytes([FC_WRITE_SINGLE_COIL]) + struct.pack(">HH", 0, 0xFF00))

        coil_bits = [False] * 9
        coil_payload = bytes((len(coil_bits) + 7) // 8)
        client.request(
            bytes([FC_WRITE_MULTIPLE_COILS])
            + struct.pack(">HHB", 10, len(coil_bits), len(coil_payload))
            + coil_payload
        )

        client.request(bytes([FC_WRITE_SINGLE_REGISTER]) + struct.pack(">HH", 0, 4321))

        register_values = [1, 2, 3]
        register_payload = b"".join(struct.pack(">H", value) for value in register_values)
        client.request(
            bytes([FC_WRITE_MULTIPLE_REGISTERS])
            + struct.pack(">HHB", 20, len(register_values), len(register_payload))
            + register_payload
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="向 Modbus TCP 服务器写入协议测试预置数据")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=502)
    parser.add_argument("--unit-id", type=int, default=1)
    args = parser.parse_args()

    seed_protocol_data(args.host, args.port, args.unit_id)
    print("协议测试预置数据写入完成。")