#include "ModbusDataStore.h"

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
    if (count == 0 || count > ModbusConst::MAX_READ_COILS) {
        return false;
    }

    QReadLocker locker(&m_coilsLock);
    values.resize(count);

    for (quint16 i = 0; i < count; ++i) {
        quint16 address = startAddress + i;
        values.setBit(i, m_coils.value(address, false));
    }

    return true;
}

bool ModbusDataStore::writeCoil(quint16 address, bool value)
{
    QWriteLocker locker(&m_coilsLock);
    m_coils[address] = value;
    emit coilChanged(address, value);
    return true;
}

bool ModbusDataStore::writeCoils(quint16 startAddress, const QBitArray &values)
{
    if (values.isEmpty() || values.size() > ModbusConst::MAX_WRITE_COILS) {
        return false;
    }

    QWriteLocker locker(&m_coilsLock);
    for (int i = 0; i < values.size(); ++i) {
        quint16 address = startAddress + i;
        m_coils[address] = values.testBit(i);
        emit coilChanged(address, values.testBit(i));
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
    if (count == 0 || count > ModbusConst::MAX_READ_COILS) {
        return false;
    }

    QReadLocker locker(&m_discreteInputsLock);
    values.resize(count);

    for (quint16 i = 0; i < count; ++i) {
        quint16 address = startAddress + i;
        values.setBit(i, m_discreteInputs.value(address, false));
    }

    return true;
}

bool ModbusDataStore::writeDiscreteInput(quint16 address, bool value)
{
    QWriteLocker locker(&m_discreteInputsLock);
    m_discreteInputs[address] = value;
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
    if (count == 0 || count > ModbusConst::MAX_READ_REGISTERS) {
        return false;
    }

    QReadLocker locker(&m_holdingRegistersLock);
    values.clear();
    values.reserve(count);

    for (quint16 i = 0; i < count; ++i) {
        quint16 address = startAddress + i;
        values.append(m_holdingRegisters.value(address, 0));
    }

    return true;
}

bool ModbusDataStore::writeHoldingRegister(quint16 address, quint16 value)
{
    QWriteLocker locker(&m_holdingRegistersLock);
    m_holdingRegisters[address] = value;
    emit holdingRegisterChanged(address, value);
    return true;
}

bool ModbusDataStore::writeHoldingRegisters(quint16 startAddress, const QVector<quint16> &values)
{
    if (values.isEmpty() || values.size() > ModbusConst::MAX_WRITE_REGISTERS) {
        return false;
    }

    QWriteLocker locker(&m_holdingRegistersLock);
    for (int i = 0; i < values.size(); ++i) {
        quint16 address = startAddress + i;
        m_holdingRegisters[address] = values[i];
        emit holdingRegisterChanged(address, values[i]);
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
    if (count == 0 || count > ModbusConst::MAX_READ_REGISTERS) {
        return false;
    }

    QReadLocker locker(&m_inputRegistersLock);
    values.clear();
    values.reserve(count);

    for (quint16 i = 0; i < count; ++i) {
        quint16 address = startAddress + i;
        values.append(m_inputRegisters.value(address, 0));
    }

    return true;
}

bool ModbusDataStore::writeInputRegister(quint16 address, quint16 value)
{
    QWriteLocker locker(&m_inputRegistersLock);
    m_inputRegisters[address] = value;
    return true;
}

// ========== 数据初始化 ==========

void ModbusDataStore::initializeCoils(quint16 startAddress, quint16 count, bool value)
{
    QWriteLocker locker(&m_coilsLock);
    for (quint16 i = 0; i < count; ++i) {
        m_coils[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeDiscreteInputs(quint16 startAddress, quint16 count, bool value)
{
    QWriteLocker locker(&m_discreteInputsLock);
    for (quint16 i = 0; i < count; ++i) {
        m_discreteInputs[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeHoldingRegisters(quint16 startAddress, quint16 count, quint16 value)
{
    QWriteLocker locker(&m_holdingRegistersLock);
    for (quint16 i = 0; i < count; ++i) {
        m_holdingRegisters[startAddress + i] = value;
    }
}

void ModbusDataStore::initializeInputRegisters(quint16 startAddress, quint16 count, quint16 value)
{
    QWriteLocker locker(&m_inputRegistersLock);
    for (quint16 i = 0; i < count; ++i) {
        m_inputRegisters[startAddress + i] = value;
    }
}

void ModbusDataStore::clearAll()
{
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
}
