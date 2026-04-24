/**
 * @file DatabaseManager.h
 * @brief 数据库管理器（支持 SQLite 和 PostgreSQL）
 *
 * 管理 Modbus 数据的持久化存储，包括线圈、离散输入、保持寄存器、输入寄存器和文件记录
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QVector>
#include <QMap>
#include <QBitArray>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    enum DatabaseType {
        SQLite,
        PostgreSQL
    };

    static DatabaseManager& instance();

    // SQLite 连接（推荐，无需额外配置）
    bool connectSQLite(const QString &dbPath = "modbus_data.db");

    // PostgreSQL 连接
    bool connectPostgreSQL(const QString &host, int port, const QString &database,
                           const QString &user, const QString &password);

    void disconnectDatabase();
    bool isConnected() const;
    DatabaseType databaseType() const { return m_dbType; }

    // 初始化表结构
    bool initializeTables();

    // ========== 线圈操作 ==========
    bool readCoil(quint16 address, bool &value);
    bool readCoils(quint16 startAddress, quint16 count, QBitArray &values);
    bool writeCoil(quint16 address, bool value);
    bool writeCoils(quint16 startAddress, const QBitArray &values);

    // ========== 离散输入操作 ==========
    bool readDiscreteInput(quint16 address, bool &value);
    bool readDiscreteInputs(quint16 startAddress, quint16 count, QBitArray &values);
    bool writeDiscreteInput(quint16 address, bool value);

    // ========== 保持寄存器操作 ==========
    bool readHoldingRegister(quint16 address, quint16 &value);
    bool readHoldingRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values);
    bool writeHoldingRegister(quint16 address, quint16 value);
    bool writeHoldingRegisters(quint16 startAddress, const QVector<quint16> &values);

    // ========== 输入寄存器操作 ==========
    bool readInputRegister(quint16 address, quint16 &value);
    bool readInputRegisters(quint16 startAddress, quint16 count, QVector<quint16> &values);
    bool writeInputRegister(quint16 address, quint16 value);

    // ========== 文件记录操作 ==========
    bool createFile(quint16 fileNumber, const QString &description, quint16 totalRecords);
    bool fileExists(quint16 fileNumber);
    bool readFileRecords(quint16 fileNumber, quint16 startRecord, quint16 count, QByteArray &data);
    bool writeFileRecords(quint16 fileNumber, quint16 startRecord, const QByteArray &data);
    QStringList getFileList();
    QString getFileInfo(quint16 fileNumber);
    QMap<quint16, quint16> getAllRecords(quint16 fileNumber, quint16 maxRecords);

    // ========== 清理操作 ==========
    bool clearAllCoils();
    bool clearAllDiscreteInputs();
    bool clearAllHoldingRegisters();
    bool clearAllInputRegisters();
    bool clearAllFiles();
    bool clearAll();

    // ========== 统计信息 ==========
    size_t getCoilCount();
    size_t getDiscreteInputCount();
    size_t getHoldingRegisterCount();
    size_t getInputRegisterCount();
    size_t getFileCount();
    size_t getTotalRecordCount();

    QString lastError() const { return m_lastError; }

signals:
    void connectionStatusChanged(bool connected);
    void databaseError(const QString &error);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool executeQuery(QSqlQuery &query);
    void setLastError(const QString &error);

    // 获取 UPSERT 语句（SQLite 和 PostgreSQL 语法不同）
    QString getUpsertSQL(const QString &table, const QString &keyColumn, 
                         const QStringList &columns) const;

    QSqlDatabase m_database;
    QString m_lastError;
    mutable QMutex m_mutex;
    bool m_connected;
    DatabaseType m_dbType;

    static const QString CONNECTION_NAME;
};

#endif // DATABASEMANAGER_H
