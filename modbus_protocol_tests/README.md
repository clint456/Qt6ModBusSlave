# Modbus 协议回归测试

这个文件夹用于对服务器做协议级回归测试，目标是一次性验证标准 Modbus 行为是否符合预期。

## 覆盖范围

### TCP 标准功能码
- 01 读线圈
- 02 读离散输入
- 03 读保持寄存器
- 04 读输入寄存器
- 05 写单个线圈
- 06 写单个寄存器
- 15 写多个线圈
- 16 写多个寄存器

当前回归套件已增加：
- 各功能码数量上下限边界验证
- 地址边界与越界异常验证
- 非法字节计数与短帧请求验证
- 未实现标准功能码（0x07/0x08/0x0B/0x0C/0x11/0x16/0x17）异常验证
- MBAP 事务号回显序列验证

### 协议验证项
- MBAP 头字段是否正确
- Function Code 是否正确
- 异常码是否正确
- 地址越界 / 非法值 / 数量错误是否按标准返回异常
- 写后读回是否一致
- RTU CRC 是否正确

### 不纳入标准判断
- 20 / 21 文件记录
- 203 / 204 自定义功能码

## 运行前提

1. 先启动你的 Modbus 服务器。
2. 默认 TCP 端口为 `502`，Unit ID 默认 `1`。
3. RTU 测试是可选的，需要实际串口设备。

## 安装依赖

```bash
python3 -m pip install -r modbus_protocol_tests/requirements.txt
```

## 一键运行 TCP + RTU

```bash
python3 modbus_protocol_tests/run_all.py --host 127.0.0.1 --port 502
```

可选日志级别（默认 `INFO`）：

```bash
python3 modbus_protocol_tests/run_all.py --host 127.0.0.1 --port 502 --log-level DEBUG
```

如果要同时测 RTU：

```bash
python3 modbus_protocol_tests/run_all.py --host 127.0.0.1 --port 502 --rtu-port /dev/ttyUSB0 --rtu-baudrate 9600 --rtu-address 1
```

## 单独运行 TCP

```bash
python3 modbus_protocol_tests/tcp_regression.py --host 127.0.0.1 --port 502 --unit-id 1
```

## 单独运行 RTU

```bash
python3 modbus_protocol_tests/rtu_regression.py --port /dev/ttyUSB0 --baudrate 9600 --address 1
```

## 结果判定

- 脚本正常退出，说明协议基础行为通过。
- 任一断言失败，会直接抛出错误并返回非零退出码。
- 建议把这套脚本作为每次改动 Modbus 协议层之后的回归标准。

## 报告输出

执行 `run_all.py` 后会生成三份报告：

- `modbus_protocol_tests/reports/modbus_protocol_report.xml`
	- pytest 的 JUnit XML 原始报告
- `modbus_protocol_tests/reports/modbus_protocol_run.log`
	- pytest 运行日志（包含用例开始/结束、Modbus TCP 请求与响应 PDU）
- `modbus_protocol_tests/reports/modbus_protocol_report_detailed.md`
	- 详细可读报告：每个测试用例、覆盖内容、是否通过、断言错误输出
- `modbus_protocol_tests/reports/modbus_protocol_report_detailed.json`
	- 结构化明细，适合后续自动化统计/看板接入

## 标准一致性矩阵

- `modbus_protocol_tests/MODBUS_COMPLIANCE_MATRIX.md`
	- 说明当前实现范围内已覆盖的标准条目
	- 列出未实现或未纳入自动化的标准项，便于后续补齐
