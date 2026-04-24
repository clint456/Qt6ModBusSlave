#ifndef MODBUSDATASTORE_H
#define MODBUSDATASTORE_H

#include <QObject>
#include <QReadWriteLock>
#include <QMap>
#include <QBitArray>
#include "ModbusTypes.h"

class DatabaseManager;

// Modbus 数据存储类
class ModbusDataStore : public QObject
{
    Q_OBJECT

public:
    // 地址空间上限常量（防止内存无限增长）
    static constexpr quint16 MAX_COIL_ADDRESS = 10000;
    static constexpr quint16 MAX_DISCRETE_INPUT_ADDRESS = 10000;
    static constexpr quint16 MAX_HOLDING_REGISTER_ADDRESS = 10000;
    static constexpr quint16 MAX_INPUT_REGISTER_ADDRESS = 10000;

    // 内存监控阈值
    static constexpr size_t MEMORY_WARNING_THRESHOLD = 8000;

    explicit ModbusDataStore(QObject *parent = nullptr);

    // 数据库模式控制
    void setDatabaseEnabled(bool enabled);
    bool isDatabaseEnabled() const { return m_databaseEnabled; }

    // 从数据库加载数据到内存缓存
    bool loadFromDatabase();

    // 线圈操作
    Q_INVOKABLE bool readCoil(quint16 address) const;
    bool readCoils(quint16 startAddress, quint16 count, QBitArray &values) const;
    Q_INVOKABLE bool writeCoil(quint16 address, bool value);
    bool writeCoils(quint16 startAddress, const QBitArray &values);

    // 离散输入操作
    Q_INVOKABLE bool readDiscreteInput(quint16 address) const;
    bool readDiscreteInputs(quint16 startAddress, quint16 count, QBitArray &values) const;
    bool writeDiscreteInput(quint16 address, bool value);

    // 保持寄存器操作
    Q_INVOKABLE quint16 readHoldingRegister(quint16 address) const;
    bool readHoldingRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values) const;
    Q_INVOKABLE bool writeHoldingRegister(quint16 address, quint16 value);
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

    // 内存使用统计
    Q_INVOKABLE size_t getTotalItemCount() const;
    Q_INVOKABLE size_t getCoilCount() const;
    Q_INVOKABLE size_t getDiscreteInputCount() const;
    Q_INVOKABLE size_t getHoldingRegisterCount() const;
    Q_INVOKABLE size_t getInputRegisterCount() const;

    // 检查内存使用是否超过阈值
    bool isMemoryWarningLevel() const;

signals:
    void coilChanged(quint16 address, bool value);
    void discreteInputChanged(quint16 address, bool value);
    void holdingRegisterChanged(quint16 address, quint16 value);
    // 批量保持寄存器变更：起始地址与连续寄存器值列表（用于减少频繁单条通知）
    void holdingRegistersChanged(quint16 startAddress, const QVector<quint16> &values);
    void inputRegisterChanged(quint16 address, quint16 value);
    // 内存使用警告信号
    void memoryWarning(size_t totalItems);

private:
    // 内存缓存
    QMap<quint16, bool> m_coils;
    QMap<quint16, bool> m_discreteInputs;
    QMap<quint16, quint16> m_holdingRegisters;
    QMap<quint16, quint16> m_inputRegisters;

    mutable QReadWriteLock m_coilsLock;
    mutable QReadWriteLock m_discreteInputsLock;
    mutable QReadWriteLock m_holdingRegistersLock;
    mutable QReadWriteLock m_inputRegistersLock;

    // 数据库模式标志
    bool m_databaseEnabled;
};

#endif // MODBUSDATASTORE_H
