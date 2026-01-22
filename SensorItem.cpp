/**
 * @file SensorItem.cpp
 * @brief 传感器数据项实现
 */

#include "SensorItem.h"

// ==================== 构造函数 ====================

SensorItem::SensorItem()
    : m_address(0)
    , m_pointType(SensorPointType::HoldingRegister)
    , m_valueType(ModbusDataValueType::UINT16)
    , m_readOnly(false)
    , m_registerCount(1)
{
}

SensorItem::SensorItem(quint16 addr, const QString &name, SensorPointType pointType,
                       ModbusDataValueType valueType, const QString &initVal)
    : m_address(addr)
    , m_name(name)
    , m_pointType(pointType)
    , m_valueType(valueType)
    , m_initialValue(initVal)
    , m_readOnly(isReadOnlyType(pointType))
    , m_registerCount(ModbusValueConverter::registerCount(valueType))
{
}

// ==================== 静态工厂方法 ====================

SensorPointType SensorItem::parsePointType(const QString &typeStr)
{
    QString trimmed = typeStr.trimmed();
    
    if (trimmed.contains("线圈") || trimmed.toLower() == "coil") {
        return SensorPointType::Coil;
    } else if (trimmed.contains("离散") || trimmed.contains("高载") || 
               trimmed.toLower() == "discreteinput" || trimmed.toLower() == "di") {
        return SensorPointType::DiscreteInput;
    } else if (trimmed.contains("保持") || 
               trimmed.toLower() == "holdingregister" || trimmed.toLower() == "hr") {
        return SensorPointType::HoldingRegister;
    } else if (trimmed.contains("输入寄存器") || trimmed.contains("输入") ||
               trimmed.toLower() == "inputregister" || trimmed.toLower() == "ir") {
        return SensorPointType::InputRegister;
    }
    
    // 默认返回保持寄存器
    return SensorPointType::HoldingRegister;
}

QString SensorItem::pointTypeToString(SensorPointType type)
{
    switch (type) {
    case SensorPointType::Coil:
        return "线圈";
    case SensorPointType::DiscreteInput:
        return "离散输入";
    case SensorPointType::HoldingRegister:
        return "保持寄存器";
    case SensorPointType::InputRegister:
        return "输入寄存器";
    }
    return "保持寄存器";
}

bool SensorItem::isReadOnlyType(SensorPointType type)
{
    return (type == SensorPointType::DiscreteInput || 
            type == SensorPointType::InputRegister);
}

// ==================== 属性设置器 ====================

void SensorItem::setValueType(ModbusDataValueType type)
{
    m_valueType = type;
    m_registerCount = ModbusValueConverter::registerCount(type);
}

// ==================== 辅助方法 ====================

bool SensorItem::isValid() const
{
    return !m_name.isEmpty();
}

QPair<quint16, quint16> SensorItem::addressRange() const
{
    return qMakePair(m_address, static_cast<quint16>(m_address + m_registerCount - 1));
}

bool SensorItem::toRegisters(QVector<quint16> &registers) const
{
    return ModbusValueConverter::stringToRegisters(m_initialValue, m_valueType, registers);
}

QVariantMap SensorItem::toVariantMap() const
{
    QVariantMap map;
    map["address"] = m_address;
    map["index"] = m_address;  // 兼容旧版QML
    map["name"] = m_name;
    map["pointName"] = m_name;  // 兼容旧版
    map["pointType"] = pointTypeToString(m_pointType);
    map["pointTypeEnum"] = static_cast<int>(m_pointType);
    map["valueType"] = valueTypeString();
    map["valueTypeEnum"] = static_cast<int>(m_valueType);
    map["dataType"] = static_cast<int>(m_valueType);  // 兼容旧版
    map["initialValue"] = m_initialValue;
    map["description"] = m_description;
    map["note"] = m_description;  // 兼容旧版
    map["unit"] = m_unit;
    map["minValue"] = m_minValue;
    map["maxValue"] = m_maxValue;
    map["readOnly"] = m_readOnly;
    map["registerCount"] = m_registerCount;
    return map;
}

SensorItem SensorItem::fromVariantMap(const QVariantMap &map)
{
    SensorItem item;
    
    item.m_address = static_cast<quint16>(map.value("address", 0).toUInt());
    item.m_name = map.value("name", map.value("pointName")).toString();
    
    // 解析点类型
    if (map.contains("pointTypeEnum")) {
        item.m_pointType = static_cast<SensorPointType>(map["pointTypeEnum"].toInt());
    } else {
        item.m_pointType = parsePointType(map.value("pointType").toString());
    }
    
    // 解析值类型
    if (map.contains("valueTypeEnum")) {
        item.m_valueType = static_cast<ModbusDataValueType>(map["valueTypeEnum"].toInt());
    } else {
        item.m_valueType = ModbusValueConverter::parseTypeString(map.value("valueType").toString());
    }
    
    item.m_initialValue = map.value("initialValue").toString();
    item.m_description = map.value("description").toString();
    item.m_unit = map.value("unit").toString();
    item.m_minValue = map.value("minValue").toString();
    item.m_maxValue = map.value("maxValue").toString();
    item.m_readOnly = map.value("readOnly", false).toBool();
    item.m_registerCount = ModbusValueConverter::registerCount(item.m_valueType);
    
    return item;
}
