#ifndef MODBUSTYPES_H
#define MODBUSTYPES_H

#include <QtGlobal>

// Modbus 功能码
enum ModbusFunctionCode : quint8 {
    ReadCoils = 0x01,               // 读线圈
    ReadDiscreteInputs = 0x02,      // 读离散输入
    ReadHoldingRegisters = 0x03,    // 读保持寄存器
    ReadInputRegisters = 0x04,      // 读输入寄存器
    WriteSingleCoil = 0x05,         // 写单个线圈
    WriteSingleRegister = 0x06,     // 写单个寄存器
    WriteMultipleCoils = 0x0F,      // 写多个线圈 (15)
    WriteMultipleRegisters = 0x10,  // 写多个寄存器 (16)
    ReadFileRecord = 0x14,          // 读文件记录 (20)
    WriteFileRecord = 0x15,         // 写文件记录 (21)
    ReadFile = 0xCB,                // 自定义读文件 (203)
    WriteFile = 0xCC                // 自定义写文件 (204)
};

// Modbus 异常码
enum ModbusExceptionCode : quint8 {
    IllegalFunction = 0x01,         // 非法功能
    IllegalDataAddress = 0x02,      // 非法数据地址
    IllegalDataValue = 0x03,        // 非法数据值
    SlaveDeviceFailure = 0x04,      // 从站设备故障
    Acknowledge = 0x05,             // 确认
    SlaveDeviceBusy = 0x06,         // 从站设备忙
    MemoryParityError = 0x08        // 内存奇偶性错误
};

// Modbus 通信模式
enum ModbusMode {
    ModeTCP,
    ModeRTU
};

// Modbus 数据区类型
enum ModbusDataType {
    DataTypeCoil,           // 线圈
    DataTypeDiscreteInput,  // 离散输入
    DataTypeHoldingRegister,// 保持寄存器
    DataTypeInputRegister   // 输入寄存器
};

// 常量定义
namespace ModbusConst {
    constexpr quint16 MAX_COILS = 65536;
    constexpr quint16 MAX_REGISTERS = 65536;
    constexpr quint16 MAX_READ_COILS = 2000;
    constexpr quint16 MAX_READ_REGISTERS = 125;
    constexpr quint16 MAX_WRITE_COILS = 1968;
    constexpr quint16 MAX_WRITE_REGISTERS = 123;
    constexpr quint16 MAX_FILE_RECORDS = 10000;
}

#endif // MODBUSTYPES_H
