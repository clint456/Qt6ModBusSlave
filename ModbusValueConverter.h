/**
 * @file ModbusValueConverter.h
 * @brief Modbus 数据类型转换器
 * 
 * 提供各种数据类型与 Modbus 寄存器之间的转换功能
 * 支持: BOOL, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, FLOAT32, FLOAT64
 */

#ifndef MODBUSVALUECONVERTER_H
#define MODBUSVALUECONVERTER_H

#include <QtGlobal>
#include <QVector>
#include <QString>
#include <QVariant>
#include <cstring>

/**
 * @brief 数据类型枚举
 */
enum class ModbusDataValueType {
    BOOL,       // 布尔值（线圈/离散输入）
    INT8,       // 8位有符号整数 (-128 to 127)
    UINT8,      // 8位无符号整数 (0 to 255)
    INT16,      // 16位有符号整数 (-32768 to 32767)
    UINT16,     // 16位无符号整数 (0 to 65535)
    INT32,      // 32位有符号整数 (需要2个寄存器)
    UINT32,     // 32位无符号整数 (需要2个寄存器)
    INT64,      // 64位有符号整数 (需要4个寄存器)
    UINT64,     // 64位无符号整数 (需要4个寄存器)
    FLOAT32,    // 32位浮点数 (需要2个寄存器)
    FLOAT64     // 64位双精度浮点数 (需要4个寄存器)
};

/**
 * @brief Modbus 值转换器 - 提供类型安全的值与寄存器转换
 * 
 * 使用静态方法设计，无需实例化
 */
class ModbusValueConverter
{
public:
    // ==================== 类型信息查询 ====================
    
    /**
     * @brief 获取数据类型占用的寄存器数量
     * @param type 数据类型
     * @return 寄存器数量 (1, 2, 或 4)
     */
    static int registerCount(ModbusDataValueType type);
    
    /**
     * @brief 从字符串解析数据类型
     * @param typeStr 类型字符串 (如 "INT16", "FLOAT32")
     * @return 对应的枚举值，默认返回 UINT16
     */
    static ModbusDataValueType parseTypeString(const QString &typeStr);
    
    /**
     * @brief 将数据类型转为字符串
     * @param type 数据类型枚举
     * @return 类型字符串
     */
    static QString typeToString(ModbusDataValueType type);
    
    // ==================== 值到寄存器转换 ====================
    
    /**
     * @brief 将 QVariant 值转换为寄存器数组
     * @param value 输入值
     * @param type 数据类型
     * @param registers 输出寄存器数组
     * @return 转换是否成功
     */
    static bool valueToRegisters(const QVariant &value, ModbusDataValueType type, 
                                  QVector<quint16> &registers);
    
    /**
     * @brief 将字符串值转换为寄存器数组
     * @param valueStr 输入值字符串
     * @param type 数据类型
     * @param registers 输出寄存器数组
     * @return 转换是否成功
     */
    static bool stringToRegisters(const QString &valueStr, ModbusDataValueType type,
                                   QVector<quint16> &registers);
    
    // ==================== 寄存器到值转换 ====================
    
    /**
     * @brief 将寄存器数组转换为 QVariant 值
     * @param registers 输入寄存器数组
     * @param type 数据类型
     * @return 转换后的值
     */
    static QVariant registersToValue(const QVector<quint16> &registers, ModbusDataValueType type);
    
    /**
     * @brief 将寄存器数组转换为字符串
     * @param registers 输入寄存器数组
     * @param type 数据类型
     * @return 值的字符串表示
     */
    static QString registersToString(const QVector<quint16> &registers, ModbusDataValueType type);
    
    // ==================== 具体类型转换 - 值到寄存器 ====================
    
    static void boolToRegister(bool value, quint16 &reg);
    static void int8ToRegister(qint8 value, quint16 &reg);
    static void uint8ToRegister(quint8 value, quint16 &reg);
    static void int16ToRegister(qint16 value, quint16 &reg);
    static void uint16ToRegister(quint16 value, quint16 &reg);
    static void int32ToRegisters(qint32 value, quint16 &high, quint16 &low);
    static void uint32ToRegisters(quint32 value, quint16 &high, quint16 &low);
    static void int64ToRegisters(qint64 value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3);
    static void uint64ToRegisters(quint64 value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3);
    static void float32ToRegisters(float value, quint16 &high, quint16 &low);
    static void float64ToRegisters(double value, quint16 &r0, quint16 &r1, quint16 &r2, quint16 &r3);
    
    // ==================== 具体类型转换 - 寄存器到值 ====================
    
    static bool registerToBool(quint16 reg);
    static qint8 registerToInt8(quint16 reg);
    static quint8 registerToUint8(quint16 reg);
    static qint16 registerToInt16(quint16 reg);
    static quint16 registerToUint16(quint16 reg);
    static qint32 registersToInt32(quint16 high, quint16 low);
    static quint32 registersToUint32(quint16 high, quint16 low);
    static qint64 registersToInt64(quint16 r0, quint16 r1, quint16 r2, quint16 r3);
    static quint64 registersToUint64(quint16 r0, quint16 r1, quint16 r2, quint16 r3);
    static float registersToFloat32(quint16 high, quint16 low);
    static double registersToFloat64(quint16 r0, quint16 r1, quint16 r2, quint16 r3);

private:
    ModbusValueConverter() = delete;  // 禁止实例化
};

#endif // MODBUSVALUECONVERTER_H
