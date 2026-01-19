import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1200
    height: 800
    visible: true
    title: "Modbus TCP/RTU从站服务器"
    minimumWidth: 1000
    minimumHeight: 700

    // modbusServer 和 sensorManager 通过 C++ setContextProperty 注入
    // 不需要在这里声明

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // 服务器控制区域
        GroupBox {
            title: "服务器控制"
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            font.bold: false

            RowLayout {
                anchors.fill: parent
                spacing: 15
                // TCP 控制
                GroupBox {
                    title: "TCP 模式"
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10

                        Label { text: "端口:" }
                        TextField {
                            id: tcpPortField
                            text: "502"
                            placeholderText: "502"
                            Layout.preferredWidth: 100
                        }

                        Button {
                            id: startTcpButton
                            text: "启动 TCP"
                            enabled: modbusServer && !modbusServer.running
                            onClicked: {
                                if (modbusServer) {
                                    var port = parseInt(tcpPortField.text)
                                    addLog("尝试启动 TCP 服务器，端口: " + port)
                                    if (modbusServer. startTcp(port)) {
                                        statusLabel.text = "TCP 服务器已启动"
                                        addLog("TCP 服务器启动成功")
                                    } else {




                                        addLog("TCP 服务器启动失败")
                                    }
                                } else {
                                    addLog("错误: ModbusServer 对象未初始化")
                                }
                            }
                        }
                    }
                }

                // RTU 控制
                GroupBox {
                    title:  "RTU 模式"
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.fill: parent
                        spacing: 10

                        Label { text: "串口:" }
                        TextField {
                            id: rtuPortField
                            text: "COM1"
                            placeholderText: "COM1"
                            Layout.preferredWidth: 100
                        }

                        Label { text:  "波特率:" }
                        ComboBox {
                            id:  baudRateCombo
                            model: ["9600", "19200", "38400", "57600", "115200"]
                            currentIndex: 0
                            Layout. preferredWidth: 100
                        }

                        Button {
                            id: startRtuButton
                            text: "启动 RTU"
                            enabled: modbusServer && !modbusServer.running
                            onClicked: {
                                if (modbusServer) {
                                    var baudRate = parseInt(baudRateCombo. currentText)
                                    addLog("尝试启动 RTU 服务器，串口: " + rtuPortField. text + ", 波特率:  " + baudRate)
                                    if (modbusServer.startRtu(rtuPortField. text, baudRate)) {
                                        statusLabel.text = "RTU 服务器已启动"
                                        addLog("RTU 服务器启动成功")
                                    } else {
                                        addLog("RTU 服务器启动失败")
                                    }
                                } else {
                                    addLog("错误: ModbusServer 对象未初始化")
                                }
                            }
                        }
                    }
                }

                // 通用控制
                GroupBox {
                    title: "操作"
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.fill: parent
                        spacing: 8

                        Button {
                            text: "停止服务器"
                            Layout.fillWidth: true
                            enabled: modbusServer && modbusServer.running
                            onClicked: {
                                if (modbusServer) {
                                    modbusServer.stop()
                                    statusLabel.text = "服务器已停止"
                                }
                            }
                        }

                        Button {
                            text: "初始化数据"
                            Layout.fillWidth: true
                            onClicked: {
                                if (modbusServer) {
                                    modbusServer.initializeData()
                                    statusLabel.text = "数据已初始化"
                                }
                            }
                        }
                    }
                }
            }
        }

        // 状态显示区域
        GroupBox {
            title: "服务器状态"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            font.bold: false

            RowLayout {
                anchors.fill: parent
                spacing: 8
                Label {
                    text: "运行状态:"
                    font.bold: false
                }
                Label {
                    id: runningLabel
                    text: modbusServer ? (modbusServer.running ? "● 运行中" : "○ 已停止") : "○ 未知"
                    color: modbusServer && modbusServer.running ? "#27ae60" : "#e74c3c"
                    font.pixelSize: 14
                    font.bold: false
                }

                Label {
                    text: "模式:"
                    font.bold: false
                }
                Label {
                    id: modeLabel
                    text: {
                        if (!modbusServer) return "未知"
                        return modbusServer.mode === 0 ? "TCP" : "RTU"
                    }
                    font.pixelSize: 14
                }

                Label {
                    text: "请求计数:"
                    font.bold: false
                }
                Label {
                    id: requestCountLabel
                    text: modbusServer ? modbusServer.requestCount.toString() : "0"
                    color: "#2980b9"
                    font.pixelSize: 14
                    font.bold: false
                }

                Label {
                    text: "最后功能码:"
                    font.bold: false
                }
                Label {
                    id: lastFcLabel
                    text: {
                        if (!modbusServer || modbusServer.lastFunctionCode === 0) return "无"
                        var fc = modbusServer.lastFunctionCode
                        var fcName = getFunctionCodeName(fc)
                        return fc + " (0x" + fc.toString(16).toUpperCase() + ") - " + fcName
                    }
                    color: "#27ae60"
                    font.pixelSize: 13
                    font.bold: false
                }

                Label {
                    text: "状态消息:"
                    font.bold: false
                }
                Label {
                    id: statusLabel
                    text: modbusServer ? modbusServer.statusMessage : "未启动"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: 13
                    color: "#34495e"
                }
            }
        }

        // 数据监控区域 - 使用 TabBar
        GroupBox {
            title: "文件寄存器与传感器配置"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            font.bold: false

            ColumnLayout {
                anchors.fill: parent
                // spacing: 8

                TabBar {
                    id: dataTabBar
                    Layout.fillWidth: true

                    TabButton {
                        text: "文件寄存器"
                        font.pixelSize: 13
                    }
                    TabButton {
                        text: "传感器配置"
                        font.pixelSize: 13
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: dataTabBar.currentIndex
                    // Tab 1: 文件寄存器
                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            // 使用 RowLayout 实现三栏
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 10

                                // 标准文件记录
                                GroupBox {
                                    title: "标准文件记录 (FC 20/21)"
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    font.bold: false

                                    // 内容用 ColumnLayout + GridLayout 组合
                                    ColumnLayout {
                                        anchors.fill: parent
                                        spacing: 6

                                        GridLayout {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            columns: 2
                                            rowSpacing: 6
                                            columnSpacing: 8

                                            Label { text: "文件号:" }
                                            SpinBox {
                                                id: fileNumberSpinBox
                                                from: 0; to: 65535; value: 1; editable: true
                                                Layout.fillWidth: true
                                            }

                                            Label { text: "记录号:" }
                                            SpinBox {
                                                id: recordNumberSpinBox
                                                from: 0; to: 9999; value: 0; editable: true
                                                Layout.fillWidth: true
                                            }

                                            Label { text: "记录数:" }
                                            SpinBox {
                                                id: recordCountSpinBox
                                                from: 1; to: 126; value: 10; editable: true
                                                Layout.fillWidth: true
                                                ToolTip.visible: hovered
                                                ToolTip.text: "Modbus标准限制：单次最多读取126个记录（252字节）"
                                            }
                                        }

                                        // 按钮区域（靠底）
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignBottom

                                            Button {
                                                text: "🔍 查询文件内容"
                                                Layout.fillWidth: true
                                                onClicked: queryFileContent()
                                                // background: Rectangle {
                                                //     color: parent.hovered ? "#3498db" : "#2980b9"
                                                //     radius: 3
                                                // }
                                                // contentItem: Text {
                                                //     text: parent.text
                                                //     color: "white"
                                                //     horizontalAlignment: Text.AlignHCenter
                                                //     verticalAlignment: Text.AlignVCenter
                                                //     font.pixelSize: 12
                                                //     font.bold: false
                                                // }
                                            }
                                        }
                                    }
                                }

                                // 保持寄存器查询
                                GroupBox {
                                    title: "保持寄存器查询 (FC 3/16)"
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    font.bold: false

                                    ColumnLayout {
                                        anchors.fill: parent
                                        spacing: 6

                                        GridLayout {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            columns: 2
                                            rowSpacing: 6
                                            columnSpacing: 8

                                            Label { text: "起始地址:" }
                                            SpinBox {
                                                id: fileAddressSpinBox
                                                from: 0; to: 65535; value: 1000; editable: true
                                                Layout.fillWidth: true
                                            }

                                            Label { text: "寄存器数:" }
                                            SpinBox {
                                                id: fileRegisterCountSpinBox
                                                from: 1; to: 125; value: 20; editable: true
                                                Layout.fillWidth: true
                                            }
                                        }

                                        // 按钮靠底
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignBottom

                                            Button {
                                                text: "🔍 查询保持寄存器"
                                                Layout.fillWidth: true
                                                onClicked: queryAddressFileContent()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Tab 2: 传感器配置
                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            // Excel 导入导出控制
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40
                                color: "#ecf0f1"
                                radius: 5
                                border.color: "#bdc3c7"
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    // anchors.margins: 15
                                    // spacing: 15

                                    Label {
                                        text: "传感器配置管理"
                                        // font.pixelSize: 14
                                        font.bold: false
                                    }

                                    Rectangle {
                                        width: 1
                                        Layout.fillHeight: true
                                        Layout.topMargin: 10
                                        Layout.bottomMargin: 10
                                        color: "#bdc3c7"
                                    }

                                    Button {
                                        text: "导入 Excel/CSV"
                                        icon.name: "document-import"
                                        Layout.preferredWidth: 140
                                        onClicked: importFileDialog.open()
                                    }

                                    Button {
                                        text: "导出配置"
                                        icon.name: "document-export"
                                        Layout.preferredWidth: 120
                                        onClicked: exportFileDialog.open()
                                    }

                                    Button {
                                        text: "应用到服务器"
                                        icon.name: "application-x-executable"
                                        Layout.preferredWidth: 140
                                        enabled: sensorManager && sensorManager.sensorCount > 0
                                        onClicked: applySensorsToServer()
                                    }

                                    Button {
                                        text: "测试更新"
                                        Layout.preferredWidth: 100
                                        onClicked: {
                                            addLog("===== 测试数据更新 =====")
                                            if (modbusServer && modbusServer.dataStore) {
                                                // 测试写入线圈地址0
                                                addLog("测试写入线圈 地址0 值true")
                                                modbusServer.dataStore.writeCoil(0, true)
                                                
                                                // 测试写入保持寄存器地址0
                                                addLog("测试写入保持寄存器 地址0 值999")
                                                modbusServer.dataStore.writeHoldingRegister(0, 999)
                                            } else {
                                                addLog("错误：无法访问数据存储")
                                            }
                                        }
                                    }

                                    Item { Layout.fillWidth: true }

                                    Label {
                                        text: sensorManager ? ("传感器数: " + sensorManager.sensorCount) : "传感器数: 0"
                                        font.pixelSize: 13
                                        color: "#2980b9"
                                        font.bold: false
                                    }
                                }
                            }

                            // 传感器列表显示
                            GroupBox {
                                title: "传感器列表"
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 0

                                    // 表格标题行
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 35
                                        color: "#e8e8e8"
                                        border.color: "#c0c0c0"
                                        border.width: 1

                                        Row {
                                            anchors.fill: parent
                                            spacing: 0

                                            Rectangle {
                                                width: 60
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "地址"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 180
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "点位名称"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 120
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "寄存器类型"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 80
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "初始值"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 80
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "当前值"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                    color: "#2196F3"
                                                }
                                            }
                                            Rectangle {
                                                width: 200
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "描述"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 100
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "单位"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 80
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "最小值"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 80
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "最大值"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                            Rectangle {
                                                width: 80
                                                height: parent.height
                                                color: "transparent"
                                                border.color: "#c0c0c0"
                                                border.width: 1
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "只读"
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                }
                                            }
                                        }
                                    }

                                    // 数据显示区域 - ListView
                                    ListView {
                                        id: sensorListView
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        clip: true
                                        boundsBehavior: Flickable.StopAtBounds
                                        
                                        model: ListModel {
                                            id: sensorListModel
                                        }

                                        delegate: Rectangle {
                                            width: sensorListView.width
                                            height: 30
                                            color: index % 2 === 0 ? "#ffffff" : "#f5f5f5"
                                            border.color: "#e0e0e0"
                                            border.width: 1

                                            Row {
                                                anchors.fill: parent
                                                spacing: 0

                                                Rectangle {
                                                    width: 60
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.address !== undefined ? model.address : ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 180
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.verticalCenter: parent.verticalCenter
                                                        anchors.left: parent.left
                                                        anchors.leftMargin: 8
                                                        anchors.right: parent.right
                                                        anchors.rightMargin: 8
                                                        text: model.pointName || ""
                                                        font.pixelSize: 11
                                                        elide: Text.ElideRight
                                                    }
                                                }
                                                Rectangle {
                                                    width: 120
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.pointType || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 80
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.initialValue || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 80
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.currentValue || ""
                                                        font.pixelSize: 11
                                                        font.bold: true
                                                        color: "#2196F3"
                                                    }
                                                }
                                                Rectangle {
                                                    width: 200
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.verticalCenter: parent.verticalCenter
                                                        anchors.left: parent.left
                                                        anchors.leftMargin: 8
                                                        anchors.right: parent.right
                                                        anchors.rightMargin: 8
                                                        text: model.note || ""
                                                        font.pixelSize: 11
                                                        elide: Text.ElideRight
                                                    }
                                                }
                                                Rectangle {
                                                    width: 100
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.unit || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 80
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.minValue || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 80
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.maxValue || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                                Rectangle {
                                                    width: 80
                                                    height: parent.height
                                                    color: "transparent"
                                                    border.color: "#e0e0e0"
                                                    border.width: 1
                                                    Label {
                                                        anchors.centerIn: parent
                                                        text: model.readOnly || ""
                                                        font.pixelSize: 11
                                                    }
                                                }
                                            }
                                        }

                                        // 空状态提示
                                        Label {
                                            anchors.centerIn: parent
                                            visible: sensorListModel.count === 0
                                            text: "点击'导入 Excel/CSV'加载传感器配置...\n\n支持格式：\n• CSV 文件 (*.csv)\n• Tab 分隔文件 (*.txt)"
                                            font.pixelSize: 12
                                            color: "#666666"
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        ScrollBar.vertical: ScrollBar {
                                            policy: ScrollBar.AsNeeded
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // 日志区域
        GroupBox {
            title: "操作日志"
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            font.bold: false

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Item { Layout.fillWidth: true }

                    Button {
                        text: "清空日志"
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 25
                        font.pixelSize: 11
                        onClicked: logDisplay.clear()
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: logDisplay
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                        font.family: "Consolas, Monaco, monospace"
                        font.pixelSize: 11
                        text: "服务器日志将显示在这里...\n"
                        background: Rectangle {
                            color: "#2c3e50"
                            radius: 3
                        }
                        color: "#ecf0f1"
                    }
                }
            }
        }
    }

    // 文件导入对话框
    FileDialog {
        id: importFileDialog
        title: "选择传感器配置文件"
        nameFilters: ["文本文件 (*.txt *.csv)", "所有文件 (*)"]
        onAccepted: {
            if (sensorManager) {
                var filePath = currentFile.toString()
                addLog("正在导入文件: " + filePath)
                if (sensorManager.importFromExcel(filePath)) {
                    addLog("成功导入 " + sensorManager.sensorCount + " 个传感器配置")
                    displaySensorList()
                } else {
                    addLog("导入失败")
                }
            } else {
                addLog("错误: 传感器管理器未初始化")
            }
        }
    }

    // 文件导出对话框
    FileDialog {
        id: exportFileDialog
        title: "导出传感器配置"
        fileMode: FileDialog.SaveFile
        nameFilters: ["文本文件 (*.txt)", "CSV文件 (*.csv)"]
        defaultSuffix: "txt"
        onAccepted: {
            if (sensorManager) {
                var filePath = currentFile.toString()
                addLog("正在导出文件: " + filePath)
                if (sensorManager.exportToExcel(filePath)) {
                    addLog("成功导出 " + sensorManager.sensorCount + " 个传感器配置")
                } else {
                    addLog("导出失败")
                }
            } else {
                addLog("错误: 传感器管理器未初始化")
            }
        }
    }

    // 刷新数据显示
    // 查询文件内容
    function queryFileContent() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化")
            return
        }

        var fileNum = fileNumberSpinBox.value
        addLog("正在查询文件 " + fileNum + " 的内容...")
        addLog("")

        // 调用C++后端查询
        var content = modbusServer.queryFileContent(fileNum, 50)

        // 将内容按行输出到日志
        var lines = content.split('\n')
        for (var i = 0; i < lines.length; i++) {
            if (lines[i].trim() !== "") {
                addLog(lines[i])
            }
        }

        addLog("")
    }

    // 读取地址文件（模拟功能码 203）
    // 查询地址文件内容
    function queryAddressFileContent() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化")
            return
        }

        var startAddr = fileAddressSpinBox.value
        var count = fileRegisterCountSpinBox.value
        addLog("正在查询保持寄存器...")
        addLog("")

        // 调用C++后端查询
        var content = modbusServer.queryAddressFile(startAddr, count)

        // 将内容按行输出到日志
        var lines = content.split('\n')
        for (var i = 0; i < lines.length; i++) {
            if (lines[i].trim() !== "") {
                addLog(lines[i])
            }
        }

        addLog("")
    }

    // 显示传感器列表
    function displaySensorList() {
        console.log("===== displaySensorList 开始 =====")
        if (!sensorManager) {
            console.log("错误：sensorManager 不存在")
            sensorListModel.clear()
            return
        }

        var sensors = sensorManager.getSensorList()
        console.log("获取到传感器列表，数量:", sensors ? sensors.length : 0)
        sensorListModel.clear()
        
        if (!sensors || sensors.length === 0) {
            console.log("传感器列表为空")
            return
        }

        for (var i = 0; i < sensors.length; i++) {
            var sensor = sensors[i]
            var currentVal = ""
            
            // 获取当前值
            if (modbusServer && modbusServer.dataStore) {
                if (sensor.pointType === "线圈") {
                    currentVal = modbusServer.dataStore.readCoil(sensor.index) ? "1" : "0"
                } else if (sensor.pointType === "离散输入") {
                    currentVal = modbusServer.dataStore.readDiscreteInput(sensor.index) ? "1" : "0"
                } else if (sensor.pointType === "保持寄存器") {
                    currentVal = String(modbusServer.dataStore.readHoldingRegister(sensor.index))
                } else if (sensor.pointType === "输入寄存器") {
                    currentVal = String(modbusServer.dataStore.readInputRegister(sensor.index))
                }
            }
            
            sensorListModel.append({
                "address": sensor.index,
                "pointName": sensor.pointName ,
                "pointType": sensor.pointType ,
                "initialValue": sensor.initialValue ,
                "currentValue": currentVal,
                "note": sensor.note,
                "unit": sensor.unit,
                "minValue": sensor.minValue,
                "maxValue": sensor.maxValue,
                "readOnly": sensor.readOnly
            })
        }
        console.log("===== displaySensorList 完成，添加了", sensorListModel.count, "条记录 =====")
    }

    // 更新线圈值
    function updateCoilValue(address, value) {
        console.log("收到线圈变化信号 - 地址:", address, "值:", value)
        updateSensorValue(address, "线圈", value ? "1" : "0")
    }

    // 更新离散输入值
    function updateDiscreteInputValue(address, value) {
        console.log("收到离散输入变化信号 - 地址:", address, "值:", value)
        updateSensorValue(address, "离散输入", value ? "1" : "0")
    }

    // 更新保持寄存器值
    function updateHoldingRegisterValue(address, value) {
        console.log("收到保持寄存器变化信号 - 地址:", address, "值:", value)
        updateSensorValue(address, "保持寄存器", String(value))
    }

    // 更新输入寄存器值
    function updateInputRegisterValue(address, value) {
        console.log("收到输入寄存器变化信号 - 地址:", address, "值:", value)
        updateSensorValue(address, "输入寄存器", String(value))
    }

    // 通用更新函数
    function updateSensorValue(address, pointType, value) {
        console.log("开始更新传感器值 - 地址:", address, "类型:", pointType, "值:", value)
        var found = false
        for (var i = 0; i < sensorListModel.count; i++) {
            var item = sensorListModel.get(i)
            if (item.address === address && item.pointType === pointType) {
                console.log("找到匹配项，索引:", i, "更新前值:", item.currentValue)
                sensorListModel.setProperty(i, "currentValue", value)
                console.log("更新后值:", sensorListModel.get(i).currentValue)
                found = true
                break
            }
        }
        if (!found) {
            console.log("警告：未找到匹配的传感器 - 地址:", address, "类型:", pointType)
        }
    }

    // 应用传感器配置到服务器
    function applySensorsToServer() {
        if (!sensorManager || !modbusServer) {
            addLog("错误: 传感器管理器或服务器未初始化")
            return
        }

        addLog("正在应用传感器配置到服务器...")
        if (sensorManager.applyToServer(modbusServer)) {
            addLog("成功应用 " + sensorManager.sensorCount + " 个传感器配置")
        } else {
            addLog("应用失败: " + sensorManager.getLastError())
        }
    }

    // 添加日志（自动滚动到底部）
    function addLog(message) {
        var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss")
        logDisplay.append("[" + timestamp + "] " + message)
        // 自动滚动到底部
        logDisplay.cursorPosition = logDisplay.length
    }

    // 获取功能码名称（新增）
    function getFunctionCodeName(fc) {
        var fcNames = {
            1: "读线圈",
            2: "读离散输入",
            3: "读保持寄存器",
            4: "读输入寄存器",
            5: "写单个线圈",
            6: "写单个寄存器",
            15: "写多个线圈",
            16: "写多个寄存器",
            20: "读文件记录",
            21: "写文件记录",
            203: "读文件(自定义)",
            204: "写文件(自定义)"
        }
        return fcNames[fc] || "未知功能"
    }

    Component.onCompleted: {
        // modbusServer 和 sensorManager 已经通过 setContextProperty 注入
        // 直接使用即可，但需要验证它们是否存在

        addLog("QML 界面已加载")

        if (modbusServer) {
            addLog("服务器已初始化")

            // 连接信号
            modbusServer.requestReceived.connect(function(fc) {
                addLog("收到请求，功能码: " + fc)
            })

            modbusServer.errorOccurred.connect(function(error) {
                addLog("错误: " + error)
            })

            // 连接数据变化信号
            if (modbusServer.dataStore) {
                console.log("连接数据变化信号...")
                modbusServer.dataStore.coilChanged.connect(updateCoilValue)
                modbusServer.dataStore.discreteInputChanged.connect(updateDiscreteInputValue)
                modbusServer.dataStore.holdingRegisterChanged.connect(updateHoldingRegisterValue)
                modbusServer.dataStore.inputRegisterChanged.connect(updateInputRegisterValue)
                addLog("数据变化信号已连接")
            }
        } else {
            addLog("警告: 无法获取 ModbusServer 对象")
        }

        if (sensorManager) {
            addLog("传感器管理器已初始化")

            // 连接传感器管理器信号
            sensorManager.sensorsLoaded.connect(function(count) {
                addLog("已加载 " + count + " 个传感器配置")
                displaySensorList()
            })

            sensorManager.errorOccurred.connect(function(error) {
                addLog("传感器错误: " + error)
            })
        } else {
            addLog("警告: 无法获取 SensorManager 对象")
        }
    }
}
