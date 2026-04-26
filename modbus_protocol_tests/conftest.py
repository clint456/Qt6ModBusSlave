from __future__ import annotations

import logging
import sys
from pathlib import Path

import pytest

if __package__ is None or __package__ == "":
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from modbus_protocol_tests.tcp_regression import ModbusTcpTester, TcpConfig


LOGGER = logging.getLogger("modbus.protocol.pytest")


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption("--modbus-host", action="store", default="127.0.0.1")
    parser.addoption("--modbus-port", action="store", type=int, default=502)
    parser.addoption("--modbus-unit-id", action="store", type=int, default=1)
    parser.addoption("--rtu-port", action="store", default="")
    parser.addoption("--rtu-baudrate", action="store", type=int, default=9600)
    parser.addoption("--rtu-address", action="store", type=int, default=1)


@pytest.fixture(scope="session")
def modbus_tcp_config(pytestconfig: pytest.Config) -> TcpConfig:
    return TcpConfig(
        host=pytestconfig.getoption("--modbus-host"),
        port=pytestconfig.getoption("--modbus-port"),
        unit_id=pytestconfig.getoption("--modbus-unit-id"),
    )


@pytest.fixture(scope="session")
def modbus_tcp_client(modbus_tcp_config: TcpConfig):
    with ModbusTcpTester(modbus_tcp_config) as client:
        yield client


@pytest.fixture(scope="session")
def rtu_port(pytestconfig: pytest.Config) -> str:
    return pytestconfig.getoption("--rtu-port")


@pytest.fixture(scope="session")
def rtu_baudrate(pytestconfig: pytest.Config) -> int:
    return pytestconfig.getoption("--rtu-baudrate")


@pytest.fixture(scope="session")
def rtu_address(pytestconfig: pytest.Config) -> int:
    return pytestconfig.getoption("--rtu-address")


def pytest_runtest_setup(item: pytest.Item) -> None:
    LOGGER.info("[用例开始] %s", item.nodeid)


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item: pytest.Item, call: pytest.CallInfo):
    outcome = yield
    report = outcome.get_result()
    if report.when != "call":
        return
    LOGGER.info("[用例结果] %s -> %s", item.nodeid, report.outcome.upper())