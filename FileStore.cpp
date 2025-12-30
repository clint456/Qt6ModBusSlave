/**
 * @file FileStore.cpp
 * @brief Modbus文件记录存储实现
 * 
 * 实现Modbus功能码20(读文件记录)和21(写文件记录)
 * 支持最多10000条记录，每条记录2字节
 */

#include "FileStore.h"
#include <QtEndian>
#include <QDebug>

// ========== FileRecord 实现 ==========

FileRecord::FileRecord(quint16 fileNumber, quint16 totalRecords)
    : m_fileNumber(fileNumber)
    , m_totalRecords(totalRecords)
{
}

QByteArray FileRecord::readRecords(quint16 startRecord, quint16 length) const
{
    QReadLocker locker(&m_lock);

    if (startRecord + length > m_totalRecords) {
        return QByteArray();
    }

    QByteArray data;
    for (quint16 i = 0; i < length; ++i) {
        quint16 recordNum = startRecord + i;
        QByteArray recordData = m_records.value(recordNum, QByteArray(2, 0));
        data.append(recordData);
    }

    return data;
}

bool FileRecord::writeRecords(quint16 startRecord, const QByteArray &data)
{
    QWriteLocker locker(&m_lock);

    int recordLength = data.size() / 2;
    if (startRecord + recordLength > m_totalRecords) {
        return false;
    }

    for (int i = 0; i < recordLength; ++i) {
        quint16 recordNum = startRecord + i;
        QByteArray recordData = data.mid(i * 2, 2);
        m_records[recordNum] = recordData;
    }

    return true;
}

// ========== FileStore 实现 ==========

FileStore::FileStore(QObject *parent)
    : QObject(parent)
{
}

FileStore::~FileStore()
{
    QWriteLocker locker(&m_lock);
    qDeleteAll(m_files);
    m_files.clear();
}

bool FileStore::createFile(quint16 fileNumber, const QString &description, quint16 totalRecords)
{
    QWriteLocker locker(&m_lock);

    if (m_files.contains(fileNumber)) {
        return false;
    }

    FileRecord *file = new FileRecord(fileNumber, totalRecords);
    file->setDescription(description);
    m_files[fileNumber] = file;

    return true;
}

QByteArray FileStore::handleReadFileRecord(const QByteArray &request)
{
    // 验证最小长度（功能码 + 字节计数 + 参考类型 + 文件号 + 记录号 + 记录长度）
    if (request.size() < 8) {
        return buildErrorResponse(0x94, IllegalDataValue);
    }

    // 解析请求参数
    quint8 refType = static_cast<quint8>(request[2]);
    
    // 验证参考类型必须为6（Modbus标准）
    if (refType != 6) {
        return buildErrorResponse(0x94, IllegalDataValue);
    }

    quint16 fileNumber = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));
    quint16 recordNumber = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 5));
    quint16 recordLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 7));

    // Modbus标准限制：最多126个记录（每个记录2字节 = 252字节）
    // ByteCount字段只有1字节，最大值255 = 1(SubRespLength) + 1(RefType) + 252(数据) + 1(余量)
    if (recordLength > 126) {
        return buildErrorResponse(0x94, IllegalDataValue);
    }

    // 验证记录号范围
    if (recordNumber > 9999) {
        return buildErrorResponse(0x94, IllegalDataAddress);
    }

    // 查找文件
    QReadLocker locker(&m_lock);
    if (!m_files.contains(fileNumber)) {
        qDebug() << "错误: 文件不存在，文件号:" << fileNumber;
        qDebug() << "当前已创建的文件:" << m_files.keys();
        return buildErrorResponse(0x94, IllegalDataAddress);
    }

    FileRecord *file = m_files[fileNumber];
    locker.unlock();

    // 读取数据
    QByteArray recordData = file->readRecords(recordNumber, recordLength);
    if (recordData.isEmpty()) {
        qDebug() << "错误: 读取记录失败";
        return buildErrorResponse(0x94, IllegalDataAddress);
    }

    // 构建响应（严格遵守Modbus标准格式）
    // 格式：功能码(1) + ByteCount(1) + 子响应长度(1) + 参考类型(1) + 数据(N)
    // ByteCount = 子响应长度字段(1) + 参考类型(1) + 数据(N)
    // 子响应长度 = 参考类型(1) + 数据(N)
    
    quint8 subRespLength = 1 + recordData.size();  // 参考类型(1) + 数据
    quint8 byteCountValue = 1 + subRespLength;      // 子响应长度字段(1) + 子响应内容
    
    qDebug() << "计算响应长度 - 数据:" << recordData.size() << "字节,"
             << "子响应长度:" << subRespLength << "字节,"
             << "ByteCount:" << byteCountValue << "字节";
    
    // 验证长度不会溢出1字节
    if (subRespLength > 255 || byteCountValue > 255) {
        qDebug() << "致命错误: 响应长度超过255字节限制！";
        return buildErrorResponse(0x94, IllegalDataValue);
    }
    
    QByteArray response;
    response.append(static_cast<char>(0x14));                    // 功能码
    response.append(static_cast<char>(byteCountValue));          // ByteCount
    response.append(static_cast<char>(subRespLength));           // 子响应长度
    response.append(static_cast<char>(6));                       // 参考类型
    response.append(recordData);                                  // 记录数据

    qDebug() << "成功读取文件记录，响应总长度:" << response.size();
    qDebug() << "响应格式: FC(1) + ByteCount(" << byteCountValue << ") + SubRespLen(" 
             << subRespLength << ") + RefType(1) + Data(" << recordData.size() << ")";
    qDebug() << "响应前32字节 (十六进制):" << response.left(32).toHex(' ').toUpper();
    if (response.size() > 32) {
        qDebug() << "响应总长度:" << response.size() << "字节 (仅显示前32字节)";
    }
    emit fileRead(fileNumber, recordNumber, recordLength);
    return response;
}

QByteArray FileStore::handleWriteFileRecord(const QByteArray &request)
{
    // 验证最小长度（功能码 + 字节计数 + 参考类型 + 文件号 + 记录号 + 记录长度 + 至少2字节数据）
    if (request.size() < 10) {
        return buildErrorResponse(0x95, IllegalDataValue);
    }

    quint8 refType = static_cast<quint8>(request[2]);
    
    // 验证参考类型必须为6（Modbus标准）
    if (refType != 6) {
        return buildErrorResponse(0x95, IllegalDataValue);
    }

    quint16 fileNumber = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));
    quint16 recordNumber = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 5));
    quint16 recordLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 7));
    QByteArray recordData = request.mid(9);

    // Modbus标准限制：最多126个记录（与读操作一致）
    if (recordLength > 126) {
        return buildErrorResponse(0x95, IllegalDataValue);
    }

    // 验证数据长度（每个记录2字节）
    if (recordData.size() != recordLength * 2) {
        return buildErrorResponse(0x95, IllegalDataValue);
    }

    // 验证记录号范围
    if (recordNumber > 9999) {
        return buildErrorResponse(0x95, IllegalDataAddress);
    }

    // 获取或自动创建文件（最多10000条记录）
    QWriteLocker locker(&m_lock);
    if (!m_files.contains(fileNumber)) {
        m_files[fileNumber] = new FileRecord(fileNumber, 10000);
    }

    FileRecord *file = m_files[fileNumber];
    locker.unlock();

    // 写入记录数据
    if (!file->writeRecords(recordNumber, recordData)) {
        return buildErrorResponse(0x95, SlaveDeviceFailure);
    }

    emit fileWritten(fileNumber, recordNumber, recordData.size());

    // 回显请求作为响应（Modbus FC21标准行为）
    return request;
}

QByteArray FileStore::buildErrorResponse(quint8 errorCode, quint8 exceptionCode) const
{
    QByteArray response;
    response.append(static_cast<char>(errorCode));
    response.append(static_cast<char>(exceptionCode));
    return response;
}

// 获取文件列表
QStringList FileStore::getFileList() const
{
    QReadLocker locker(&m_lock);
    QStringList list;
    for (auto it = m_files.begin(); it != m_files.end(); ++it) {
        FileRecord *file = it.value();
        list.append(QString("文件 %1: %2 (%3 记录)")
                    .arg(file->fileNumber())
                    .arg(file->description())
                    .arg(file->totalRecords()));
    }
    return list;
}

// 获取文件信息
QString FileStore::getFileInfo(quint16 fileNumber) const
{
    QReadLocker locker(&m_lock);
    if (!m_files.contains(fileNumber)) {
        return QString("文件 %1 不存在").arg(fileNumber);
    }
    
    FileRecord *file = m_files[fileNumber];
    return QString("文件号: %1\n描述: %2\n总记录数: %3\n已写入记录数: %4")
            .arg(file->fileNumber())
            .arg(file->description())
            .arg(file->totalRecords())
            .arg(file->getAllRecords().size());
}

// 获取所有已写入的记录
QMap<quint16, quint16> FileStore::getAllRecords(quint16 fileNumber, quint16 maxRecords) const
{
    QReadLocker locker(&m_lock);
    QMap<quint16, quint16> result;
    
    if (!m_files.contains(fileNumber)) {
        return result;
    }
    
    FileRecord *file = m_files[fileNumber];
    QMap<quint16, QByteArray> records = file->getAllRecords();
    
    int count = 0;
    for (auto it = records.begin(); it != records.end() && count < maxRecords; ++it, ++count) {
        quint16 recordNum = it.key();
        QByteArray data = it.value();
        
        // 将2字节数据转换为16位整数
        if (data.size() >= 2) {
            quint16 value = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(data.data()));
            result[recordNum] = value;
        }
    }
    
    return result;
}

// ========== FileAddressStore 实现 ==========

FileAddressStore::FileAddressStore(QObject *parent)
    : QObject(parent)
{
}

void FileAddressStore::initializeRegion(quint16 startAddress, quint16 count)
{
    QWriteLocker locker(&m_lock);
    for (quint16 i = 0; i < count; ++i) {
        m_data[startAddress + i] = QByteArray(2, 0);
    }
}

QByteArray FileAddressStore::handleReadFile(const QByteArray &request)
{
    // 解析请求
    if (request.size() < 5) {
        return buildErrorResponse(0xCB, IllegalDataValue);
    }

    quint8 functionCode = static_cast<quint8>(request[0]);  // 203 (0xCB)
    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));

    // 验证请求
    if (quantity == 0 || quantity > 125) {
        return buildErrorResponse(0xCB, IllegalDataValue);
    }

    // 读取数据
    QReadLocker locker(&m_lock);
    QByteArray data;
    for (quint16 i = 0; i < quantity; ++i) {
        quint16 addr = startAddress + i;
        QByteArray regData = m_data.value(addr, QByteArray(2, 0));
        data.append(regData);
    }
    locker.unlock();

    // 构建响应
    QByteArray response;
    response.append(static_cast<char>(0xCB));           // 功能码 203
    response.append(static_cast<char>(data.size()));    // 字节数
    response.append(data);

    emit registerRead(startAddress, quantity);
    return response;
}

QByteArray FileAddressStore::handleWriteFile(const QByteArray &request)
{
    // 解析请求
    if (request.size() < 7) {
        return buildErrorResponse(0xCC, IllegalDataValue);
    }

    quint8 functionCode = static_cast<quint8>(request[0]);  // 204 (0xCC)
    quint16 startAddress = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 1));
    quint16 quantity = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(request.data() + 3));
    quint8 byteCount = static_cast<quint8>(request[5]);
    QByteArray data = request.mid(6);

    // 验证请求
    if (quantity == 0 || quantity > 123) {
        return buildErrorResponse(0xCC, IllegalDataValue);
    }

    if (byteCount != data.size() || byteCount != quantity * 2) {
        return buildErrorResponse(0xCC, IllegalDataValue);
    }

    // 写入数据
    QWriteLocker locker(&m_lock);
    for (quint16 i = 0; i < quantity; ++i) {
        quint16 addr = startAddress + i;
        QByteArray regData = data.mid(i * 2, 2);
        m_data[addr] = regData;
    }
    locker.unlock();

    emit registerWritten(startAddress, data.size());

    // 构建响应
    QByteArray response;
    response.append(static_cast<char>(0xCC));  // 功能码 204
    quint16 addr = qToBigEndian(startAddress);
    response.append(reinterpret_cast<const char*>(&addr), 2);
    quint16 qty = qToBigEndian(quantity);
    response.append(reinterpret_cast<const char*>(&qty), 2);

    return response;
}

QByteArray FileAddressStore::buildErrorResponse(quint8 errorCode, quint8 exceptionCode) const
{
    QByteArray response;
    response.append(static_cast<char>(errorCode));
    response.append(static_cast<char>(exceptionCode));
    return response;
}
