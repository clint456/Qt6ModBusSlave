/**
 * @file ModbusServer.h
 * @brief Modbus服务器头文件
 * 
 * 提供Modbus TCP和RTU服务器功能，支持标准功能码和文件记录操作
 */

#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSerialPort>
#include <QTimer>
#include "ModbusTypes.h"
#include "ModbusDataStore.h"
#include "ModbusFunctionHandler.h"
#include "FileStore.h"

// Modbus TCP/RTU 服务器
class ModbusServer : public QObject
{
    Q_OBJECT
    // 描述：
    // 我有一个叫 running 的属性
    // 类型是 bool
    // 读它用 isRunning()
    // 状态变化用 runningChanged 标记
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(ModbusMode mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int requestCount READ requestCount NOTIFY requestCountChanged)
    Q_PROPERTY(int lastFunctionCode READ lastFunctionCode NOTIFY lastFunctionCodeChanged)

public:
    explicit ModbusServer(QObject *parent = nullptr);
    ~ModbusServer();

    // TCP 服务器控制
    Q_INVOKABLE bool startTcp(quint16 port = 502);
    Q_INVOKABLE void stopTcp();

    // RTU 服务器控制
    Q_INVOKABLE bool startRtu(const QString &portName, int baudRate = 9600);
    Q_INVOKABLE void stopRtu();

    // 通用控制
    Q_INVOKABLE void stop();

    // 数据初始化
    Q_INVOKABLE void initializeData(); 
    
    // 文件查询
    Q_INVOKABLE QStringList getFileList() const;
    Q_INVOKABLE QString queryFileContent(int fileNumber, int maxRecords = 100);

    // 获取器
    bool isRunning() const { return m_running; } // const表示该方法不会修改类的成员变量
    ModbusMode mode() const { return m_mode; }
    QString statusMessage() const { return m_statusMessage; }
    int requestCount() const { return m_requestCount; }
    int lastFunctionCode() const { return m_lastFunctionCode; }

    // 获取数据存储对象（用于UI更新）
    ModbusDataStore* dataStore() const { return m_dataStore; }

signals:
    void runningChanged(bool running);
    void modeChanged(ModbusMode mode);
    void statusMessageChanged(const QString &message);
    void requestCountChanged(int count);
    void lastFunctionCodeChanged(int functionCode);
    void requestReceived(quint8 functionCode);
    void errorOccurred(const QString &error);

private slots:
    void onNewTcpConnection();
    void onTcpReadyRead();
    void onTcpDisconnected();
    void onRtuReadyRead();
    void onRtuError(QSerialPort::SerialPortError error);

private:
    QByteArray processTcpRequest(const QByteArray &adu);
    QByteArray processRtuRequest(const QByteArray &adu);
    QByteArray routeFunctionCode(quint8 functionCode, const QByteArray &pdu);
    quint16 calculateCRC(const QByteArray &data);
    int getExpectedFrameLength(quint8 functionCode, const QByteArray &buffer);
    void setStatusMessage(const QString &message);
    void incrementRequestCount();

    // TCP
    QTcpServer *m_tcpServer;
    QList<QTcpSocket*> m_tcpClients;
    QMap<QTcpSocket*, QByteArray> m_tcpBuffers;

    // RTU
    QSerialPort *m_serialPort;
    QByteArray m_rtuBuffer;
    QTimer *m_rtuTimer;

    // 数据存储
    ModbusDataStore *m_dataStore;
    ModbusFunctionHandler *m_functionHandler;
    FileStore *m_fileStore;
    FileAddressStore *m_addressStore;

    // 状态
    bool m_running;
    ModbusMode m_mode;
    QString m_statusMessage;
    int m_requestCount;
    int m_lastFunctionCode;
};

#endif // MODBUSSERVER_H
