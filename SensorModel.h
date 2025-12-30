#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QObject>
#include <QString>
#include <QVariant>

// 传感器数据项
class SensorItem
{
public:
    enum PointType {
        Coil,               // 线圈
        DiscreteInput,      // 离散输入
        HoldingRegister,    // 保持寄存器
        InputRegister       // 输入寄存器
    };

    int index;              // 索引
    QString pointName;      // 点名称
    QString pointType;      // 点类型（中文）
    PointType type;         // 点类型（枚举）
    QString initialValue;   // 初始值
    QString unit;           // 单位
    QString minValue;       // 起始值/最小值
    QString maxValue;       // 最大值
    QString note;           // 备注

    SensorItem()
        : index(0), type(Coil) {}

    SensorItem(int idx, const QString &name, const QString &typeStr, 
               const QString &initVal, const QString &unitStr,
               const QString &minVal, const QString &maxVal, const QString &noteStr)
        : index(idx), pointName(name), pointType(typeStr), initialValue(initVal),
          unit(unitStr), minValue(minVal), maxValue(maxVal), note(noteStr)
    {
        // 根据点类型字符串确定枚举类型
        if (pointType == "线圈") {
            type = Coil;
        } else if (pointType == "离散输入" || pointType == "高载输入") {
            type = DiscreteInput;
        } else if (pointType == "保持寄存器") {
            type = HoldingRegister;
        } else if (pointType == "输入寄存器") {
            type = InputRegister;
        }
    }
};

// 传感器模型管理器
class SensorModelManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sensorCount READ sensorCount NOTIFY sensorCountChanged)

public:
    explicit SensorModelManager(QObject *parent = nullptr);

    // Excel 导入导出
    Q_INVOKABLE bool importFromExcel(const QString &filePath);
    Q_INVOKABLE bool exportToExcel(const QString &filePath);
    Q_INVOKABLE QString getLastError() const { return m_lastError; }

    // 应用到 Modbus 服务器
    Q_INVOKABLE bool applyToServer(QObject *modbusServer);

    // 获取传感器数据
    Q_INVOKABLE QVariantList getSensorList() const;
    Q_INVOKABLE int sensorCount() const { return m_sensors.size(); }

    // 添加/删除传感器
    Q_INVOKABLE void addSensor(const QString &name, const QString &type, 
                               const QString &initValue, const QString &unit);
    Q_INVOKABLE void clearSensors();

signals:
    void sensorCountChanged(int count);
    void sensorsLoaded(int count);
    void errorOccurred(const QString &error);

private:
    bool parseExcelRow(const QStringList &row, SensorItem &item);
    QString pointTypeToString(SensorItem::PointType type) const;

    QList<SensorItem> m_sensors;
    QString m_lastError;
};

#endif // SENSORMODEL_H
