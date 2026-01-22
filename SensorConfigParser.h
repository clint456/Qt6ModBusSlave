/**
 * @file SensorConfigParser.h
 * @brief 传感器配置解析器
 * 
 * 支持从多种格式（TSV/CSV/JSON）解析传感器配置
 */

#ifndef SENSORCONFIGPARSER_H
#define SENSORCONFIGPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include "SensorItem.h"

/**
 * @brief 配置文件格式
 */
enum class ConfigFormat {
    Auto,       // 自动检测
    TSV,        // Tab分隔值
    CSV,        // 逗号分隔值
    JSON        // JSON格式
};

/**
 * @brief 传感器配置解析器
 * 
 * 负责解析和导出传感器配置文件
 */
class SensorConfigParser : public QObject
{
    Q_OBJECT

public:
    explicit SensorConfigParser(QObject *parent = nullptr);
    
    // ==================== 导入功能 ====================
    
    /**
     * @brief 从文件导入传感器配置
     * @param filePath 文件路径
     * @param format 文件格式（默认自动检测）
     * @return 解析出的传感器列表
     */
    QList<SensorItem> importFromFile(const QString &filePath, 
                                      ConfigFormat format = ConfigFormat::Auto);
    
    /**
     * @brief 从字符串内容解析
     * @param content 文件内容
     * @param format 内容格式
     * @return 解析出的传感器列表
     */
    QList<SensorItem> parseContent(const QString &content, ConfigFormat format);
    
    // ==================== 导出功能 ====================
    
    /**
     * @brief 导出传感器配置到文件
     * @param filePath 文件路径
     * @param sensors 传感器列表
     * @param format 文件格式（默认自动检测）
     * @return 是否成功
     */
    bool exportToFile(const QString &filePath, const QList<SensorItem> &sensors,
                      ConfigFormat format = ConfigFormat::Auto);
    
    /**
     * @brief 生成配置内容字符串
     * @param sensors 传感器列表
     * @param format 格式
     * @return 格式化的配置内容
     */
    QString generateContent(const QList<SensorItem> &sensors, ConfigFormat format);
    
    // ==================== 错误信息 ====================
    
    QString lastError() const { return m_lastError; }
    int errorLine() const { return m_errorLine; }
    
signals:
    void parseProgress(int current, int total);
    void parseError(const QString &error, int line);

private:
    // 格式检测
    ConfigFormat detectFormat(const QString &filePath) const;
    
    // TSV/CSV 解析
    QList<SensorItem> parseTsvCsv(const QString &content, QChar separator);
    bool parseTsvCsvRow(const QStringList &fields, SensorItem &item);
    
    // JSON 解析
    QList<SensorItem> parseJson(const QString &content);
    
    // TSV/CSV 生成
    QString generateTsvCsv(const QList<SensorItem> &sensors, QChar separator);
    
    // JSON 生成
    QString generateJson(const QList<SensorItem> &sensors);
    
    // URL处理
    QString normalizeFilePath(const QString &path) const;
    
    QString m_lastError;
    int m_errorLine;
};

#endif // SENSORCONFIGPARSER_H
