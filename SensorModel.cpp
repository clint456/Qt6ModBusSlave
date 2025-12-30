#include "SensorModel.h"
#include "ModbusServer.h"
#include "ModbusDataStore.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrl>

SensorModelManager::SensorModelManager(QObject *parent)
    : QObject(parent)
{
}

bool SensorModelManager::importFromExcel(const QString &filePath)
{
    m_sensors.clear();
    m_lastError.clear();

    // 处理 file:/// URL
    QString localPath = filePath;
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(filePath).toLocalFile();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开文件: %1").arg(file.errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    int lineNumber = 0;
    bool headerSkipped = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // 跳过空行
        if (line.isEmpty()) {
            continue;
        }

        // 跳过标题行
        if (!headerSkipped) {
            if (line.contains("点组") || line.contains("点名称") || line.contains("序号")) {
                headerSkipped = true;
                continue;
            }
        }

        // 解析 CSV 或 Tab 分隔的数据
        QStringList fields;
        if (line.contains('\t')) {
            fields = line.split('\t', Qt::SkipEmptyParts);
        } else if (line.contains(',')) {
            fields = line.split(',', Qt::SkipEmptyParts);
        } else {
            continue;
        }

        if (fields.size() < 3) {
            continue; // 跳过字段不足的行
        }

        SensorItem item;
        if (parseExcelRow(fields, item)) {
            m_sensors.append(item);
        }
    }

    file.close();

    if (m_sensors.isEmpty()) {
        m_lastError = "未找到有效的传感器数据";
        emit errorOccurred(m_lastError);
        return false;
    }

    emit sensorsLoaded(m_sensors.size());
    emit sensorCountChanged(m_sensors.size());
    
    qDebug() << "成功导入" << m_sensors.size() << "个传感器配置";
    return true;
}

bool SensorModelManager::parseExcelRow(const QStringList &fields, SensorItem &item)
{
    if (fields.size() < 3) {
        return false;
    }

    int fieldIndex = 0;

    // 跳过序号列（如果存在）
    if (fields[0].toInt() > 0 || fields[0] == "0") {
        item.index = fields[0].toInt();
        fieldIndex = 1;
    }

    // 点名称
    if (fieldIndex < fields.size()) {
        item.pointName = fields[fieldIndex++].trimmed();
    }

    // 点类型
    if (fieldIndex < fields.size()) {
        item.pointType = fields[fieldIndex++].trimmed();
        
        // 解析点类型
        if (item.pointType.contains("线圈")) {
            item.type = SensorItem::Coil;
        } else if (item.pointType.contains("离散") || item.pointType.contains("高载")) {
            item.type = SensorItem::DiscreteInput;
        } else if (item.pointType.contains("保持")) {
            item.type = SensorItem::HoldingRegister;
        } else if (item.pointType.contains("输入寄存器")) {
            item.type = SensorItem::InputRegister;
        }
    }

    // 初始值
    if (fieldIndex < fields.size()) {
        item.initialValue = fields[fieldIndex++].trimmed();
    }

    // 单位
    if (fieldIndex < fields.size()) {
        item.unit = fields[fieldIndex++].trimmed();
    }

    // 起始值/最小值
    if (fieldIndex < fields.size()) {
        item.minValue = fields[fieldIndex++].trimmed();
    }

    // 最大值
    if (fieldIndex < fields.size()) {
        item.maxValue = fields[fieldIndex++].trimmed();
    }

    // 备注
    if (fieldIndex < fields.size()) {
        item.note = fields[fieldIndex++].trimmed();
    }

    return !item.pointName.isEmpty();
}

bool SensorModelManager::exportToExcel(const QString &filePath)
{
    m_lastError.clear();

    // 处理 file:/// URL
    QString localPath = filePath;
    if (localPath.startsWith("file:///")) {
        localPath = QUrl(filePath).toLocalFile();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法创建文件: %1").arg(file.errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 写入标题行
    out << "序号\t点名称\t点类型\t初始值\t单位\t起始值\t最大值\t备注\n";

    // 写入数据行
    for (int i = 0; i < m_sensors.size(); ++i) {
        const SensorItem &item = m_sensors[i];
        out << (i + 1) << "\t"
            << item.pointName << "\t"
            << item.pointType << "\t"
            << item.initialValue << "\t"
            << item.unit << "\t"
            << item.minValue << "\t"
            << item.maxValue << "\t"
            << item.note << "\n";
    }

    file.close();
    
    qDebug() << "成功导出" << m_sensors.size() << "个传感器配置到" << localPath;
    return true;
}

bool SensorModelManager::applyToServer(QObject *serverObj)
{
    if (!serverObj) {
        m_lastError = "服务器对象为空";
        emit errorOccurred(m_lastError);
        return false;
    }

    ModbusServer *server = qobject_cast<ModbusServer*>(serverObj);
    if (!server) {
        m_lastError = "无效的服务器对象";
        emit errorOccurred(m_lastError);
        return false;
    }

    ModbusDataStore *dataStore = server->dataStore();
    if (!dataStore) {
        m_lastError = "数据存储未初始化";
        emit errorOccurred(m_lastError);
        return false;
    }

    int appliedCount = 0;

    for (const SensorItem &item : m_sensors) {
        bool ok = false;
        
        switch (item.type) {
        case SensorItem::Coil: {
            bool value = (item.initialValue.toInt() != 0) || 
                        item.initialValue.toLower().contains("on") ||
                        item.initialValue.toLower().contains("true");
            dataStore->writeCoil(item.index, value);
            appliedCount++;
            break;
        }
        
        case SensorItem::DiscreteInput: {
            bool value = (item.initialValue.toInt() != 0) || 
                        item.initialValue.toLower().contains("on") ||
                        item.initialValue.toLower().contains("true");
            dataStore->writeDiscreteInput(item.index, value);
            appliedCount++;
            break;
        }
        
        case SensorItem::HoldingRegister: {
            quint16 value = item.initialValue.toUShort(&ok);
            if (ok) {
                dataStore->writeHoldingRegister(item.index, value);
                appliedCount++;
            }
            break;
        }
        
        case SensorItem::InputRegister: {
            quint16 value = item.initialValue.toUShort(&ok);
            if (ok) {
                dataStore->writeInputRegister(item.index, value);
                appliedCount++;
            }
            break;
        }
        }
    }

    qDebug() << "成功应用" << appliedCount << "个传感器配置到服务器";
    return true;
}

QVariantList SensorModelManager::getSensorList() const
{
    QVariantList list;
    
    for (const SensorItem &item : m_sensors) {
        QVariantMap map;
        map["index"] = item.index;
        map["pointName"] = item.pointName;
        map["pointType"] = item.pointType;
        map["initialValue"] = item.initialValue;
        map["unit"] = item.unit;
        map["minValue"] = item.minValue;
        map["maxValue"] = item.maxValue;
        map["note"] = item.note;
        list.append(map);
    }
    
    return list;
}

void SensorModelManager::addSensor(const QString &name, const QString &type, 
                                  const QString &initValue, const QString &unit)
{
    SensorItem item;
    item.index = m_sensors.size();
    item.pointName = name;
    item.pointType = type;
    item.initialValue = initValue;
    item.unit = unit;
    
    // 解析类型
    if (type.contains("线圈")) {
        item.type = SensorItem::Coil;
    } else if (type.contains("离散") || type.contains("高载")) {
        item.type = SensorItem::DiscreteInput;
    } else if (type.contains("保持")) {
        item.type = SensorItem::HoldingRegister;
    } else {
        item.type = SensorItem::InputRegister;
    }
    
    m_sensors.append(item);
    emit sensorCountChanged(m_sensors.size());
}

void SensorModelManager::clearSensors()
{
    m_sensors.clear();
    emit sensorCountChanged(0);
}

QString SensorModelManager::pointTypeToString(SensorItem::PointType type) const
{
    switch (type) {
    case SensorItem::Coil:
        return "线圈";
    case SensorItem::DiscreteInput:
        return "离散输入";
    case SensorItem::HoldingRegister:
        return "保持寄存器";
    case SensorItem::InputRegister:
        return "输入寄存器";
    }
    return "未知";
}
