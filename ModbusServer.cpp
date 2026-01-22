/**
 * @file ModbusServer.cpp
 * @brief Modbus TCP/RTU服务器实现
 * 
 * 支持Modbus TCP和RTU两种模式，实现标准Modbus功能码以及文件记录操作
 */

#include "ModbusServer.h"
#include <QtEndian>
#include <QDebug>

ModbusServer::ModbusServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(nullptr)
    , m_serialPort(nullptr)
    , m_rtuTimer(nullptr)
    , m_running(false)
    , m_mode(ModeTCP)
    , m_requestCount(0)
    , m_lastFunctionCode(0)
{
    // 创建数据存储
    m_dataStore = new ModbusDataStore(this); // 加上this可用进行自动管理子对象生命周期
    m_functionHandler = new ModbusFunctionHandler(m_dataStore, this);
    m_fileStore = new FileStore(this);
    m_addressStore = new FileAddressStore(this);

    // 连接信号
    connect(m_functionHandler, &ModbusFunctionHandler::requestProcessed,
            this, [this](quint8 fc, bool success) {
        incrementRequestCount();
        emit requestReceived(fc);
    });
}

ModbusServer::~ModbusServer()
{
    stop();
}

// ========== TCP 服务器 ==========

bool ModbusServer::startTcp(quint16 port)
{
    if (m_running) {
        stop();
    }

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ModbusServer::onNewTcpConnection); // 参数依次为：发布对象，信号，接受对象，槽函数

    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        setStatusMessage(QString("TCP 启动失败: %1").arg(m_tcpServer->errorString()));
        emit errorOccurred(m_statusMessage);
        delete m_tcpServer;
        m_tcpServer = nullptr;
        return false;
    }

    m_running = true;
    m_mode = ModeTCP;
    m_requestCount = 0;
    setStatusMessage(QString("TCP 服务器运行中 (端口 %1)").arg(port));
    
    emit runningChanged(true);
    emit modeChanged(m_mode);
    emit requestCountChanged(m_requestCount);
    
    return true;
}

void ModbusServer::stopTcp()
{
    if (m_tcpServer) {
        m_tcpServer->close();
        
        for (QTcpSocket *socket : m_tcpClients) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
        m_tcpClients.clear();
        m_tcpBuffers.clear();
        
        delete m_tcpServer;
        m_tcpServer = nullptr;
    }
}

void ModbusServer::onNewTcpConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        m_tcpClients.append(socket);
        m_tcpBuffers[socket] = QByteArray();

        connect(socket, &QTcpSocket::readyRead, this, &ModbusServer::onTcpReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &ModbusServer::onTcpDisconnected);
    }
}

void ModbusServer::onTcpReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray &buffer = m_tcpBuffers[socket];
    buffer.append(socket->readAll());

    // 处理所有完整的请求
    while (buffer.size() >= 8) {  // MBAP 头 (7 字节) + 至少 1 字节 PDU
        // 读取长度字段
        quint16 length = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(buffer.data() + 4));
        int totalLength = 6 + length;  // MBAP 头前 6 字节 + 长度

        if (buffer.size() < totalLength) {
            break;  // 等待更多数据
        }

        // 提取完整的 ADU
        QByteArray adu = buffer.left(totalLength);
        buffer.remove(0, totalLength);

        // 处理请求
        QByteArray response = processTcpRequest(adu);
        if (!response.isEmpty()) {
            socket->write(response);
        }
    }
}

void ModbusServer::onTcpDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qDebug() << "客户端断开连接:" << socket->peerAddress().toString();
    
    m_tcpClients.removeOne(socket);
    m_tcpBuffers.remove(socket);
    socket->deleteLater();
}

QByteArray ModbusServer::processTcpRequest(const QByteArray &adu)
{
    if (adu.size() < 8) {
        return QByteArray();
    }

    // 提取 MBAP 头
    // 将constchar*转换为const uchar*以符合qFromBigEndian的参数要求
    quint16 transactionId = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(adu.data())); 
    quint16 protocolId = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(adu.data() + 2));
    quint16 length = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(adu.data() + 4));
    quint8 unitId = static_cast<quint8>(adu[6]);

    // 验证协议 ID
    if (protocolId != 0) {
        return QByteArray();
    }

    // 提取 PDU,截取前7字节后的数据
    /* 例如00 01 00 00 00 06 02 03 00 12 00 04的前7个字节: 
    | 字节 | 含义 | 值（十六进制） | 说明 |
    |------|------|----------------|------|
    | 0–1  | Transaction ID | `0x0001` | 请求 ID = 1 |
    | 2–3  | Protocol ID    | `0x0000` | Modbus/TCP 标准 |
    | 4–5  | Length         | `0x0006` | 后续有 6 字节（Unit ID + PDU） |
    | 6    | Unit ID        | `0x02`   | 目标从站地址 = 2 |
    | 7    | Function Code  | `0x03`   | 读保持寄存器（Read Holding Registers） |
    | 8–9  | Starting Address | `0x0012` | 起始寄存器地址 = 0x12 = 18（十进制） |
    | 10–11| Quantity       | `0x0004` | 读取数量 = 4 个寄存器（8 字节数据） |    
    */
    QByteArray pdu = adu.mid(7);
    quint8 functionCode = static_cast<quint8>(pdu[0]);

    // 发送接收报文信号到UI
    QString recvPacket = QString("TCP 请求 - FC %1 (0x%2) PDU:%3字节")
        .arg(functionCode)
        .arg(functionCode, 2, 16, QChar('0')).toUpper()
        .arg(pdu.size());
    emit packetReceived(formatPacket(adu, "← 接收"));
    qDebug() << recvPacket;

    // 更新最后接收到的功能码
    if (m_lastFunctionCode != functionCode) {
        m_lastFunctionCode = functionCode;
        emit lastFunctionCodeChanged(functionCode);
    }

    // 路由到对应处理器
    QByteArray responsePdu = routeFunctionCode(functionCode, pdu);
    if (responsePdu.isEmpty()) {
        return QByteArray();
    }

    // 构建响应 ADU
    QByteArray responseAdu;
    quint16 txId = qToBigEndian(transactionId);
    responseAdu.append(reinterpret_cast<const char*>(&txId), 2);
    quint16 prId = qToBigEndian(quint16(0));
    responseAdu.append(reinterpret_cast<const char*>(&prId), 2);
    quint16 len = qToBigEndian(quint16(responsePdu.size() + 1));
    responseAdu.append(reinterpret_cast<const char*>(&len), 2);
    responseAdu.append(static_cast<char>(unitId));
    responseAdu.append(responsePdu);

    emit packetSent(formatPacket(responseAdu, "→ 发送"));
    return responseAdu;
}

// ========== RTU 服务器 ==========

bool ModbusServer::startRtu(const QString &portName, int baudRate)
{
    if (m_running) {
        stop();
    }

    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        setStatusMessage(QString("RTU 启动失败: %1").arg(m_serialPort->errorString()));
        emit errorOccurred(m_statusMessage);
        delete m_serialPort;
        m_serialPort = nullptr;
        return false;
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &ModbusServer::onRtuReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &ModbusServer::onRtuError);

    // 创建帧间隔定时器
    // 对于9600波特率，一个完整帧（最多256字节）传输需要约300ms
    // 使用更保守的超时时间，确保完整帧都能接收完毕
    m_rtuTimer = new QTimer(this);
    int charTime = (11 * 1000) / baudRate;  // 毫秒
    int timeout = qMax(50, charTime * 35);  // 至少50ms，或35个字符时间
    m_rtuTimer->setInterval(timeout);
    m_rtuTimer->setSingleShot(true);
    qDebug() << "RTU 定时器间隔设置为:" << timeout << "ms (字符时间:" << charTime << "ms, 波特率:" << baudRate << ")";
    connect(m_rtuTimer, &QTimer::timeout, this, [this]() {
        if (!m_rtuBuffer.isEmpty()) {
            qDebug() << "⏱ 定时器超时，处理缓冲区数据，长度:" << m_rtuBuffer.size();
            QByteArray response = processRtuRequest(m_rtuBuffer);
            if (!response.isEmpty() && m_serialPort) {
                m_serialPort->write(response);
            }
            m_rtuBuffer.clear();
        }
    });

    m_running = true;
    m_mode = ModeRTU;
    m_requestCount = 0;
    setStatusMessage(QString("RTU 服务器运行中 (%1, %2)").arg(portName).arg(baudRate));
    
    emit runningChanged(true);
    emit modeChanged(m_mode);
    emit requestCountChanged(m_requestCount);
    
    return true;
}

void ModbusServer::stopRtu()
{
    if (m_rtuTimer) {
        m_rtuTimer->stop();
        delete m_rtuTimer;
        m_rtuTimer = nullptr;
    }

    if (m_serialPort) {
        m_serialPort->close();
        delete m_serialPort;
        m_serialPort = nullptr;
    }

    m_rtuBuffer.clear();
}

void ModbusServer::onRtuReadyRead()
{
    if (!m_serialPort) return;

    // 读取串口数据并追加到缓冲区
    m_rtuBuffer.append(m_serialPort->readAll());
    
    // 检查是否已接收到完整帧（最小4字节：从站地址 + 功能码 + CRC）
    if (m_rtuBuffer.size() >= 4) {
        quint8 functionCode = static_cast<quint8>(m_rtuBuffer.at(1));
        int expectedLength = getExpectedFrameLength(functionCode, m_rtuBuffer);
        
        if (expectedLength > 0 && m_rtuBuffer.size() >= expectedLength) {
            // 已接收完整帧，立即处理
            m_rtuTimer->stop();
            QByteArray response = processRtuRequest(m_rtuBuffer);
            if (!response.isEmpty() && m_serialPort) {
                m_serialPort->write(response);
            }
            m_rtuBuffer.clear();
            return;
        }
    }
    
    // 帧不完整，重启定时器等待更多数据
    m_rtuTimer->start();
}

void ModbusServer::onRtuError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        setStatusMessage(QString("RTU 错误: %1").arg(m_serialPort->errorString()));
        emit errorOccurred(m_statusMessage);
    }
}

QByteArray ModbusServer::processRtuRequest(const QByteArray &adu)
{
    emit packetReceived(formatPacket(adu, "← RTU接收"));
    
    // 验证帧长度（最小4字节：从站地址 + 功能码 + CRC）
    if (adu.size() < 4) {
        qWarning() << "RTU请求长度不足:" << adu.size() << "字节";
        return QByteArray();
    }

    // 验证CRC校验码
    quint16 receivedCrc = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(adu.data() + adu.size() - 2));
    quint16 calculatedCrc = calculateCRC(adu.left(adu.size() - 2));
    
    if (receivedCrc != calculatedCrc) {
        qWarning() << "RTU CRC校验失败";
        return QByteArray();
    }

    // 提取从站地址和PDU（协议数据单元）
    quint8 slaveAddress = static_cast<quint8>(adu[0]);
    QByteArray pdu = adu.mid(1, adu.size() - 3);
    quint8 functionCode = static_cast<quint8>(pdu[0]);

    // 路由到对应功能处理器
    QByteArray responsePdu = routeFunctionCode(functionCode, pdu);
    if (responsePdu.isEmpty()) {
        return QByteArray();
    }

    // 构建响应ADU（应用数据单元：从站地址 + PDU + CRC）
    QByteArray responseAdu;
    responseAdu.append(static_cast<char>(slaveAddress));
    responseAdu.append(responsePdu);
    
    // 计算并添加CRC校验码
    quint16 crc = calculateCRC(responseAdu);
    quint16 crcLe = qToLittleEndian(crc);
    responseAdu.append(reinterpret_cast<const char*>(&crcLe), 2);

    emit packetSent(formatPacket(responseAdu, "→ RTU发送"));
    return responseAdu;
}

quint16 ModbusServer::calculateCRC(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

// ========== 通用控制 ==========

void ModbusServer::stop()
{
    stopTcp();
    stopRtu();
    
    m_running = false;
    setStatusMessage("服务器已停止");
    emit runningChanged(false);
}

void ModbusServer::initializeData()
{
    // 初始化线圈
    m_dataStore->initializeCoils(0, 100, false);
    
    // 初始化离散输入
    m_dataStore->initializeDiscreteInputs(0, 100, false);
    
    // 初始化保持寄存器
    m_dataStore->initializeHoldingRegisters(0, 100, 0);
    
    // 初始化输入寄存器
    m_dataStore->initializeInputRegisters(0, 100, 0);
    
    // 创建测试文件
    m_fileStore->createFile(1, "温度数据文件", 256);
    m_fileStore->createFile(2, "状态数据文件", 128);
    
    // 初始化地址存储
    m_addressStore->initializeRegion(1000, 200);
    
    qDebug() << "数据初始化完成";
}

// 获取文件列表
QStringList ModbusServer::getFileList() const
{
    return m_fileStore->getFileList();
}

// 查询文件内容
QString ModbusServer::queryFileContent(int fileNumber, int maxRecords)
{
    QString result;
    result += QString("========== 文件 %1 内容查询 ==========\n\n").arg(fileNumber);
    
    // 获取文件信息
    QString info = m_fileStore->getFileInfo(fileNumber);
    result += info + "\n\n";
    
    // 获取所有记录（原始字节数据）
    QMap<quint16, QByteArray> records = m_fileStore->getAllRecordsRaw(fileNumber, maxRecords);
    
    if (records.isEmpty()) {
        result += "该文件暂无数据写入\n";
    } else {
        result += QString("已写入的记录（最多显示 %1 条）：\n").arg(maxRecords);
        result += "记录号  十六进制                            ASCII字符串\n";
        result += "------  --------------------------------    ----------------------\n";
        
        for (auto it = records.begin(); it != records.end(); ++it) {
            quint16 recordNum = it.key();
            QByteArray data = it.value();
            
            // 十六进制显示
            QString hexStr;
            for (int i = 0; i < data.size(); ++i) {
                hexStr += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper();
            }
            hexStr = hexStr.leftJustified(36, ' ');
            
            // ASCII显示（可打印字符）
            QString asciiStr;
            for (int i = 0; i < data.size(); ++i) {
                char c = data[i];
                if (c >= 32 && c <= 126) {  // 可打印字符
                    asciiStr += c;
                } else {
                    asciiStr += '.';
                }
            }
            
            result += QString("%1    %2  %3\n")
                    .arg(recordNum, 6)
                    .arg(hexStr)
                    .arg(asciiStr);
        }
        
        result += QString("\n总计: %1 条记录\n").arg(records.size());
    }
    
    result += "\n" + QString("=").repeated(40) + "\n";
    return result;
}

QString ModbusServer::queryAddressFile(int startAddress, int count)
{
    QString result;
    result += QString("========== 保持寄存器查询 ==========\n\n");
    result += QString("起始地址: %1\n").arg(startAddress);
    result += QString("查询数量: %1\n\n").arg(count);
    
    // 从ModbusDataStore读取保持寄存器
    QMap<quint16, QByteArray> data;
    bool hasData = false;
    
    for (int i = 0; i < count; ++i) {
        quint16 addr = startAddress + i;
        quint16 value = m_dataStore->readHoldingRegister(addr);
        
        // 将16位值转换为大端字节序的2字节数据
        QByteArray bytes(2, 0);
        bytes[0] = static_cast<char>((value >> 8) & 0xFF);  // 高字节
        bytes[1] = static_cast<char>(value & 0xFF);         // 低字节
        
        // 只记录非零的寄存器
        if (value != 0) {
            hasData = true;
        }
        data[addr] = bytes;
    }
    
    if (!hasData) {
        result += "该地址区域暂无数据写入（所有值为0）\n";
    } else {
        result += QString("保持寄存器数据（显示 %1 个地址）：\n").arg(count);
        result += "地址    十进制值  十六进制                        ASCII字符串\n";
        result += "------  --------  --------------------------------  ----------------------\n";
        
        for (auto it = data.begin(); it != data.end(); ++it) {
            quint16 address = it.key();
            QByteArray bytes = it.value();
            
            // 转换回16位值用于十进制显示
            quint16 value = ((unsigned char)bytes[0] << 8) | (unsigned char)bytes[1];
            
            // 十六进制显示
            QString hexStr;
            for (int i = 0; i < bytes.size(); ++i) {
                hexStr += QString("%1 ").arg((unsigned char)bytes[i], 2, 16, QChar('0')).toUpper();
            }
            hexStr = hexStr.leftJustified(34, ' ');
            
            // ASCII显示（可打印字符）
            QString asciiStr;
            for (int i = 0; i < bytes.size(); ++i) {
                char c = bytes[i];
                if (c >= 32 && c <= 126) {  // 可打印字符
                    asciiStr += c;
                } else {
                    asciiStr += '.';
                }
            }
            
            result += QString("%1    %2    %3  %4\n")
                    .arg(address, 6)
                    .arg(value, 8)
                    .arg(hexStr)
                    .arg(asciiStr);
        }
        
        // 统计非零值数量
        int nonZeroCount = 0;
        for (auto it = data.begin(); it != data.end(); ++it) {
            quint16 value = ((unsigned char)it.value()[0] << 8) | (unsigned char)it.value()[1];
            if (value != 0) nonZeroCount++;
        }
        result += QString("\n总计: %1 个地址，其中 %2 个非零值\n").arg(data.size()).arg(nonZeroCount);
    }
    
    result += "\n" + QString("=").repeated(40) + "\n";
    return result;
}

// ========== 功能码路由 ==========

QByteArray ModbusServer::routeFunctionCode(quint8 functionCode, const QByteArray &pdu)
{
    QByteArray response;

    switch (functionCode) {
    // 标准Modbus功能码：01-16
    case ReadCoils:
    case ReadDiscreteInputs:
    case ReadHoldingRegisters:
    case ReadInputRegisters:
    case WriteSingleCoil:
    case WriteSingleRegister:
    case WriteMultipleCoils:
    case WriteMultipleRegisters:
        response = m_functionHandler->processRequest(pdu);
        break;
        
    // 文件记录操作：20-21
    case ReadFileRecord:
        response = m_fileStore->handleReadFileRecord(pdu);
        break;
        
    case WriteFileRecord:
        response = m_fileStore->handleWriteFileRecord(pdu);
        break;
        
    // 自定义文件操作：203-204
    case ReadFile:
        response = m_addressStore->handleReadFile(pdu);
        break;
        
    case WriteFile:
        response = m_addressStore->handleWriteFile(pdu);
        break;
        
    default:
        // 返回非法功能错误
        response.append(static_cast<char>(functionCode | 0x80));
        response.append(static_cast<char>(IllegalFunction));
        break;
    }

    if (response.isEmpty()) {
        qWarning() << "功能码" << functionCode << "处理失败，返回空响应";
    }

    return response;
}

int ModbusServer::getExpectedFrameLength(quint8 functionCode, const QByteArray &buffer)
{
    // RTU帧结构：从站地址(1) + 功能码(1) + 数据(N) + CRC(2)
    int minLength = 4;  // 最小长度
    
    if (buffer.size() < 3) {
        return -1;  // 数据不足，无法判断
    }
    
    switch (functionCode) {
    case ReadCoils:           // 0x01
    case ReadDiscreteInputs:  // 0x02
    case ReadHoldingRegisters:// 0x03
    case ReadInputRegisters:  // 0x04
        // 读请求：从站地址 + 功能码 + 起始地址(2) + 数量(2) + CRC(2) = 8字节
        return 8;
        
    case WriteSingleCoil:     // 0x05
    case WriteSingleRegister: // 0x06
        // 写单个：从站地址 + 功能码 + 地址(2) + 值(2) + CRC(2) = 8字节
        return 8;
        
    case WriteMultipleCoils:  // 0x0F
    case WriteMultipleRegisters: // 0x10
        if (buffer.size() >= 7) {
            quint8 byteCount = static_cast<quint8>(buffer.at(6));
            // 从站地址 + 功能码 + 起始地址(2) + 数量(2) + 字节数(1) + 数据(N) + CRC(2)
            return 7 + byteCount + 2;
        }
        return -1;
        
    case ReadFileRecord:  // 0x14 (20)
        if (buffer.size() >= 3) {
            quint8 byteCount = static_cast<quint8>(buffer.at(2));
            // 从站地址 + 功能码 + 字节数(1) + 数据(N) + CRC(2)
            return 3 + byteCount + 2;
        }
        return -1;
        
    case WriteFileRecord: // 0x15 (21)
        if (buffer.size() >= 3) {
            quint8 byteCount = static_cast<quint8>(buffer.at(2));
            // 从站地址 + 功能码 + 字节数(1) + 数据(N) + CRC(2)
            return 3 + byteCount + 2;
        }
        return -1;
        
    case ReadFile:    // 0xCB (203)
    case WriteFile:   // 0xCC (204)
        // 自定义功能码：从站地址 + 功能码 + 文件编号(2) + CRC(2) = 6字节
        return 6;
        
    default:
        // 未知功能码，返回最小长度
        return minLength;
    }
}

// ========== 辅助方法 ==========

QString ModbusServer::formatPacket(const QByteArray &data, const QString &prefix)
{
    QString result = prefix + " [" + QString::number(data.size()) + " 字节]: ";
    for (int i = 0; i < data.size(); ++i) {
        result += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper();
    }
    return result;
}

void ModbusServer::setStatusMessage(const QString &message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged(message);
        qDebug() << message;
    }
}

void ModbusServer::incrementRequestCount()
{
    m_requestCount++;
    emit requestCountChanged(m_requestCount);
}
