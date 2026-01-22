/**
 * @file SensorItem.h
 * @brief 传感器数据项定义
 * 
 * 定义Modbus传感器/点位的数据结构，包含地址、类型、值等完整信息
 */

#ifndef SENSORITEM_H
#define SENSORITEM_H

#include <QString>
#include <QVariant>
#include <QtGlobal>
#include "ModbusValueConverter.h"

/**
 * @brief Modbus 点类型枚举
 */
enum class SensorPointType {
    Coil,               // 线圈 (读/写, 1位)
    DiscreteInput,      // 离散输入 (只读, 1位)
    HoldingRegister,    // 保持寄存器 (读/写, 16位)
    InputRegister       // 输入寄存器 (只读, 16位)
};

/**
 * @brief 传感器/点位数据项
 * 
 * 表示一个Modbus点位的完整配置信息
 */
class SensorItem
{
public:
    // ==================== 构造函数 ====================
    
    SensorItem();
    
    /**
     * @brief 完整构造函数
     */
    SensorItem(quint16 addr, const QString &name, SensorPointType pointType,
               ModbusDataValueType valueType, const QString &initVal = QString());
    
    // ==================== 静态工厂方法 ====================
    
    /**
     * @brief 从字符串创建点类型
     * @param typeStr 类型字符串 (如 "线圈", "保持寄存器")
     * @return 点类型枚举
     */
    static SensorPointType parsePointType(const QString &typeStr);
    
    /**
     * @brief 将点类型转为字符串
     * @param type 点类型枚举
     * @return 中文类型字符串
     */
    static QString pointTypeToString(SensorPointType type);
    
    /**
     * @brief 根据点类型判断是否为只读类型
     * @param type 点类型
     * @return 是否只读
     */
    static bool isReadOnlyType(SensorPointType type);
    
    // ==================== 属性访问器 ====================
    
    quint16 address() const { return m_address; }
    void setAddress(quint16 addr) { m_address = addr; }
    
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    
    SensorPointType pointType() const { return m_pointType; }
    void setPointType(SensorPointType type) { m_pointType = type; }
    QString pointTypeString() const { return pointTypeToString(m_pointType); }
    
    ModbusDataValueType valueType() const { return m_valueType; }
    void setValueType(ModbusDataValueType type);
    QString valueTypeString() const { return ModbusValueConverter::typeToString(m_valueType); }
    
    QString initialValue() const { return m_initialValue; }
    void setInitialValue(const QString &value) { m_initialValue = value; }
    
    QString description() const { return m_description; }
    void setDescription(const QString &desc) { m_description = desc; }
    
    QString unit() const { return m_unit; }
    void setUnit(const QString &unit) { m_unit = unit; }
    
    QString minValue() const { return m_minValue; }
    void setMinValue(const QString &min) { m_minValue = min; }
    
    QString maxValue() const { return m_maxValue; }
    void setMaxValue(const QString &max) { m_maxValue = max; }
    
    bool isReadOnly() const { return m_readOnly; }
    void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
    
    int registerCount() const { return m_registerCount; }
    void setRegisterCount(int count) { m_registerCount = count; }
    
    // ==================== 辅助方法 ====================
    
    /**
     * @brief 检查配置是否有效
     * @return 是否有效
     */
    bool isValid() const;
    
    /**
     * @brief 获取此点位占用的地址范围 [startAddr, endAddr]
     * @return QPair<起始地址, 结束地址>
     */
    QPair<quint16, quint16> addressRange() const;
    
    /**
     * @brief 将初始值转换为寄存器数组
     * @param registers 输出寄存器数组
     * @return 转换是否成功
     */
    bool toRegisters(QVector<quint16> &registers) const;
    
    /**
     * @brief 转换为 QVariantMap (用于QML)
     * @return 包含所有属性的Map
     */
    QVariantMap toVariantMap() const;
    
    /**
     * @brief 从 QVariantMap 创建 (用于QML)
     * @param map 属性Map
     * @return SensorItem 对象
     */
    static SensorItem fromVariantMap(const QVariantMap &map);

private:
    quint16 m_address;                  // Modbus地址
    QString m_name;                     // 点位名称
    SensorPointType m_pointType;        // 点类型
    ModbusDataValueType m_valueType;    // 值类型
    QString m_initialValue;             // 初始值
    QString m_description;              // 描述
    QString m_unit;                     // 单位
    QString m_minValue;                 // 最小值
    QString m_maxValue;                 // 最大值
    bool m_readOnly;                    // 只读标志
    int m_registerCount;                // 占用寄存器数量
};

#endif // SENSORITEM_H
