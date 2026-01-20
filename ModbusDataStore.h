#ifndef MODBUSDATASTORE_H
#define MODBUSDATASTORE_H

#include <QObject>
#include <QReadWriteLock>
#include <QMap>
#include <QBitArray>
#include "ModbusTypes.h"

// Modbus 数据存储类
class ModbusDataStore : public QObject
{
    Q_OBJECT

public:
    explicit ModbusDataStore(QObject *parent = nullptr);

    // 线圈操作
    Q_INVOKABLE bool readCoil(quint16 address) const;
    bool readCoils(quint16 startAddress, quint16 count, QBitArray &values) const; // 这里的const表示readCoils方法不会修改对象的成员变量
    bool writeCoil(quint16 address, bool value);
    bool writeCoils(quint16 startAddress, const QBitArray &values); // 这里的const表示避免在参数传递时复制大对象，提高性能

    // 离散输入操作
    Q_INVOKABLE bool readDiscreteInput(quint16 address) const;
    bool readDiscreteInputs(quint16 startAddress, quint16 count, QBitArray &values) const;
    bool writeDiscreteInput(quint16 address, bool value);

    // 保持寄存器操作
    Q_INVOKABLE quint16 readHoldingRegister(quint16 address) const;
    bool readHoldingRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values) const;
    bool writeHoldingRegister(quint16 address, quint16 value);
    bool writeHoldingRegisters(quint16 startAddress, const QVector<quint16> &values);

    // 输入寄存器操作
    Q_INVOKABLE quint16 readInputRegister(quint16 address) const;
    bool readInputRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values) const;
    bool writeInputRegister(quint16 address, quint16 value);

    // 数据初始化
    void initializeCoils(quint16 startAddress, quint16 count, bool value = false);
    void initializeDiscreteInputs(quint16 startAddress, quint16 count, bool value = false);
    void initializeHoldingRegisters(quint16 startAddress, quint16 count, quint16 value = 0);
    void initializeInputRegisters(quint16 startAddress, quint16 count, quint16 value = 0);

    // 清空所有数据
    void clearAll();

signals:
    void coilChanged(quint16 address, bool value);
    void discreteInputChanged(quint16 address, bool value);
    void holdingRegisterChanged(quint16 address, quint16 value);
    void inputRegisterChanged(quint16 address, quint16 value);

private:
    QMap<quint16, bool> m_coils;
    QMap<quint16, bool> m_discreteInputs;
    QMap<quint16, quint16> m_holdingRegisters;
    QMap<quint16, quint16> m_inputRegisters;

    mutable QReadWriteLock m_coilsLock;
    mutable QReadWriteLock m_discreteInputsLock;
    mutable QReadWriteLock m_holdingRegistersLock;
    mutable QReadWriteLock m_inputRegistersLock;
};

#endif // MODBUSDATASTORE_H
