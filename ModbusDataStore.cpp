#include "ModbusDataStore.h"
#include <QDebug>

ModbusDataStore::ModbusDataStore(QObject *parent)
    : QObject(parent)
{
}

// ========== 线圈操作 ==========

bool ModbusDataStore::readCoil(quint16 address) const
{
    QReadLocker locker(&m_coilsLock);
    return m_coils.value(address, false);
}

bool ModbusDataStore::readCoils(quint16 startAddress, quint16 count, QBitArray &values) const
{
    if (count == 0 || count > ModbusConst::MAX_READ_COILS)
    {
        return false;
    }

    quint32 endAddress = static_cast<quint32>(startAddress) + static_cast<quint32>(count) - 1;
    if (startAddress >= MAX_COIL_ADDRESS || endAddress >= MAX_COIL_ADDRESS)
    {
        return false;
    }

    QReadLocker locker(&m_coilsLock);
    values.resize(count);

    for (quint16 i = 0; i < count; ++i)
    {
        quint16 address = startAddress + i;
        values.setBit(i, m_coils.value(address, false));
    }

    return true;
}

bool ModbusDataStore::writeCoil(quint16 address, bool value)
{
    // 地址范围检查
    if (address >= MAX_COIL_ADDRESS)
    {
        qWarning() << "[DataStore] 线圈地址超出范围:" << address << "(最大:" << MAX_COIL_ADDRESS - 1 << ")";
        return false;
    }

    // 写入内存缓存
    {
        QWriteLocker locker(&m_coilsLock);
        m_coils[address] = value;
    }
    qDebug() << "[DataStore] 写入线圈 - 地址:" << address << "值:" << value << "准备发送信号";
    emit coilChanged(address, value);
    qDebug() << "[DataStore] 线圈变化信号已发送";

    // 检查内存使用
    if (isMemoryWarningLevel())
    {
        emit memoryWarning(getTotalItemCount());
    }
    return true;
}

bool ModbusDataStore::writeCoils(quint16 startAddress, const QBitArray &values)
{
    if (values.isEmpty() || values.size() > ModbusConst::MAX_WRITE_COILS)
    {
        return false;
    }

    // 地址范围检查
    quint16 endAddress = startAddress + values.size() - 1;
    if (endAddress >= MAX_COIL_ADDRESS)
    {
        qWarning() << "[DataStore] 批量写入线圈地址超出范围: " << startAddress << "-" << endAddress << "(最大:" << MAX_COIL_ADDRESS - 1 << ")";
        return false;
    }

    QVector<QPair<quint16, bool>> changes;
    {
        QWriteLocker locker(&m_coilsLock);
        for (int i = 0; i < values.size(); ++i)
        {
            quint16 address = startAddress + i;
            m_coils[address] = values.testBit(i);
            changes.append(qMakePair(address, values.testBit(i)));
            qDebug() << "[DataStore] 批量写入线圈 - 地址:" << address << "值:" << values.testBit(i);
        }
    }

    for (const auto &p : changes)
    {
        emit coilChanged(p.first, p.second);
    }

    // 检查内存使用
    if (isMemoryWarningLevel())
    {
        emit memoryWarning(getTotalItemCount());
    }

    return true;
}

// ========== 离散输入操作 ==========

bool ModbusDataStore::readDiscreteInput(quint16 address) const
{
    QReadLocker locker(&m_discreteInputsLock);
    return m_discreteInputs.value(address, false);
}

bool ModbusDataStore::readDiscreteInputs(quint16 startAddress, quint16 count, QBitArray &values) const
{
    if (count == 0 || count > ModbusConst::MAX_READ_COILS)
    {
        return false;
    }

    quint32 endAddress = static_cast<quint32>(startAddress) + static_cast<quint32>(count) - 1;
    if (startAddress >= MAX_DISCRETE_INPUT_ADDRESS || endAddress >= MAX_DISCRETE_INPUT_ADDRESS)
    {
        return false;
    }

    QReadLocker locker(&m_discreteInputsLock);
    values.resize(count);

    for (quint16 i = 0; i < count; ++i)
    {
        quint16 address = startAddress + i;
        values.setBit(i, m_discreteInputs.value(address, false));
    }

    return true;
}

bool ModbusDataStore::writeDiscreteInput(quint16 address, bool value)
{
    // 地址范围检查
    if (address >= MAX_DISCRETE_INPUT_ADDRESS)
    {
        qWarning() << "[DataStore] 离散输入地址超出范围:" << address << "(最大:" << MAX_DISCRETE_INPUT_ADDRESS - 1 << ")";
        return false;
    }

    {
        QWriteLocker locker(&m_discreteInputsLock);
        m_discreteInputs[address] = value;
    }
    qDebug() << "[DataStore] 写入离散输入 - 地址:" << address << "值:" << value;
    emit discreteInputChanged(address, value);
    return true;
}

// ========== 保持寄存器操作 ==========

quint16 ModbusDataStore::readHoldingRegister(quint16 address) const
{
    QReadLocker locker(&m_holdingRegistersLock);
    return m_holdingRegisters.value(address, 0);
}

bool ModbusDataStore::readHoldingRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values) const
{
    if (count == 0 || count > ModbusConst::MAX_READ_REGISTERS)
    {
        return false;
    }

    quint32 endAddress = static_cast<quint32>(startAddress) + static_cast<quint32>(count) - 1;
    if (startAddress >= MAX_HOLDING_REGISTER_ADDRESS || endAddress >= MAX_HOLDING_REGISTER_ADDRESS)
    {
        return false;
    }

    QReadLocker locker(&m_holdingRegistersLock);
    values.clear();
    values.reserve(count);

    for (quint16 i = 0; i < count; ++i)
    {
        quint16 address = startAddress + i;
        values.append(m_holdingRegisters.value(address, 0));
    }

    return true;
}

bool ModbusDataStore::writeHoldingRegister(quint16 address, quint16 value)
{
    // 地址范围检查
    if (address >= MAX_HOLDING_REGISTER_ADDRESS)
    {
        qWarning() << "[DataStore] 保持寄存器地址超出范围:" << address << "(最大:" << MAX_HOLDING_REGISTER_ADDRESS - 1 << ")";
        return false;
    }

    {
        QWriteLocker locker(&m_holdingRegistersLock);
        m_holdingRegisters[address] = value;
    }
    qDebug() << "[DataStore] 写入保持寄存器 - 地址:" << address << "值:" << value << "准备发送信号";
    emit holdingRegisterChanged(address, value);
    qDebug() << "[DataStore] 保持寄存器变化信号已发送";

    // 检查内存使用
    if (isMemoryWarningLevel())
    {
        emit memoryWarning(getTotalItemCount());
    }
    return true;
}

bool ModbusDataStore::writeHoldingRegisters(quint16 startAddress, const QVector<quint16> &values)
{
    if (values.isEmpty() || values.size() > ModbusConst::MAX_WRITE_REGISTERS)
    {
        return false;
    }

    // 地址范围检查
    quint16 endAddress = startAddress + values.size() - 1;
    if (endAddress >= MAX_HOLDING_REGISTER_ADDRESS)
    {
        qWarning() << "[DataStore] 批量写入保持寄存器地址超出范围: " << startAddress << "-" << endAddress << "(最大:" << MAX_HOLDING_REGISTER_ADDRESS - 1 << ")";
        return false;
    }

    QVector<quint16> writtenValues;
    writtenValues.reserve(values.size());
    {
        QWriteLocker locker(&m_holdingRegistersLock);
        for (int i = 0; i < values.size(); ++i)
        {
            quint16 address = startAddress + i;
            m_holdingRegisters[address] = values[i];
            writtenValues.append(values[i]);
            qDebug() << "[DataStore] 批量写入保持寄存器 - 地址:" << address << "值:" << values[i];
        }
    }

    qDebug() << "[DataStore] 批量写入保持寄存器 完成 - 起始:" << startAddress << "个数:" << writtenValues.size();
    emit holdingRegistersChanged(startAddress, writtenValues);

    // 检查内存使用
    if (isMemoryWarningLevel())
    {
        emit memoryWarning(getTotalItemCount());
    }

    return true;
}

// ========== 输入寄存器操作 ==========

quint16 ModbusDataStore::readInputRegister(quint16 address) const
{
    QReadLocker locker(&m_inputRegistersLock);
    return m_inputRegisters.value(address, 0);
}

bool ModbusDataStore::readInputRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values) const
{
    if (count == 0 || count > ModbusConst::MAX_READ_REGISTERS)
    {
        return false;
    }

    quint32 endAddress = static_cast<quint32>(startAddress) + static_cast<quint32>(count) - 1;
    if (startAddress >= MAX_INPUT_REGISTER_ADDRESS || endAddress >= MAX_INPUT_REGISTER_ADDRESS)
    {
        return false;
    }

    QReadLocker locker(&m_inputRegistersLock);
    values.clear();
    values.reserve(count);

    for (quint16 i = 0; i < count; ++i)
    {
        quint16 address = startAddress + i;
        values.append(m_inputRegisters.value(address, 0));
    }

    return true;
}

bool ModbusDataStore::writeInputRegister(quint16 address, quint16 value)
{
    // 地址范围检查
    if (address >= MAX_INPUT_REGISTER_ADDRESS)
    {
        qWarning() << "[DataStore] 输入寄存器地址超出范围:" << address << "(最大:" << MAX_INPUT_REGISTER_ADDRESS - 1 << ")";
        return false;
    }

    qDebug() << "[DataStore] 写入输入寄存器 - 地址:" << address << "值:" << value;
    {
        QWriteLocker locker(&m_inputRegistersLock);
        m_inputRegisters[address] = value;
    }
    emit inputRegisterChanged(address, value);
    return true;
}

// ========== 数据初始化 ==========

void ModbusDataStore::initializeCoils(quint16 startAddress, quint16 count, bool value)
{
    QWriteLocker locker(&m_coilsLock);
    for (quint16 i = 0; i < count; ++i)
    {
        m_coils[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeDiscreteInputs(quint16 startAddress, quint16 count, bool value)
{
    QWriteLocker locker(&m_discreteInputsLock);
    for (quint16 i = 0; i < count; ++i)
    {
        m_discreteInputs[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeHoldingRegisters(quint16 startAddress, quint16 count, quint16 value)
{
    QWriteLocker locker(&m_holdingRegistersLock);
    for (quint16 i = 0; i < count; ++i)
    {
        m_holdingRegisters[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeInputRegisters(quint16 startAddress, quint16 count, quint16 value)
{
    QWriteLocker locker(&m_inputRegistersLock);
    for (quint16 i = 0; i < count; ++i)
    {
        m_inputRegisters[startAddress + i] = value;
    }
}

void ModbusDataStore::clearAll()
{
    // 清理内存缓存
    {
        QWriteLocker locker(&m_coilsLock);
        m_coils.clear();
    }
    {
        QWriteLocker locker(&m_discreteInputsLock);
        m_discreteInputs.clear();
    }
    {
        QWriteLocker locker(&m_holdingRegistersLock);
        m_holdingRegisters.clear();
    }
    {
        QWriteLocker locker(&m_inputRegistersLock);
        m_inputRegisters.clear();
    }
    qDebug() << "[DataStore] 已清理内存中的所有数据";
}

// ========== 内存使用统计 ==========

size_t ModbusDataStore::getTotalItemCount() const
{
    return getCoilCount() + getDiscreteInputCount() +
           getHoldingRegisterCount() + getInputRegisterCount();
}

size_t ModbusDataStore::getCoilCount() const
{
    QReadLocker locker(&m_coilsLock);
    return m_coils.size();
}

size_t ModbusDataStore::getDiscreteInputCount() const
{
    QReadLocker locker(&m_discreteInputsLock);
    return m_discreteInputs.size();
}

size_t ModbusDataStore::getHoldingRegisterCount() const
{
    QReadLocker locker(&m_holdingRegistersLock);
    return m_holdingRegisters.size();
}

size_t ModbusDataStore::getInputRegisterCount() const
{
    QReadLocker locker(&m_inputRegistersLock);
    return m_inputRegisters.size();
}

bool ModbusDataStore::isMemoryWarningLevel() const
{
    return getTotalItemCount() >= MEMORY_WARNING_THRESHOLD;
}
