/**
 * @file ModbusValueConverter.cpp
 * @brief Modbus 数据类型转换器实现
 */

#include "ModbusValueConverter.h"
#include <QDebug>

// ==================== 类型信息查询 ====================

int ModbusValueConverter::registerCount(ModbusDataValueType type)
{
    switch (type) {
    case ModbusDataValueType::BOOL:
    case ModbusDataValueType::INT8:
    case ModbusDataValueType::UINT8:
    case ModbusDataValueType::INT16:
    case ModbusDataValueType::UINT16:
        return 1;
    case ModbusDataValueType::INT32:
    case ModbusDataValueType::UINT32:
    case ModbusDataValueType::FLOAT32:
        return 2;
    case ModbusDataValueType::INT64:
    case ModbusDataValueType::UINT64:
    case ModbusDataValueType::FLOAT64:
        return 4;
    }
    return 1;
}

ModbusDataValueType ModbusValueConverter::parseTypeString(const QString &typeStr)
{
    QString upper = typeStr.toUpper().trimmed();
    
    if (upper == "BOOL" || upper == "BOOLEAN") {
        return ModbusDataValueType::BOOL;
    } else if (upper == "INT8" || upper == "SINT8") {
        return ModbusDataValueType::INT8;
    } else if (upper == "UINT8" || upper == "BYTE") {
        return ModbusDataValueType::UINT8;
    } else if (upper == "INT16" || upper == "SHORT" || upper == "SINT16") {
        return ModbusDataValueType::INT16;
    } else if (upper == "UINT16" || upper == "WORD" || upper == "USHORT") {
        return ModbusDataValueType::UINT16;
    } else if (upper == "INT32" || upper == "INT" || upper == "SINT32" || upper == "DINT") {
        return ModbusDataValueType::INT32;
    } else if (upper == "UINT32" || upper == "DWORD" || upper == "UDINT") {
        return ModbusDataValueType::UINT32;
    } else if (upper == "INT64" || upper == "LONG" || upper == "SINT64" || upper == "LINT") {
        return ModbusDataValueType::INT64;
    } else if (upper == "UINT64" || upper == "ULONG" || upper == "QWORD" || upper == "ULINT") {
        return ModbusDataValueType::UINT64;
    } else if (upper == "FLOAT32" || upper == "FLOAT" || upper == "REAL") {
        return ModbusDataValueType::FLOAT32;
    } else if (upper == "FLOAT64" || upper == "DOUBLE" || upper == "LREAL") {
        return ModbusDataValueType::FLOAT64;
    }
    
    // 默认返回 UINT16
    return ModbusDataValueType::UINT16;
}

QString ModbusValueConverter::typeToString(ModbusDataValueType type)
{
    switch (type) {
    case ModbusDataValueType::BOOL:    return "BOOL";
    case ModbusDataValueType::INT8:    return "INT8";
    case ModbusDataValueType::UINT8:   return "UINT8";
    case ModbusDataValueType::INT16:   return "INT16";
    case ModbusDataValueType::UINT16:  return "UINT16";
    case ModbusDataValueType::INT32:   return "INT32";
    case ModbusDataValueType::UINT32:  return "UINT32";
    case ModbusDataValueType::INT64:   return "INT64";
    case ModbusDataValueType::UINT64:  return "UINT64";
    case ModbusDataValueType::FLOAT32: return "FLOAT32";
    case ModbusDataValueType::FLOAT64: return "FLOAT64";
    }
    return "UINT16";
}

// ==================== 值到寄存器转换 ====================

bool ModbusValueConverter::valueToRegisters(const QVariant &value, ModbusDataValueType type,
                                             QVector<quint16> &registers)
{
    registers.clear();
    int count = registerCount(type);
    registers.resize(count);
    
    bool ok = true;
    
    switch (type) {
    case ModbusDataValueType::BOOL:
        boolToRegister(value.toBool(), registers[0]);
        break;
    case ModbusDataValueType::INT8:
        int8ToRegister(static_cast<qint8>(value.toInt(&ok)), registers[0]);
        break;
    case ModbusDataValueType::UINT8:
        uint8ToRegister(static_cast<quint8>(value.toUInt(&ok)), registers[0]);
        break;
    case ModbusDataValueType::INT16:
        int16ToRegister(static_cast<qint16>(value.toInt(&ok)), registers[0]);
        break;
    case ModbusDataValueType::UINT16:
        uint16ToRegister(static_cast<quint16>(value.toUInt(&ok)), registers[0]);
        break;
    case ModbusDataValueType::INT32:
        int32ToRegisters(value.toInt(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::UINT32:
        uint32ToRegisters(value.toUInt(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::INT64:
        int64ToRegisters(value.toLongLong(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::UINT64:
        uint64ToRegisters(value.toULongLong(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::FLOAT32:
        float32ToRegisters(value.toFloat(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::FLOAT64:
        float64ToRegisters(value.toDouble(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    }
    
    return ok;
}

bool ModbusValueConverter::stringToRegisters(const QString &valueStr, ModbusDataValueType type,
                                              QVector<quint16> &registers)
{
    registers.clear();
    int count = registerCount(type);
    registers.resize(count);
    
    bool ok = true;
    
    switch (type) {
    case ModbusDataValueType::BOOL: {
        QString lower = valueStr.toLower().trimmed();
        bool boolVal = (lower == "true" || lower == "1" || lower == "on" || lower == "是");
        boolToRegister(boolVal, registers[0]);
        break;
    }
    case ModbusDataValueType::INT8:
        int8ToRegister(static_cast<qint8>(valueStr.toShort(&ok)), registers[0]);
        break;
    case ModbusDataValueType::UINT8:
        uint8ToRegister(static_cast<quint8>(valueStr.toUShort(&ok)), registers[0]);
        break;
    case ModbusDataValueType::INT16:
        int16ToRegister(valueStr.toShort(&ok), registers[0]);
        break;
    case ModbusDataValueType::UINT16:
        uint16ToRegister(valueStr.toUShort(&ok), registers[0]);
        break;
    case ModbusDataValueType::INT32:
        int32ToRegisters(valueStr.toInt(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::UINT32:
        uint32ToRegisters(valueStr.toUInt(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::INT64:
        int64ToRegisters(valueStr.toLongLong(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::UINT64:
        uint64ToRegisters(valueStr.toULongLong(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::FLOAT32:
        float32ToRegisters(valueStr.toFloat(&ok), registers[0], registers[1]);
        break;
    case ModbusDataValueType::FLOAT64:
        float64ToRegisters(valueStr.toDouble(&ok), registers[0], registers[1], registers[2], registers[3]);
        break;
    }
    
    return ok;
}

// ==================== 寄存器到值转换 ====================

QVariant ModbusValueConverter::registersToValue(const QVector<quint16> &registers, ModbusDataValueType type)
{
    if (registers.isEmpty()) {
        return QVariant();
    }
    
    switch (type) {
    case ModbusDataValueType::BOOL:
        return registerToBool(registers[0]);
    case ModbusDataValueType::INT8:
        return registerToInt8(registers[0]);
    case ModbusDataValueType::UINT8:
        return registerToUint8(registers[0]);
    case ModbusDataValueType::INT16:
        return registerToInt16(registers[0]);
    case ModbusDataValueType::UINT16:
        return registerToUint16(registers[0]);
    case ModbusDataValueType::INT32:
        if (registers.size() >= 2)
            return registersToInt32(registers[0], registers[1]);
        break;
    case ModbusDataValueType::UINT32:
        if (registers.size() >= 2)
            return registersToUint32(registers[0], registers[1]);
        break;
    case ModbusDataValueType::INT64:
        if (registers.size() >= 4)
            return registersToInt64(registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::UINT64:
        if (registers.size() >= 4)
            return registersToUint64(registers[0], registers[1], registers[2], registers[3]);
        break;
    case ModbusDataValueType::FLOAT32:
        if (registers.size() >= 2)
            return registersToFloat32(registers[0], registers[1]);
        break;
    case ModbusDataValueType::FLOAT64:
        if (registers.size() >= 4)
            return registersToFloat64(registers[0], registers[1], registers[2], registers[3]);
        break;
    }
    
    return QVariant();
}

QString ModbusValueConverter::registersToString(const QVector<quint16> &registers, ModbusDataValueType type)
{
    QVariant value = registersToValue(registers, type);
    
    if (!value.isValid()) {
        return QString();
    }
    
    switch (type) {
    case ModbusDataValueType::BOOL:
        return value.toBool() ? "true" : "false";
    case ModbusDataValueType::FLOAT32:
        return QString::number(value.toFloat(), 'g', 7);
    case ModbusDataValueType::FLOAT64:
        return QString::number(value.toDouble(), 'g', 15);
    default:
        return value.toString();
    }
}

// ==================== 具体类型转换 - 值到寄存器 ====================

void ModbusValueConverter::boolToRegister(bool value, quint16 &reg)
{
    reg = value ? 0xFF00 : 0x0000;
}

void ModbusValueConverter::int8ToRegister(qint8 value, quint16 &reg)
{
    // 存储在寄存器的低8位，符号扩展到16位
    reg = static_cast<quint16>(static_cast<qint16>(value));
}

void ModbusValueConverter::uint8ToRegister(quint8 value, quint16 &reg)
{
    reg = static_cast<quint16>(value);
}

void ModbusValueConverter::int16ToRegister(qint16 value, quint16 &reg)
{
    reg = static_cast<quint16>(value);
}

void ModbusValueConverter::uint16ToRegister(quint16 value, quint16 &reg)
{
    reg = value;
}

void ModbusValueConverter::int32ToRegisters(qint32 value, quint16 &high, quint16 &low)
{
    high = static_cast<quint16>((value >> 16) & 0xFFFF);
    low = static_cast<quint16>(value & 0xFFFF);
}

void ModbusValueConverter::uint32ToRegisters(quint32 value, quint16 &high, quint16 &low)
{
    high = static_cast<quint16>((value >> 16) & 0xFFFF);
    low = static_cast<quint16>(value & 0xFFFF);
}

void ModbusValueConverter::int64ToRegisters(qint64 value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3)
{
    r0 = static_cast<quint16>((value >> 48) & 0xFFFF);
    r1 = static_cast<quint16>((value >> 32) & 0xFFFF);
    r2 = static_cast<quint16>((value >> 16) & 0xFFFF);
    r3 = static_cast<quint16>(value & 0xFFFF);
}

void ModbusValueConverter::uint64ToRegisters(quint64 value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3)
{
    r0 = static_cast<quint16>((value >> 48) & 0xFFFF);
    r1 = static_cast<quint16>((value >> 32) & 0xFFFF);
    r2 = static_cast<quint16>((value >> 16) & 0xFFFF);
    r3 = static_cast<quint16>(value & 0xFFFF);
}

void ModbusValueConverter::float32ToRegisters(float value, quint16 &high, quint16 &low)
{
    quint32 intValue;
    std::memcpy(&intValue, &value, sizeof(float));
    high = static_cast<quint16>((intValue >> 16) & 0xFFFF);
    low = static_cast<quint16>(intValue & 0xFFFF);
}

void ModbusValueConverter::float64ToRegisters(double value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3)
{
    quint64 intValue;
    std::memcpy(&intValue, &value, sizeof(double));
    r0 = static_cast<quint16>((intValue >> 48) & 0xFFFF);
    r1 = static_cast<quint16>((intValue >> 32) & 0xFFFF);
    r2 = static_cast<quint16>((intValue >> 16) & 0xFFFF);
    r3 = static_cast<quint16>(intValue & 0xFFFF);
}

// ==================== 具体类型转换 - 寄存器到值 ====================

bool ModbusValueConverter::registerToBool(quint16 reg)
{
    return reg != 0;
}

qint8 ModbusValueConverter::registerToInt8(quint16 reg)
{
    return static_cast<qint8>(reg & 0xFF);
}

quint8 ModbusValueConverter::registerToUint8(quint16 reg)
{
    return static_cast<quint8>(reg & 0xFF);
}

qint16 ModbusValueConverter::registerToInt16(quint16 reg)
{
    return static_cast<qint16>(reg);
}

quint16 ModbusValueConverter::registerToUint16(quint16 reg)
{
    return reg;
}

qint32 ModbusValueConverter::registersToInt32(quint16 high, quint16 low)
{
    return static_cast<qint32>((static_cast<quint32>(high) << 16) | low);
}

quint32 ModbusValueConverter::registersToUint32(quint16 high, quint16 low)
{
    return (static_cast<quint32>(high) << 16) | low;
}

qint64 ModbusValueConverter::registersToInt64(quint16 r0, quint16 r1, quint16 r2, quint16 r3)
{
    return static_cast<qint64>(
        (static_cast<quint64>(r0) << 48) |
        (static_cast<quint64>(r1) << 32) |
        (static_cast<quint64>(r2) << 16) |
        r3
    );
}

quint64 ModbusValueConverter::registersToUint64(quint16 r0, quint16 r1, quint16 r2, quint16 r3)
{
    return (static_cast<quint64>(r0) << 48) |
           (static_cast<quint64>(r1) << 32) |
           (static_cast<quint64>(r2) << 16) |
           r3;
}

float ModbusValueConverter::registersToFloat32(quint16 high, quint16 low)
{
    quint32 intValue = (static_cast<quint32>(high) << 16) | low;
    float value;
    std::memcpy(&value, &intValue, sizeof(float));
    return value;
}

double ModbusValueConverter::registersToFloat64(quint16 r0, quint16 r1, quint16 r2, quint16 r3)
{
    quint64 intValue = (static_cast<quint64>(r0) << 48) |
                       (static_cast<quint64>(r1) << 32) |
                       (static_cast<quint64>(r2) << 16) |
                       r3;
    double value;
    std::memcpy(&value, &intValue, sizeof(double));
    return value;
}
