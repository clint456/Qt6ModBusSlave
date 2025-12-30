/**
 * @file FileStore.h
 * @brief 文件记录存储头文件
 * 
 * 定义Modbus文件记录的数据结构和操作接口
 */

#ifndef FILESTORE_H
#define FILESTORE_H

#include <QObject>
#include <QMap>
#include <QReadWriteLock>
#include "ModbusTypes.h"

// 文件记录数据结构
class FileRecord
{
public:
    FileRecord(quint16 fileNumber, quint16 totalRecords);

    QByteArray readRecords(quint16 startRecord, quint16 length) const;
    bool writeRecords(quint16 startRecord, const QByteArray &data);

    quint16 fileNumber() const { return m_fileNumber; }
    quint16 totalRecords() const { return m_totalRecords; }
    QString description() const { return m_description; }
    void setDescription(const QString &desc) { m_description = desc; }
    
    QMap<quint16, QByteArray> getAllRecords() const { 
        QReadLocker locker(&m_lock);
        return m_records; 
    }

private:
    quint16 m_fileNumber;
    quint16 m_totalRecords;
    QString m_description;
    QMap<quint16, QByteArray> m_records;  // 记录号 -> 2字节数据
    mutable QReadWriteLock m_lock;
};

// 标准文件存储管理器（功能码 20/21）
class FileStore : public QObject
{
    Q_OBJECT

public:
    explicit FileStore(QObject *parent = nullptr);
    ~FileStore();

    bool createFile(quint16 fileNumber, const QString &description, quint16 totalRecords = 10000);
    QByteArray handleReadFileRecord(const QByteArray &request);
    QByteArray handleWriteFileRecord(const QByteArray &request);
    
    // 查询功能
    QStringList getFileList() const;
    QString getFileInfo(quint16 fileNumber) const;
    QMap<quint16, quint16> getAllRecords(quint16 fileNumber, quint16 maxRecords = 100) const;

signals:
    void fileRead(quint16 fileNumber, quint16 recordNumber, quint16 length);
    void fileWritten(quint16 fileNumber, quint16 recordNumber, int dataSize);

private:
    QByteArray buildErrorResponse(quint8 errorCode, quint8 exceptionCode) const;

    QMap<quint16, FileRecord*> m_files;
    mutable QReadWriteLock m_lock;
};

// 地址文件存储管理器（功能码 203/204）
class FileAddressStore : public QObject
{
    Q_OBJECT

public:
    explicit FileAddressStore(QObject *parent = nullptr);

    void initializeRegion(quint16 startAddress, quint16 count);
    QByteArray handleReadFile(const QByteArray &request);
    QByteArray handleWriteFile(const QByteArray &request);

signals:
    void registerRead(quint16 address, quint16 count);
    void registerWritten(quint16 address, int dataSize);

private:
    QByteArray buildErrorResponse(quint8 errorCode, quint8 exceptionCode) const;

    QMap<quint16, QByteArray> m_data;  // 地址 -> 2字节数据
    mutable QReadWriteLock m_lock;
};

#endif // FILESTORE_H
