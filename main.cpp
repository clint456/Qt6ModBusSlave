#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "ModbusServer.h"
#include "SensorModel.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv); // 创建qt应用程序
 
    qDebug() << "应用程序启动...";

    // 初始化数据库连接
    qDebug() << "正在连接 PostgreSQL 数据库...";
    DatabaseManager& dbManager = DatabaseManager::instance();
    bool dbConnected = dbManager.connectPostgreSQL(
        "localhost",    // 主机
        5555,           // 端口
        "postgres",     // 数据库名
        "postgres",     // 用户名
        "158023"        // 密码
    );
    
    if (dbConnected) {
        qDebug() << "PostgreSQL 数据库连接成功！";
    } else {
        qWarning() << "PostgreSQL 数据库连接失败：" << dbManager.lastError();
        qWarning() << "将使用内存模式运行（数据不会持久化）";
    }

    // 创建 Modbus 服务器
    ModbusServer modbusServer;
    qDebug() << "ModbusServer 已创建";

    // 启用数据库模式（如果连接成功）
    if (dbConnected) {
        modbusServer.dataStore()->setDatabaseEnabled(true);
        modbusServer.dataStore()->loadFromDatabase();
        qDebug() << "数据库模式已启用";
    }

    // 创建传感器模型管理器
    SensorModelManager sensorManager;
    qDebug() << "SensorModelManager 已创建";

    QQmlApplicationEngine engine; // 创建QML引擎

    // 将C++对象暴露给 QML
    // 这可以让QML对象直接访问C++对象
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
    // 加载QML模块（QT6新特性，替代旧版的load()【动态加载方法】）
    engine.loadFromModule(QStringLiteral("Qt6ModBusSlave"), QStringLiteral("Main")); 

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "QML 加载失败，没有根对象创建！";
        return -1;
    }

    qDebug() << "QML 加载成功，应用程序运行中...";
    return app.exec();
}
