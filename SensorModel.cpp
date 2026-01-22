/**
 * @file SensorModel.cpp
 * @brief 传感器模型管理器实现
 * 
 * 使用模块化架构简化代码
 */

#include "SensorModel.h"
#include "ModbusServer.h"
#include "ModbusDataStore.h"
#include "ModbusValueConverter.h"
#include <QDebug>
#include <QSet>
#include <QMap>

SensorModelManager::SensorModelManager(QObject *parent)
    : QObject(parent)
{
}

// ==================== 导入导出 ====================

bool SensorModelManager::importFromFile(const QString &filePath)
{
    m_lastError.clear();
    m_sensors.clear();
    
    m_sensors = m_parser.importFromFile(filePath);
    
    if (m_sensors.isEmpty()) {
        m_lastError = m_parser.lastError();
        if (m_lastError.isEmpty()) {
            m_lastError = "未找到有效的传感器数据";
        }
        emit errorOccurred(m_lastError);
        return false;
    }
    
    // 检查地址冲突
    checkAddressConflicts();
    
    emit sensorsLoaded(m_sensors.count());
    emit sensorCountChanged(m_sensors.count());
    
    qDebug() << "[SensorModel] 成功导入" << m_sensors.count() << "个传感器配置";
    return true;
}

bool SensorModelManager::exportToFile(const QString &filePath)
{
    m_lastError.clear();
    
    if (!m_parser.exportToFile(filePath, m_sensors)) {
        m_lastError = m_parser.lastError();
        emit errorOccurred(m_lastError);
        return false;
    }
    
    qDebug() << "[SensorModel] 成功导出" << m_sensors.count() << "个传感器配置";
    return true;
}

// ==================== 应用到服务器 ====================

bool SensorModelManager::applyToServer(QObject *serverObj)
{
    if (!serverObj) {
        m_lastError = "服务器对象为空";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    ModbusServer *server = qobject_cast<ModbusServer*>(serverObj);
    if (!server) {
        m_lastError = "无效的服务器对象";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    ModbusDataStore *dataStore = server->dataStore();
    if (!dataStore) {
        m_lastError = "数据存储未初始化";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    int appliedCount = 0;
    int totalCount = m_sensors.count();
    
    for (int i = 0; i < totalCount; ++i) {
        const SensorItem &item = m_sensors[i];
        
        if (applySensorToDataStore(item, dataStore)) {
            appliedCount++;
        }
        
        emit applyProgress(i + 1, totalCount);
    }
    
    qDebug() << "[SensorModel] 成功应用" << appliedCount << "/" << totalCount << "个传感器配置";
    return true;
}

bool SensorModelManager::applySensorToDataStore(const SensorItem &item, ModbusDataStore *dataStore)
{
    QVector<quint16> registers;
    
    if (!item.toRegisters(registers)) {
        qDebug() << "[SensorModel] 转换失败 - 地址:" << item.address() 
                 << "名称:" << item.name() << "值:" << item.initialValue();
        return false;
    }
    
    switch (item.pointType()) {
    case SensorPointType::Coil: {
        bool value = (registers.isEmpty() ? false : registers[0] != 0);
        dataStore->writeCoil(item.address(), value);
        qDebug() << "[应用] 线圈 - 地址:" << item.address() << "值:" << value;
        return true;
    }
    
    case SensorPointType::DiscreteInput: {
        bool value = (registers.isEmpty() ? false : registers[0] != 0);
        dataStore->writeDiscreteInput(item.address(), value);
        qDebug() << "[应用] 离散输入 - 地址:" << item.address() << "值:" << value;
        return true;
    }
    
    case SensorPointType::HoldingRegister: {
        for (int i = 0; i < registers.count(); ++i) {
            dataStore->writeHoldingRegister(item.address() + i, registers[i]);
        }
        qDebug() << "[应用] 保持寄存器(" << item.valueTypeString() << ") - 地址:" 
                 << item.address() << "寄存器数:" << registers.count();
        return true;
    }
    
    case SensorPointType::InputRegister: {
        for (int i = 0; i < registers.count(); ++i) {
            dataStore->writeInputRegister(item.address() + i, registers[i]);
        }
        qDebug() << "[应用] 输入寄存器(" << item.valueTypeString() << ") - 地址:" 
                 << item.address() << "寄存器数:" << registers.count();
        return true;
    }
    }
    
    return false;
}

// ==================== 传感器管理 ====================

QVariantList SensorModelManager::getSensorList() const
{
    QVariantList list;
    
    for (const SensorItem &item : m_sensors) {
        list.append(item.toVariantMap());
    }
    
    return list;
}

void SensorModelManager::addSensor(const QString &name, const QString &type,
                                   const QString &initValue, const QString &unit)
{
    SensorItem item;
    item.setAddress(static_cast<quint16>(m_sensors.count()));
    item.setName(name);
    item.setPointType(SensorItem::parsePointType(type));
    item.setInitialValue(initValue);
    item.setUnit(unit);
    
    m_sensors.append(item);
    emit sensorCountChanged(m_sensors.count());
}

void SensorModelManager::clearSensors()
{
    m_sensors.clear();
    emit sensorCountChanged(0);
}

// ==================== 辅助方法 ====================

void SensorModelManager::checkAddressConflicts() const
{
    // 按点类型分组检查地址冲突
    QMap<SensorPointType, QSet<quint16>> usedAddresses;
    
    for (const SensorItem &item : m_sensors) {
        auto range = item.addressRange();
        
        for (quint16 addr = range.first; addr <= range.second; ++addr) {
            if (usedAddresses[item.pointType()].contains(addr)) {
                qDebug() << "[警告] 地址冲突 -" << item.pointTypeString() 
                         << "地址" << addr << "被多个传感器使用！"
                         << "传感器:" << item.name();
            }
            usedAddresses[item.pointType()].insert(addr);
        }
    }
}
