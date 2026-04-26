from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def run_command(command: list[str]) -> None:
    completed = subprocess.run(command, check=False)
    if completed.returncode != 0:
        raise SystemExit(completed.returncode)


def run_command_with_code(command: list[str]) -> int:
    completed = subprocess.run(command, check=False)
    return completed.returncode


def main() -> None:
    parser = argparse.ArgumentParser(description="运行 Modbus 协议回归测试")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=502)
    parser.add_argument("--unit-id", type=int, default=1)
    parser.add_argument("--rtu-port", default="")
    parser.add_argument("--rtu-baudrate", type=int, default=9600)
    parser.add_argument("--rtu-address", type=int, default=1)
    parser.add_argument("--log-level", default="INFO", choices=["DEBUG", "INFO", "WARNING", "ERROR"])
    args = parser.parse_args()

    base_dir = Path(__file__).resolve().parent
    report_dir = base_dir / "reports"
    report_dir.mkdir(exist_ok=True)
    python = sys.executable
    junit_xml = report_dir / "modbus_protocol_report.xml"
    detailed_md = report_dir / "modbus_protocol_report_detailed.md"
    detailed_json = report_dir / "modbus_protocol_report_detailed.json"
    pytest_log = report_dir / "modbus_protocol_run.log"

    print("正在写入协议测试预置数据...")
    run_command([
        python,
        str(base_dir / "initialize_test_data.py"),
        "--host",
        args.host,
        "--port",
        str(args.port),
        "--unit-id",
        str(args.unit_id),
    ])

    print("正在执行 pytest 回归测试套件...")
    pytest_exit = run_command_with_code([
        python,
        "-m",
        "pytest",
        str(base_dir / "test_protocol_regression.py"),
        "-q",
        "--tb=short",
        "-o",
        "log_cli=true",
        "--log-cli-level",
        args.log_level,
        "--log-file",
        str(pytest_log),
        "--log-file-level",
        args.log_level,
        "--log-file-format",
        "%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        "--capture",
        "tee-sys",
        "--junitxml",
        str(junit_xml),
        "--modbus-host",
        args.host,
        "--modbus-port",
        str(args.port),
        "--modbus-unit-id",
        str(args.unit_id),
        "--rtu-port",
        args.rtu_port,
        "--rtu-baudrate",
        str(args.rtu_baudrate),
        "--rtu-address",
        str(args.rtu_address),
    ])

    print("正在生成逐用例详细报告...")
    run_command([
        python,
        str(base_dir / "generate_detailed_report.py"),
        "--junit-xml",
        str(junit_xml),
        "--markdown-out",
        str(detailed_md),
        "--json-out",
        str(detailed_json),
    ])

    print(f"Pytest 报告已写入: {junit_xml}")
    print(f"运行日志已写入: {pytest_log}")
    print(f"详细 Markdown 报告已写入: {detailed_md}")
    print(f"详细 JSON 报告已写入: {detailed_json}")

    if pytest_exit != 0:
        raise SystemExit(pytest_exit)

    print("本次协议回归测试全部通过。")


if __name__ == "__main__":
    main()