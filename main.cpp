#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "ModbusServer.h"
#include "SensorModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qDebug() << "应用程序启动...";

    // 创建 Modbus 服务器
    ModbusServer modbusServer;
    qDebug() << "ModbusServer 已创建";

    // 创建传感器模型管理器
    SensorModelManager sensorManager;
    qDebug() << "SensorModelManager 已创建";

    QQmlApplicationEngine engine;

    // 将对象暴露给 QML
    engine.rootContext()->setContextProperty(QStringLiteral("modbusServer"), &modbusServer);
    engine.rootContext()->setContextProperty(QStringLiteral("sensorManager"), &sensorManager);
    qDebug() << "对象已暴露给 QML";

    // 初始化服务器数据
    modbusServer.initializeData();
    qDebug() << "服务器数据已初始化";

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { 
            qCritical() << "QML 对象创建失败！";
            QCoreApplication::exit(-1); 
        },
        Qt::QueuedConnection);
    
    qDebug() << "准备加载 QML...";
    engine.loadFromModule(QStringLiteral("Qt6ModBusSlave"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "QML 加载失败，没有根对象创建！";
        return -1;
    }

    qDebug() << "QML 加载成功，应用程序运行中...";
    return app.exec();
}
