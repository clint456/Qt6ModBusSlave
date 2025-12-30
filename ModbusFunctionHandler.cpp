#include "ModbusFunctionHandler.h"
#include <QtEndian>
#include <QDebug>

ModbusFunctionHandler::ModbusFunctionHandler(ModbusDataStore *dataStore, QObject *parent)
    : QObject(parent)
    , m_dataStore(dataStore)
{
}

QByteArray ModbusFunctionHandler::processRequest(const QByteArray &requestPdu)
{
    if (requestPdu.isEmpty()) {
        return buildErrorResponse(0x00, IllegalFunction);
    }

    quint8 functionCode = static_cast<quint8>(requestPdu[0]);

    QByteArray response;
    switch (functionCode) {
    case ReadCoils:
        response = handleReadCoils(requestPdu);
        break;
    case ReadDiscreteInputs:
        response = handleReadDiscreteInputs(requestPdu);
        break;
    case ReadHoldingRegisters:
        response = handleReadHoldingRegisters(requestPdu);
        break;
    case ReadInputRegisters:
        response = handleReadInputRegisters(requestPdu);
        break;
    case WriteSingleCoil:
        response = handleWriteSingleCoil(requestPdu);
        break;
    case WriteSingleRegister:
        response = handleWriteSingleRegister(requestPdu);
        break;
    case WriteMultipleCoils:
        response = handleWriteMultipleCoils(requestPdu);
        break;
    case WriteMultipleRegisters:
        response = handleWriteMultipleRegisters(requestPdu);
        break;
    default:
        response = buildErrorResponse(functionCode, IllegalFunction);
        emit errorOccurred(functionCode, IllegalFunction);
        break;
    }

    emit requestProcessed(functionCode, !response.isEmpty());
    return response;
}

// ========== 功能码 01：读线圈 ==========
QByteArray ModbusFunctionHandler::handleReadCoils(const QByteArray &request)
{
    if (request.size() < 5) {
        return buildErrorResponse(ReadCoils, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    if (quantity == 0 || quantity > ModbusConst::MAX_READ_COILS) {
        return buildErrorResponse(ReadCoils, IllegalDataValue);
    }

    QBitArray values;
    if (!m_dataStore->readCoils(startAddress, quantity, values)) {
        return buildErrorResponse(ReadCoils, IllegalDataAddress);
    }

    QByteArray data = bitArrayToBytes(values);
    QByteArray response;
    response.append(static_cast<char>(ReadCoils));
    response.append(static_cast<char>(data.size()));
    response.append(data);

    return response;
}

// ========== 功能码 02：读离散输入 ==========
QByteArray ModbusFunctionHandler::handleReadDiscreteInputs(const QByteArray &request)
{
    if (request.size() < 5) {
        return buildErrorResponse(ReadDiscreteInputs, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    if (quantity == 0 || quantity > ModbusConst::MAX_READ_COILS) {
        return buildErrorResponse(ReadDiscreteInputs, IllegalDataValue);
    }

    QBitArray values;
    if (!m_dataStore->readDiscreteInputs(startAddress, quantity, values)) {
        return buildErrorResponse(ReadDiscreteInputs, IllegalDataAddress);
    }

    QByteArray data = bitArrayToBytes(values);
    QByteArray response;
    response.append(static_cast<char>(ReadDiscreteInputs));
    response.append(static_cast<char>(data.size()));
    response.append(data);

    return response;
}

// ========== 功能码 03：读保持寄存器 ==========
QByteArray ModbusFunctionHandler::handleReadHoldingRegisters(const QByteArray &request)
{
    if (request.size() < 5) {
        return buildErrorResponse(ReadHoldingRegisters, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    if (quantity == 0 || quantity > ModbusConst::MAX_READ_REGISTERS) {
        return buildErrorResponse(ReadHoldingRegisters, IllegalDataValue);
    }

    QVector<quint16> values;
    if (!m_dataStore->readHoldingRegisters(startAddress, quantity, values)) {
        return buildErrorResponse(ReadHoldingRegisters, IllegalDataAddress);
    }

    QByteArray response;
    response.append(static_cast<char>(ReadHoldingRegisters));
    response.append(static_cast<char>(values.size() * 2));

    for (quint16 value : values) {
        quint16 bigEndianValue = qToBigEndian(value);
        response.append(reinterpret_cast<const char*>(&bigEndianValue), 2);
    }

    return response;
}

// ========== 功能码 04：读输入寄存器 ==========
QByteArray ModbusFunctionHandler::handleReadInputRegisters(const QByteArray &request)
{
    if (request.size() < 5) {
        return buildErrorResponse(ReadInputRegisters, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    if (quantity == 0 || quantity > ModbusConst::MAX_READ_REGISTERS) {
        return buildErrorResponse(ReadInputRegisters, IllegalDataValue);
    }

    QVector<quint16> values;
    if (!m_dataStore->readInputRegisters(startAddress, quantity, values)) {
        return buildErrorResponse(ReadInputRegisters, IllegalDataAddress);
    }

    QByteArray response;
    response.append(static_cast<char>(ReadInputRegisters));
    response.append(static_cast<char>(values.size() * 2));

    for (quint16 value : values) {
        quint16 bigEndianValue = qToBigEndian(value);
        response.append(reinterpret_cast<const char*>(&bigEndianValue), 2);
    }

    return response;
}

// ========== 功能码 05：写单个线圈 ==========
QByteArray ModbusFunctionHandler::handleWriteSingleCoil(const QByteArray &request)
{
    if (request.size() < 5) {
        return buildErrorResponse(WriteSingleCoil, IllegalDataValue);
    }

    quint16 address = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 value = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    if (value != 0x0000 && value != 0xFF00) {
        return buildErrorResponse(WriteSingleCoil, IllegalDataValue);
    }

    bool coilValue = (value == 0xFF00);
    if (!m_dataStore->writeCoil(address, coilValue)) {
        return buildErrorResponse(WriteSingleCoil, SlaveDeviceFailure);
    }

    // 回显请求
    return request;
}

// ========== 功能码 06：写单个寄存器 ==========
QByteArray ModbusFunctionHandler::handleWriteSingleRegister(const QByteArray &request)
{
    qDebug() << "ModbusFunctionHandler::handleWriteSingleRegister - 请求长度:" << request.size();
    
    if (request.size() < 5) {
        qDebug() << "错误: 请求长度不足";
        return buildErrorResponse(WriteSingleRegister, IllegalDataValue);
    }

    quint16 address = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 value = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    qDebug() << "写单个寄存器 - 地址:" << address << "值:" << value;

    if (!m_dataStore->writeHoldingRegister(address, value)) {
        qDebug() << "错误: 写入失败";
        return buildErrorResponse(WriteSingleRegister, SlaveDeviceFailure);
    }

    qDebug() << "成功写入寄存器";
    qDebug() << "响应数据 (十六进制):" << request.toHex(' ').toUpper();
    // 回显请求
    return request;
}

// ========== 功能码 15：写多个线圈 ==========
QByteArray ModbusFunctionHandler::handleWriteMultipleCoils(const QByteArray &request)
{
    if (request.size() < 6) {
        return buildErrorResponse(WriteMultipleCoils, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));
    quint8 byteCount = static_cast<quint8>(request[5]);

    if (quantity == 0 || quantity > ModbusConst::MAX_WRITE_COILS) {
        return buildErrorResponse(WriteMultipleCoils, IllegalDataValue);
    }

    int expectedByteCount = (quantity + 7) / 8;
    if (byteCount != expectedByteCount || request.size() < 6 + byteCount) {
        return buildErrorResponse(WriteMultipleCoils, IllegalDataValue);
    }

    QBitArray values(quantity);
    for (int i = 0; i < quantity; ++i) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        quint8 byte = static_cast<quint8>(request[6 + byteIndex]);
        values.setBit(i, (byte & (1 << bitIndex)) != 0);
    }

    if (!m_dataStore->writeCoils(startAddress, values)) {
        return buildErrorResponse(WriteMultipleCoils, SlaveDeviceFailure);
    }

    QByteArray response;
    response.append(static_cast<char>(WriteMultipleCoils));
    quint16 addr = qToBigEndian(startAddress);
    response.append(reinterpret_cast<const char*>(&addr), 2);
    quint16 qty = qToBigEndian(quantity);
    response.append(reinterpret_cast<const char*>(&qty), 2);

    return response;
}

// ========== 功能码 16：写多个寄存器 ==========
QByteArray ModbusFunctionHandler::handleWriteMultipleRegisters(const QByteArray &request)
{
    if (request.size() < 6) {
        return buildErrorResponse(WriteMultipleRegisters, IllegalDataValue);
    }

    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));
    quint8 byteCount = static_cast<quint8>(request[5]);

    if (quantity == 0 || quantity > ModbusConst::MAX_WRITE_REGISTERS) {
        return buildErrorResponse(WriteMultipleRegisters, IllegalDataValue);
    }

    if (byteCount != quantity * 2 || request.size() < 6 + byteCount) {
        return buildErrorResponse(WriteMultipleRegisters, IllegalDataValue);
    }

    QVector<quint16> values;
    values.reserve(quantity);
    for (int i = 0; i < quantity; ++i) {
        quint16 value = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 6 + i * 2));
        values.append(value);
    }

    if (!m_dataStore->writeHoldingRegisters(startAddress, values)) {
        return buildErrorResponse(WriteMultipleRegisters, SlaveDeviceFailure);
    }

    QByteArray response;
    response.append(static_cast<char>(WriteMultipleRegisters));
    quint16 addr = qToBigEndian(startAddress);
    response.append(reinterpret_cast<const char*>(&addr), 2);
    quint16 qty = qToBigEndian(quantity);
    response.append(reinterpret_cast<const char*>(&qty), 2);

    return response;
}

// ========== 辅助方法 ==========

QByteArray ModbusFunctionHandler::buildErrorResponse(quint8 functionCode, quint8 exceptionCode)
{
    QByteArray response;
    response.append(static_cast<char>(functionCode | 0x80));
    response.append(static_cast<char>(exceptionCode));
    return response;
}

QByteArray ModbusFunctionHandler::bitArrayToBytes(const QBitArray &bits)
{
    int byteCount = (bits.size() + 7) / 8;
    QByteArray bytes(byteCount, 0);

    for (int i = 0; i < bits.size(); ++i) {
        if (bits.testBit(i)) {
            int byteIndex = i / 8;
            int bitIndex = i % 8;
            bytes[byteIndex] |= (1 << bitIndex);
        }
    }

    return bytes;
}
