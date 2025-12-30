#ifndef MODBUSFUNCTIONHANDLER_H
#define MODBUSFUNCTIONHANDLER_H

#include <QObject>
#include <QByteArray>
#include "ModbusTypes.h"
#include "ModbusDataStore.h"

// Modbus 功能码处理器
class ModbusFunctionHandler : public QObject
{
    Q_OBJECT

public:
    explicit ModbusFunctionHandler(ModbusDataStore *dataStore, QObject *parent = nullptr);

    // 处理请求并返回响应PDU
    QByteArray processRequest(const QByteArray &requestPdu);

signals:
    void requestProcessed(quint8 functionCode, bool success);
    void errorOccurred(quint8 functionCode, quint8 exceptionCode);

private:
    // 标准功能码处理
    QByteArray handleReadCoils(const QByteArray &request);
    QByteArray handleReadDiscreteInputs(const QByteArray &request);
    QByteArray handleReadHoldingRegisters(const QByteArray &request);
    QByteArray handleReadInputRegisters(const QByteArray &request);
    QByteArray handleWriteSingleCoil(const QByteArray &request);
    QByteArray handleWriteSingleRegister(const QByteArray &request);
    QByteArray handleWriteMultipleCoils(const QByteArray &request);
    QByteArray handleWriteMultipleRegisters(const QByteArray &request);

    // 辅助方法
    QByteArray buildErrorResponse(quint8 functionCode, quint8 exceptionCode);
    QByteArray bitArrayToBytes(const QBitArray &bits);

    ModbusDataStore *m_dataStore;
};

#endif // MODBUSFUNCTIONHANDLER_H
