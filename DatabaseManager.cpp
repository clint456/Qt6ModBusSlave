/**
 * @file DatabaseManager.cpp
 * @brief 数据库管理器实现（支持 SQLite 和 PostgreSQL）
 */

#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QDir>

const QString DatabaseManager::CONNECTION_NAME = "ModbusDatabase";

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), m_connected(false), m_dbType(SQLite)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnectDatabase();
}

bool DatabaseManager::connectSQLite(const QString &dbPath)
{
    QMutexLocker locker(&m_mutex);

    if (m_connected) {
        qDebug() << "[DatabaseManager] 已经连接到数据库";
        return true;
    }

    // 确定数据库文件路径
    QString actualPath = dbPath;
    if (!QDir::isAbsolutePath(dbPath)) {
        actualPath = QCoreApplication::applicationDirPath() + "/" + dbPath;
    }

    // 创建 SQLite 连接
    m_database = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    m_database.setDatabaseName(actualPath);

    if (!m_database.open()) {
        setLastError(QString("SQLite 数据库连接失败: %1").arg(m_database.lastError().text()));
        qCritical() << "[DatabaseManager]" << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    m_connected = true;
    m_dbType = SQLite;
    qDebug() << "[DatabaseManager] 成功连接到 SQLite 数据库:" << actualPath;

    // 启用外键支持
    QSqlQuery query(m_database);
    query.exec("PRAGMA foreign_keys = ON");

    // 初始化表结构
    if (!initializeTables()) {
        qWarning() << "[DatabaseManager] 初始化表结构失败，但连接保持";
    }

    emit connectionStatusChanged(true);
    return true;
}

bool DatabaseManager::connectPostgreSQL(const QString &host, int port, const QString &database,
                                         const QString &user, const QString &password)
{
    QMutexLocker locker(&m_mutex);

    if (m_connected) {
        qDebug() << "[DatabaseManager] 已经连接到数据库";
        return true;
    }

    // 创建 PostgreSQL 连接
    m_database = QSqlDatabase::addDatabase("QPSQL", CONNECTION_NAME);
    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setDatabaseName(database);
    m_database.setUserName(user);
    m_database.setPassword(password);

    if (!m_database.open()) {
        setLastError(QString("PostgreSQL 数据库连接失败: %1").arg(m_database.lastError().text()));
        qCritical() << "[DatabaseManager]" << m_lastError;
        emit databaseError(m_lastError);
        return false;
    }

    m_connected = true;
    m_dbType = PostgreSQL;
    qDebug() << "[DatabaseManager] 成功连接到 PostgreSQL 数据库:" << host << ":" << port << "/" << database;

    // 初始化表结构
    if (!initializeTables()) {
        qWarning() << "[DatabaseManager] 初始化表结构失败，但连接保持";
    }

    emit connectionStatusChanged(true);
    return true;
}

void DatabaseManager::disconnectDatabase()
{
    QMutexLocker locker(&m_mutex);

    if (m_connected) {
        m_database.close();
        m_connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "[DatabaseManager] 数据库连接已关闭";
    }

    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

bool DatabaseManager::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_connected && m_database.isOpen();
}

bool DatabaseManager::initializeTables()
{
    qDebug() << "[DatabaseManager] 开始初始化数据库表结构...";

    QSqlQuery query(m_database);

    // 创建线圈表
    QString createCoilsTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_coils (
            address INTEGER PRIMARY KEY,
            value BOOLEAN NOT NULL DEFAULT FALSE,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // 创建离散输入表
    QString createDiscreteInputsTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_discrete_inputs (
            address INTEGER PRIMARY KEY,
            value BOOLEAN NOT NULL DEFAULT FALSE,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // 创建保持寄存器表
    QString createHoldingRegistersTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_holding_registers (
            address INTEGER PRIMARY KEY,
            value INTEGER NOT NULL DEFAULT 0,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // 创建输入寄存器表
    QString createInputRegistersTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_input_registers (
            address INTEGER PRIMARY KEY,
            value INTEGER NOT NULL DEFAULT 0,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // 创建文件信息表
    QString createFilesTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_files (
            file_number INTEGER PRIMARY KEY,
            description TEXT,
            total_records INTEGER NOT NULL DEFAULT 10000,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    // 创建文件记录表
    QString createFileRecordsTable = R"(
        CREATE TABLE IF NOT EXISTS modbus_file_records (
            file_number INTEGER NOT NULL,
            record_number INTEGER NOT NULL,
            data BYTEA NOT NULL,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (file_number, record_number),
            FOREIGN KEY (file_number) REFERENCES modbus_files(file_number) ON DELETE CASCADE
        )
    )";

    // 创建索引以提高查询性能
    QString createIndexes = R"(
        CREATE INDEX IF NOT EXISTS idx_file_records_file ON modbus_file_records(file_number);
    )";

    QStringList queries = {
        createCoilsTable,
        createDiscreteInputsTable,
        createHoldingRegistersTable,
        createInputRegistersTable,
        createFilesTable,
        createFileRecordsTable,
        createIndexes
    };

    for (const QString &sql : queries) {
        if (!query.exec(sql)) {
            setLastError(QString("创建表失败: %1").arg(query.lastError().text()));
            qCritical() << "[DatabaseManager]" << m_lastError;
            return false;
        }
    }

    qDebug() << "[DatabaseManager] 数据库表结构初始化完成";
    return true;
}

bool DatabaseManager::executeQuery(QSqlQuery &query)
{
    if (!query.exec()) {
        setLastError(query.lastError().text());
        qWarning() << "[DatabaseManager] 查询执行失败:" << m_lastError;
        return false;
    }
    return true;
}

void DatabaseManager::setLastError(const QString &error)
{
    m_lastError = error;
    emit databaseError(error);
}

// ========== 线圈操作 ==========

bool DatabaseManager::readCoil(quint16 address, bool &value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM modbus_coils WHERE address = :address");
    query.bindValue(":address", address);

    if (!executeQuery(query)) return false;

    if (query.next()) {
        value = query.value(0).toBool();
    } else {
        value = false; // 默认值
    }
    return true;
}

bool DatabaseManager::readCoils(quint16 startAddress, quint16 count, QBitArray &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    values.resize(count);
    values.fill(false);

    QSqlQuery query(m_database);
    query.prepare("SELECT address, value FROM modbus_coils WHERE address >= :start AND address < :end");
    query.bindValue(":start", startAddress);
    query.bindValue(":end", startAddress + count);

    if (!executeQuery(query)) return false;

    while (query.next()) {
        int addr = query.value(0).toInt();
        bool val = query.value(1).toBool();
        int index = addr - startAddress;
        if (index >= 0 && index < count) {
            values.setBit(index, val);
        }
    }
    return true;
}

bool DatabaseManager::writeCoil(quint16 address, bool value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_coils (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = :value2, updated_at = CURRENT_TIMESTAMP
    )");
    query.bindValue(":address", address);
    query.bindValue(":value", value);
    query.bindValue(":value2", value);

    return executeQuery(query);
}

bool DatabaseManager::writeCoils(quint16 startAddress, const QBitArray &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);

    // 使用事务批量写入
    m_database.transaction();

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_coils (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = EXCLUDED.value, updated_at = CURRENT_TIMESTAMP
    )");

    for (int i = 0; i < values.size(); ++i) {
        query.bindValue(":address", startAddress + i);
        query.bindValue(":value", values.testBit(i));
        if (!query.exec()) {
            m_database.rollback();
            setLastError(query.lastError().text());
            return false;
        }
    }

    return m_database.commit();
}

// ========== 离散输入操作 ==========

bool DatabaseManager::readDiscreteInput(quint16 address, bool &value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM modbus_discrete_inputs WHERE address = :address");
    query.bindValue(":address", address);

    if (!executeQuery(query)) return false;

    if (query.next()) {
        value = query.value(0).toBool();
    } else {
        value = false;
    }
    return true;
}

bool DatabaseManager::readDiscreteInputs(quint16 startAddress, quint16 count, QBitArray &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    values.resize(count);
    values.fill(false);

    QSqlQuery query(m_database);
    query.prepare("SELECT address, value FROM modbus_discrete_inputs WHERE address >= :start AND address < :end");
    query.bindValue(":start", startAddress);
    query.bindValue(":end", startAddress + count);

    if (!executeQuery(query)) return false;

    while (query.next()) {
        int addr = query.value(0).toInt();
        bool val = query.value(1).toBool();
        int index = addr - startAddress;
        if (index >= 0 && index < count) {
            values.setBit(index, val);
        }
    }
    return true;
}

bool DatabaseManager::writeDiscreteInput(quint16 address, bool value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_discrete_inputs (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = :value2, updated_at = CURRENT_TIMESTAMP
    )");
    query.bindValue(":address", address);
    query.bindValue(":value", value);
    query.bindValue(":value2", value);

    return executeQuery(query);
}

// ========== 保持寄存器操作 ==========

bool DatabaseManager::readHoldingRegister(quint16 address, quint16 &value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM modbus_holding_registers WHERE address = :address");
    query.bindValue(":address", address);

    if (!executeQuery(query)) return false;

    if (query.next()) {
        value = static_cast<quint16>(query.value(0).toUInt());
    } else {
        value = 0;
    }
    return true;
}

bool DatabaseManager::readHoldingRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    values.clear();
    values.resize(count, 0);

    QSqlQuery query(m_database);
    query.prepare("SELECT address, value FROM modbus_holding_registers WHERE address >= :start AND address < :end ORDER BY address");
    query.bindValue(":start", startAddress);
    query.bindValue(":end", startAddress + count);

    if (!executeQuery(query)) return false;

    while (query.next()) {
        int addr = query.value(0).toInt();
        quint16 val = static_cast<quint16>(query.value(1).toUInt());
        int index = addr - startAddress;
        if (index >= 0 && index < count) {
            values[index] = val;
        }
    }
    return true;
}

bool DatabaseManager::writeHoldingRegister(quint16 address, quint16 value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_holding_registers (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = :value2, updated_at = CURRENT_TIMESTAMP
    )");
    query.bindValue(":address", address);
    query.bindValue(":value", value);
    query.bindValue(":value2", value);

    return executeQuery(query);
}

bool DatabaseManager::writeHoldingRegisters(quint16 startAddress, const QVector<quint16> &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);

    m_database.transaction();

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_holding_registers (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = EXCLUDED.value, updated_at = CURRENT_TIMESTAMP
    )");

    for (int i = 0; i < values.size(); ++i) {
        query.bindValue(":address", startAddress + i);
        query.bindValue(":value", values[i]);
        if (!query.exec()) {
            m_database.rollback();
            setLastError(query.lastError().text());
            return false;
        }
    }

    return m_database.commit();
}

// ========== 输入寄存器操作 ==========

bool DatabaseManager::readInputRegister(quint16 address, quint16 &value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM modbus_input_registers WHERE address = :address");
    query.bindValue(":address", address);

    if (!executeQuery(query)) return false;

    if (query.next()) {
        value = static_cast<quint16>(query.value(0).toUInt());
    } else {
        value = 0;
    }
    return true;
}

bool DatabaseManager::readInputRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    values.clear();
    values.resize(count, 0);

    QSqlQuery query(m_database);
    query.prepare("SELECT address, value FROM modbus_input_registers WHERE address >= :start AND address < :end ORDER BY address");
    query.bindValue(":start", startAddress);
    query.bindValue(":end", startAddress + count);

    if (!executeQuery(query)) return false;

    while (query.next()) {
        int addr = query.value(0).toInt();
        quint16 val = static_cast<quint16>(query.value(1).toUInt());
        int index = addr - startAddress;
        if (index >= 0 && index < count) {
            values[index] = val;
        }
    }
    return true;
}

bool DatabaseManager::writeInputRegister(quint16 address, quint16 value)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_input_registers (address, value, updated_at)
        VALUES (:address, :value, CURRENT_TIMESTAMP)
        ON CONFLICT (address) DO UPDATE SET value = :value2, updated_at = CURRENT_TIMESTAMP
    )");
    query.bindValue(":address", address);
    query.bindValue(":value", value);
    query.bindValue(":value2", value);

    return executeQuery(query);
}

// ========== 文件记录操作 ==========

bool DatabaseManager::createFile(quint16 fileNumber, const QString &description, quint16 totalRecords)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_files (file_number, description, total_records, created_at)
        VALUES (:file_number, :description, :total_records, CURRENT_TIMESTAMP)
        ON CONFLICT (file_number) DO NOTHING
    )");
    query.bindValue(":file_number", fileNumber);
    query.bindValue(":description", description);
    query.bindValue(":total_records", totalRecords);

    return executeQuery(query);
}

bool DatabaseManager::fileExists(quint16 fileNumber)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM modbus_files WHERE file_number = :file_number");
    query.bindValue(":file_number", fileNumber);

    if (!executeQuery(query)) return false;
    return query.next();
}

bool DatabaseManager::readFileRecords(quint16 fileNumber, quint16 startRecord, quint16 count, QByteArray &data)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);

    // 先检查文件是否存在
    QSqlQuery checkQuery(m_database);
    checkQuery.prepare("SELECT total_records FROM modbus_files WHERE file_number = :file_number");
    checkQuery.bindValue(":file_number", fileNumber);

    if (!executeQuery(checkQuery) || !checkQuery.next()) {
        setLastError("文件不存在");
        return false;
    }

    // 读取记录
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT record_number, data FROM modbus_file_records
        WHERE file_number = :file_number AND record_number >= :start AND record_number < :end
        ORDER BY record_number
    )");
    query.bindValue(":file_number", fileNumber);
    query.bindValue(":start", startRecord);
    query.bindValue(":end", startRecord + count);

    if (!executeQuery(query)) return false;

    // 创建结果数组，每条记录2字节
    data.clear();
    data.resize(count * 2);
    data.fill(0);

    while (query.next()) {
        int recordNum = query.value(0).toInt();
        QByteArray recordData = query.value(1).toByteArray();
        int offset = (recordNum - startRecord) * 2;
        if (offset >= 0 && offset + recordData.size() <= data.size()) {
            for (int i = 0; i < recordData.size() && offset + i < data.size(); ++i) {
                data[offset + i] = recordData[i];
            }
        }
    }

    return true;
}

bool DatabaseManager::writeFileRecords(quint16 fileNumber, quint16 startRecord, const QByteArray &data)
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);

    // 确保文件存在，如果不存在则创建
    if (!fileExists(fileNumber)) {
        // 需要先解锁才能调用 createFile
        locker.unlock();
        if (!createFile(fileNumber, QString("自动创建的文件 %1").arg(fileNumber), 10000)) {
            return false;
        }
        locker.relock();
    }

    m_database.transaction();

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO modbus_file_records (file_number, record_number, data, updated_at)
        VALUES (:file_number, :record_number, :data, CURRENT_TIMESTAMP)
        ON CONFLICT (file_number, record_number) DO UPDATE SET data = EXCLUDED.data, updated_at = CURRENT_TIMESTAMP
    )");

    int recordCount = data.size() / 2;
    for (int i = 0; i < recordCount; ++i) {
        query.bindValue(":file_number", fileNumber);
        query.bindValue(":record_number", startRecord + i);
        query.bindValue(":data", data.mid(i * 2, 2));
        if (!query.exec()) {
            m_database.rollback();
            setLastError(query.lastError().text());
            return false;
        }
    }

    return m_database.commit();
}

QStringList DatabaseManager::getFileList()
{
    QStringList result;
    if (!isConnected()) return result;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare("SELECT file_number, description FROM modbus_files ORDER BY file_number");

    if (!executeQuery(query)) return result;

    while (query.next()) {
        result.append(QString("文件 %1: %2").arg(query.value(0).toInt()).arg(query.value(1).toString()));
    }
    return result;
}

QString DatabaseManager::getFileInfo(quint16 fileNumber)
{
    if (!isConnected()) return QString();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT f.file_number, f.description, f.total_records, COUNT(r.record_number) as used_records
        FROM modbus_files f
        LEFT JOIN modbus_file_records r ON f.file_number = r.file_number
        WHERE f.file_number = :file_number
        GROUP BY f.file_number, f.description, f.total_records
    )");
    query.bindValue(":file_number", fileNumber);

    if (!executeQuery(query) || !query.next()) {
        return QString();
    }

    return QString("文件号: %1, 描述: %2, 总记录数: %3, 已使用: %4")
        .arg(query.value(0).toInt())
        .arg(query.value(1).toString())
        .arg(query.value(2).toInt())
        .arg(query.value(3).toInt());
}

QMap<quint16, quint16> DatabaseManager::getAllRecords(quint16 fileNumber, quint16 maxRecords)
{
    QMap<quint16, quint16> result;
    if (!isConnected()) return result;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT record_number, data FROM modbus_file_records
        WHERE file_number = :file_number
        ORDER BY record_number
        LIMIT :limit
    )");
    query.bindValue(":file_number", fileNumber);
    query.bindValue(":limit", maxRecords);

    if (!executeQuery(query)) return result;

    while (query.next()) {
        quint16 recordNum = static_cast<quint16>(query.value(0).toInt());
        QByteArray data = query.value(1).toByteArray();
        if (data.size() >= 2) {
            quint16 value = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
            result[recordNum] = value;
        }
    }
    return result;
}

// ========== 清理操作 ==========

bool DatabaseManager::clearAllCoils()
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec("DELETE FROM modbus_coils");
}

bool DatabaseManager::clearAllDiscreteInputs()
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec("DELETE FROM modbus_discrete_inputs");
}

bool DatabaseManager::clearAllHoldingRegisters()
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec("DELETE FROM modbus_holding_registers");
}

bool DatabaseManager::clearAllInputRegisters()
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    return query.exec("DELETE FROM modbus_input_registers");
}

bool DatabaseManager::clearAllFiles()
{
    if (!isConnected()) return false;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    // 由于 ON DELETE CASCADE，删除文件会自动删除记录
    return query.exec("DELETE FROM modbus_files");
}

bool DatabaseManager::clearAll()
{
    return clearAllCoils() &&
           clearAllDiscreteInputs() &&
           clearAllHoldingRegisters() &&
           clearAllInputRegisters() &&
           clearAllFiles();
}

// ========== 统计信息 ==========

size_t DatabaseManager::getCoilCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_coils") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}

size_t DatabaseManager::getDiscreteInputCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_discrete_inputs") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}

size_t DatabaseManager::getHoldingRegisterCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_holding_registers") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}

size_t DatabaseManager::getInputRegisterCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_input_registers") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}

size_t DatabaseManager::getFileCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_files") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}

size_t DatabaseManager::getTotalRecordCount()
{
    if (!isConnected()) return 0;

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM modbus_file_records") && query.next()) {
        return static_cast<size_t>(query.value(0).toLongLong());
    }
    return 0;
}
