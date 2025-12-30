# 快速开始指南

## 第一步：构建项目

### 使用 Qt Creator（推荐）
1. 打开 Qt Creator
2. 文件 -> 打开文件或项目
3. 选择 `CMakeLists.txt`
4. 选择 Qt 6.10+ kit
5. 点击"配置项目"
6. 点击"构建"按钮（绿色三角形）

### 使用命令行
```bash
cd d:\BaiduSyncdisk\work\other\Qt6\Qt6ModBusSlave
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 第二步：运行程序

构建完成后，点击"运行"按钮或执行生成的可执行文件。

## 第三步：启动服务器

### 快速启动 TCP 服务器
1. 程序启动后会显示主界面
2. 在"TCP 端口"输入框确认端口为 `502`
3. 点击"初始化数据"按钮（初始化测试数据）
4. 点击"启动 TCP"按钮
5. 状态应显示"TCP 服务器运行中 (端口 502)"

## 第四步：测试连接

### 使用 Python 测试
安装 pymodbus:
```bash
pip install pymodbus
```

创建测试脚本 `test_modbus.py`:
```python
from pymodbus.client import ModbusTcpClient

# 连接服务器
client = ModbusTcpClient('127.0.0.1', port=502)
client.connect()

# 测试读保持寄存器
print("读取保持寄存器 0-9:")
result = client.read_holding_registers(0, 10, slave=1)
if not result.isError():
    print(f"  数据: {result.registers}")
else:
    print(f"  错误: {result}")

# 测试写单个寄存器
print("\n写入寄存器 0 = 1234")
client.write_register(0, 1234, slave=1)

# 再次读取验证
result = client.read_holding_registers(0, 1, slave=1)
print(f"  验证: {result.registers[0]}")

# 测试读线圈
print("\n读取线圈 0-9:")
result = client.read_coils(0, 10, slave=1)
if not result.isError():
    print(f"  数据: {result.bits[:10]}")

# 测试写多个寄存器
print("\n写入寄存器 10-12 = [100, 200, 300]")
client.write_registers(10, [100, 200, 300], slave=1)
result = client.read_holding_registers(10, 3, slave=1)
print(f"  验证: {result.registers}")

client.close()
print("\n测试完成！")
```

运行测试:
```bash
python test_modbus.py
```

预期输出:
```
读取保持寄存器 0-9:
  数据: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

写入寄存器 0 = 1234
  验证: 1234

读取线圈 0-9:
  数据: [False, False, False, False, False, False, False, False, False, False]

写入寄存器 10-12 = [100, 200, 300]
  验证: [100, 200, 300]

测试完成！
```

## 第五步：监控数据

在程序界面的"数据监控"区域：
1. 选择"保持寄存器"
2. 起始地址输入 `0`
3. 数量输入 `20`
4. 点击"刷新"

您应该看到：
- 地址 0 的值是 1234（刚才写入的）
- 地址 10-12 的值是 100, 200, 300

## 常见功能码示例

### 功能码 01 - 读线圈
```python
result = client.read_coils(0, 10, slave=1)
print(result.bits[:10])
```

### 功能码 03 - 读保持寄存器
```python
result = client.read_holding_registers(0, 10, slave=1)
print(result.registers)
```

### 功能码 05 - 写单个线圈
```python
client.write_coil(0, True, slave=1)
```

### 功能码 06 - 写单个寄存器
```python
client.write_register(0, 1234, slave=1)
```

### 功能码 15 - 写多个线圈
```python
client.write_coils(0, [True, False, True, True], slave=1)
```

### 功能码 16 - 写多个寄存器
```python
client.write_registers(0, [100, 200, 300, 400], slave=1)
```

## RTU 模式测试

如果您有串口设备：

1. 在界面选择 RTU 模式
2. 输入串口名称（Windows: COM1, Linux: /dev/ttyUSB0）
3. 选择波特率（通常 9600 或 19200）
4. 点击"启动 RTU"

使用 pymodbus 测试 RTU:
```python
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(
    port='COM1',          # Windows
    # port='/dev/ttyUSB0', # Linux
    baudrate=9600,
    parity='N',
    stopbits=1,
    bytesize=8,
    timeout=1
)

client.connect()
result = client.read_holding_registers(0, 10, slave=1)
print(result.registers)
client.close()
```

## 故障排除

### 问题：无法连接到 TCP 服务器
- 检查防火墙是否阻止端口 502
- Windows: 以管理员身份运行（端口 < 1024 需要权限）
- 或使用其他端口（如 5020）

### 问题：串口无法打开
- 检查串口名称是否正确
- 确保串口未被其他程序占用
- 检查串口权限（Linux: `sudo chmod 666 /dev/ttyUSB0`）

### 问题：读取数据为空
- 确保已点击"初始化数据"按钮
- 检查地址范围是否正确
- 查看日志区域的错误消息

## 下一步

- 阅读完整的 [README.md](README.md) 了解所有功能
- 尝试文件寄存器功能（功能码 20/21）
- 自定义数据初始化范围
- 集成到您的项目中

祝使用愉快！
