/**
 * @file SensorConfigParser.cpp
 * @brief 传感器配置解析器实现
 */

#include "SensorConfigParser.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

SensorConfigParser::SensorConfigParser(QObject *parent)
    : QObject(parent)
    , m_errorLine(0)
{
}

// ==================== 导入功能 ====================

QList<SensorItem> SensorConfigParser::importFromFile(const QString &filePath, ConfigFormat format)
{
    m_lastError.clear();
    m_errorLine = 0;
    
    QString localPath = normalizeFilePath(filePath);
    
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开文件: %1").arg(file.errorString());
        emit parseError(m_lastError, 0);
        return {};
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    // 自动检测格式
    if (format == ConfigFormat::Auto) {
        format = detectFormat(localPath);
    }
    
    return parseContent(content, format);
}

QList<SensorItem> SensorConfigParser::parseContent(const QString &content, ConfigFormat format)
{
    switch (format) {
    case ConfigFormat::TSV:
        return parseTsvCsv(content, QLatin1Char('\t'));
    case ConfigFormat::CSV:
        return parseTsvCsv(content, QLatin1Char(','));
    case ConfigFormat::JSON:
        return parseJson(content);
    case ConfigFormat::Auto:
        // 尝试检测内容格式
        if (content.trimmed().startsWith(QLatin1Char('['))) {
            return parseJson(content);
        } else if (content.contains(QLatin1Char('\t'))) {
            return parseTsvCsv(content, QLatin1Char('\t'));
        } else {
            return parseTsvCsv(content, QLatin1Char(','));
        }
    }
    return {};
}

// ==================== 导出功能 ====================

bool SensorConfigParser::exportToFile(const QString &filePath, const QList<SensorItem> &sensors,
                                       ConfigFormat format)
{
    m_lastError.clear();
    
    QString localPath = normalizeFilePath(filePath);
    
    // 自动检测格式
    if (format == ConfigFormat::Auto) {
        format = detectFormat(localPath);
    }
    
    QString content = generateContent(sensors, format);
    
    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法创建文件: %1").arg(file.errorString());
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    
    qDebug() << "[ConfigParser] 成功导出" << sensors.count() << "个传感器配置到" << localPath;
    return true;
}

QString SensorConfigParser::generateContent(const QList<SensorItem> &sensors, ConfigFormat format)
{
    switch (format) {
    case ConfigFormat::TSV:
    case ConfigFormat::Auto:
        return generateTsvCsv(sensors, QLatin1Char('\t'));
    case ConfigFormat::CSV:
        return generateTsvCsv(sensors, QLatin1Char(','));
    case ConfigFormat::JSON:
        return generateJson(sensors);
    }
    return {};
}

// ==================== 私有方法 ====================

ConfigFormat SensorConfigParser::detectFormat(const QString &filePath) const
{
    QString lower = filePath.toLower();
    
    if (lower.endsWith(".json")) {
        return ConfigFormat::JSON;
    } else if (lower.endsWith(".csv")) {
        return ConfigFormat::CSV;
    } else {
        // 默认使用TSV (txt, tsv等)
        return ConfigFormat::TSV;
    }
}

QString SensorConfigParser::normalizeFilePath(const QString &path) const
{
    QString localPath = path;
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(path).toLocalFile();
    }
    return localPath;
}

// ==================== TSV/CSV 解析 ====================

QList<SensorItem> SensorConfigParser::parseTsvCsv(const QString &content, QChar separator)
{
    QList<SensorItem> sensors;
    QStringList lines = content.split(QLatin1Char('\n'));
    
    bool headerSkipped = false;
    int lineNumber = 0;
    
    for (const QString &rawLine : lines) {
        lineNumber++;
        QString line = rawLine.trimmed();
        
        // 跳过空行
        if (line.isEmpty()) {
            continue;
        }
        
        // 跳过标题行
        if (!headerSkipped) {
            if (line.contains("地址") || line.contains("点位名称") || 
                line.contains("寄存器类型") || line.contains("Address")) {
                headerSkipped = true;
                continue;
            }
        }
        
        QStringList fields = line.split(separator);
        
        if (fields.count() < 4) {
            continue; // 跳过字段不足的行
        }
        
        SensorItem item;
        if (parseTsvCsvRow(fields, item)) {
            sensors.append(item);
        } else {
            qDebug() << "[ConfigParser] 解析第" << lineNumber << "行失败";
        }
        
        emit parseProgress(lineNumber, lines.count());
    }
    
    qDebug() << "[ConfigParser] 成功解析" << sensors.count() << "个传感器配置";
    return sensors;
}

bool SensorConfigParser::parseTsvCsvRow(const QStringList &fields, SensorItem &item)
{
    if (fields.count() < 4) {
        return false;
    }
    
    int idx = 0;
    
    // 第1列：地址
    item.setAddress(fields[idx++].trimmed().toUShort());
    
    // 第2列：点位名称
    item.setName(fields[idx++].trimmed());
    
    // 第3列：寄存器类型
    item.setPointType(SensorItem::parsePointType(fields[idx++].trimmed()));
    
    // 第4列：初始值
    item.setInitialValue(fields[idx++].trimmed());
    
    // 第5列：描述 (可选)
    if (idx < fields.count()) {
        item.setDescription(fields[idx++].trimmed());
    }
    
    // 第6列：单位 (可选)
    if (idx < fields.count()) {
        item.setUnit(fields[idx++].trimmed());
    }
    
    // 第7列：最小值 (可选)
    if (idx < fields.count()) {
        item.setMinValue(fields[idx++].trimmed());
    }
    
    // 第8列：最大值 (可选)
    if (idx < fields.count()) {
        item.setMaxValue(fields[idx++].trimmed());
    }
    
    // 第9列：只读 (可选)
    if (idx < fields.count()) {
        QString readOnlyStr = fields[idx++].trimmed();
        bool isReadOnly = false;
        if (readOnlyStr == "是" || readOnlyStr == "true" || readOnlyStr == "1"){
            isReadOnly = true;
        }
        else if (readOnlyStr == "否" || readOnlyStr == "false" || readOnlyStr == "0"){
            isReadOnly = false;
        }
        else if (isReadOnly && (item.pointType() == SensorPointType::InputRegister)) {
            // 强制只读点类型为只读类型
            isReadOnly = true;
        }else if (!isReadOnly && (item.pointType() == SensorPointType::DiscreteInput)) {
            // 强制只读点类型为只读类型
            isReadOnly = true;
        }
        item.setReadOnly(isReadOnly);
    }
    
    // 第10列：值类型 (可选)
    if (idx < fields.count()) {
        QString valueTypeStr = fields[idx++].trimmed().toUpper();
        item.setValueType(ModbusValueConverter::parseTypeString(valueTypeStr));
    } else {
        // 根据点类型和初始值自动推断
        if (item.pointType() == SensorPointType::Coil || 
            item.pointType() == SensorPointType::DiscreteInput) {
            item.setValueType(ModbusDataValueType::BOOL);
        } else if (item.initialValue().contains(QLatin1Char('.'))) {
            item.setValueType(ModbusDataValueType::FLOAT32);
        } else {
            item.setValueType(ModbusDataValueType::UINT16);
        }
    }
    
    // 第11列：占用寄存器数 (可选，会覆盖自动计算的值)
    if (idx < fields.count()) {
        QString regCountStr = fields[idx++].trimmed();
        if (!regCountStr.isEmpty()) {
            int regCount = regCountStr.toInt();
            if (regCount > 0) {
                item.setRegisterCount(regCount);
            }
        }
    }
    
    return item.isValid();
}

// ==================== JSON 解析 ====================

QList<SensorItem> SensorConfigParser::parseJson(const QString &content)
{
    QList<SensorItem> sensors;
    
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
    
    if (jsonError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON解析错误: %1").arg(jsonError.errorString());
        emit parseError(m_lastError, 0);
        return {};
    }
    
    if (!doc.isArray()) {
        m_lastError = "JSON格式错误: 根元素必须是数组";
        emit parseError(m_lastError, 0);
        return {};
    }
    
    QJsonArray array = doc.array();
    int index = 0;
    
    for (const QJsonValue &value : array) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject obj = value.toObject();
        QVariantMap map = obj.toVariantMap();
        
        SensorItem item = SensorItem::fromVariantMap(map);
        if (item.isValid()) {
            sensors.append(item);
        }
        
        emit parseProgress(++index, array.count());
    }
    
    qDebug() << "[ConfigParser] 成功解析" << sensors.count() << "个传感器配置 (JSON)";
    return sensors;
}

// ==================== TSV/CSV 生成 ====================

QString SensorConfigParser::generateTsvCsv(const QList<SensorItem> &sensors, QChar separator)
{
    QString content;
    QTextStream out(&content);
    
    // 写入标题行
    out << "地址" << separator
        << "点位名称" << separator
        << "寄存器类型" << separator
        << "初始值" << separator
        << "描述" << separator
        << "单位" << separator
        << "最小值" << separator
        << "最大值" << separator
        << "只读" << separator
        << "值类型" << separator
        << "占用寄存器数" << "\n";
    
    // 写入数据行
    for (const SensorItem &item : sensors) {
        out << item.address() << separator
            << item.name() << separator
            << item.pointTypeString() << separator
            << item.initialValue() << separator
            << item.description() << separator
            << item.unit() << separator
            << item.minValue() << separator
            << item.maxValue() << separator
            << (item.isReadOnly() ? "是" : "否") << separator
            << item.valueTypeString() << separator
            << item.registerCount() << "\n";
    }
    
    return content;
}

// ==================== JSON 生成 ====================

QString SensorConfigParser::generateJson(const QList<SensorItem> &sensors)
{
    QJsonArray array;
    
    for (const SensorItem &item : sensors) {
        QJsonObject obj = QJsonObject::fromVariantMap(item.toVariantMap());
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}
