cmake_minimum_required(VERSION 3.16)

project(Qt6ModBusSlave VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network SerialPort)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appQt6ModBusSlave
    main.cpp
    # Modbus 核心模块
    ModbusTypes.h
    ModbusDataStore.h
    ModbusDataStore.cpp
    ModbusFunctionHandler.h
    ModbusFunctionHandler.cpp
    ModbusServer.h
    ModbusServer.cpp
    # 数据转换模块
    ModbusValueConverter.h
    ModbusValueConverter.cpp
    # 传感器配置模块
    SensorItem.h
    SensorItem.cpp
    SensorConfigParser.h
    SensorConfigParser.cpp
    SensorModel.h
    SensorModel.cpp
    # 文件存储模块
    FileStore.h
    FileStore.cpp
)

qt_add_qml_module(appQt6ModBusSlave
    URI Qt6ModBusSlave
    QML_FILES
        Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQt6ModBusSlave PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQt6ModBusSlave
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQt6ModBusSlave
    PRIVATE Qt6::Quick Qt6::Network Qt6::SerialPort
)

include(GNUInstallDirs)
install(TARGETS appQt6ModBusSlave
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
