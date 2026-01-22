/**
 * @file SensorModel.h
 * @brief 传感器模型管理器
 * 
 * 提供传感器配置的管理、导入导出和应用到Modbus服务器的功能
 * 使用新的模块化架构: SensorItem + SensorConfigParser + ModbusValueConverter
 */

#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include "SensorItem.h"
#include "SensorConfigParser.h"

// 前向声明:  避免不必要的头文件包、 加快编译速度、打破循环依赖、降低模块耦合度
class ModbusServer;
class ModbusDataStore;

/**
 * @brief 传感器模型管理器
 * 
 * 提供QML接口，管理传感器列表，支持导入导出和应用到服务器
 */
class SensorModelManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sensorCount READ sensorCount NOTIFY sensorCountChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    explicit SensorModelManager(QObject *parent = nullptr);
    ~SensorModelManager() override = default;

    // ==================== 导入导出 ====================
    
    /**
     * @brief 从文件导入传感器配置
     * @param filePath 文件路径 (支持 file:/// URL)
     * @return 是否成功
     */
    Q_INVOKABLE bool importFromFile(const QString &filePath);
    
    /**
     * @brief 导出传感器配置到文件
     * @param filePath 文件路径
     * @return 是否成功
     */
    Q_INVOKABLE bool exportToFile(const QString &filePath);
    
    // 兼容旧API
    Q_INVOKABLE bool importFromExcel(const QString &filePath) { return importFromFile(filePath); }
    Q_INVOKABLE bool exportToExcel(const QString &filePath) { return exportToFile(filePath); }
    
    // ==================== 应用到服务器 ====================
    
    /**
     * @brief 将传感器配置应用到Modbus服务器
     * @param modbusServer Modbus服务器对象
     * @return 是否成功
     */
    Q_INVOKABLE bool applyToServer(QObject *modbusServer);
    
    // ==================== 传感器管理 ====================
    
    /**
     * @brief 获取传感器列表（用于QML）
     * @return 传感器列表的QVariantList
     */
    Q_INVOKABLE QVariantList getSensorList() const;
    
    /**
     * @brief 获取传感器数量
     */
    Q_INVOKABLE int sensorCount() const { return m_sensors.count(); }
    
    /**
     * @brief 添加传感器
     */
    Q_INVOKABLE void addSensor(const QString &name, const QString &type,
                               const QString &initValue, const QString &unit);
    
    /**
     * @brief 清空所有传感器
     */
    Q_INVOKABLE void clearSensors();
    
    /**
     * @brief 获取最后的错误信息
     */
    Q_INVOKABLE QString lastError() const { return m_lastError; }
    Q_INVOKABLE QString getLastError() const { return m_lastError; }
    
    // ==================== 内部访问 ====================
    
    /**
     * @brief 获取传感器列表（C++内部使用）
     */
    const QList<SensorItem>& sensors() const { return m_sensors; }

signals:
    void sensorCountChanged(int count);
    void sensorsLoaded(int count);
    void errorOccurred(const QString &error);
    void applyProgress(int current, int total);

private:
    // 应用单个传感器到数据存储
    bool applySensorToDataStore(const SensorItem &item, ModbusDataStore *dataStore);
    
    // 检查地址冲突
    void checkAddressConflicts() const;

    QList<SensorItem> m_sensors;
    SensorConfigParser m_parser;
    QString m_lastError;
};

#endif // SENSORMODEL_H
