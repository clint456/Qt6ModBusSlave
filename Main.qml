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
            font.bold: true

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
            font.bold: true

            RowLayout {
                anchors.fill: parent
                spacing: 8
                Label {
                    text: "运行状态:"
                    font.bold: true
                }
                Label {
                    id: runningLabel
                    text: modbusServer ? (modbusServer.running ? "● 运行中" : "○ 已停止") : "○ 未知"
                    color: modbusServer && modbusServer.running ? "#27ae60" : "#e74c3c"
                    font.pixelSize: 14
                    font.bold: true
                }

                Label {
                    text: "模式:"
                    font.bold: true
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
                    font.bold: true
                }
                Label {
                    id: requestCountLabel
                    text: modbusServer ? modbusServer.requestCount.toString() : "0"
                    color: "#2980b9"
                    font.pixelSize: 14
                    font.bold: true
                }

                Label {
                    text: "最后功能码:"
                    font.bold: true
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
                    font.bold: true
                }

                Label {
                    text: "状态消息:"
                    font.bold: true
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
            font.bold: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

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
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                // 标准文件记录
                                GroupBox {
                                    title: "标准文件记录 (FC 20/21)"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 160

                                    GridLayout {
                                        anchors.fill: parent
                                        columns: 2
                                        rowSpacing: 8
                                        columnSpacing: 10

                                        Label { text: "文件号:" }
                                        SpinBox {
                                            id: fileNumberSpinBox
                                            from: 0
                                            to: 65535
                                            value: 1
                                            editable: true
                                            Layout.fillWidth: true
                                        }

                                        Label { text: "记录号:" }
                                        SpinBox {
                                            id: recordNumberSpinBox
                                            from: 0
                                            to: 9999
                                            value: 0
                                            editable: true
                                            Layout.fillWidth: true
                                        }

                                        Label { text: "记录数:" }
                                        SpinBox {
                                            id: recordCountSpinBox
                                            from: 1
                                            to: 126
                                            value: 10
                                            editable: true
                                            Layout.fillWidth: true

                                            ToolTip.visible: hovered
                                            ToolTip.text: "Modbus标准限制：单次最多读取126个记录（252字节）"
                                        }

                                        Button {
                                            text: "读取文件"
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            onClicked: readFileRecord()
                                        }

                                        Button {
                                            text: "写入测试数据"
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            onClicked: writeFileRecord()
                                        }

                                        Button {
                                            text: "🔍 查询文件内容"
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            onClicked: queryFileContent()

                                            background: Rectangle {
                                                color: parent.hovered ? "#3498db" : "#2980b9"
                                                radius: 3
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: "white"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                font.pixelSize: 12
                                                font.bold: true
                                            }
                                        }
                                    }
                                }

                                // 地址文件
                                GroupBox {
                                    title: "地址文件 (FC 203/204)"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 160

                                    GridLayout {
                                        anchors.fill: parent
                                        columns: 2
                                        rowSpacing: 8
                                        columnSpacing: 10

                                        Label { text: "起始地址:" }
                                        SpinBox {
                                            id: fileAddressSpinBox
                                            from: 0
                                            to: 65535
                                            value: 1000
                                            editable: true
                                            Layout.fillWidth: true
                                        }

                                        Label { text: "寄存器数:" }
                                        SpinBox {
                                            id: fileRegisterCountSpinBox
                                            from: 1
                                            to: 125
                                            value: 10
                                            editable: true
                                            Layout.fillWidth: true
                                        }

                                        Item { Layout.fillHeight: true; Layout.columnSpan: 2 }

                                        Button {
                                            text: "读取地址文件"
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            onClicked: readAddressFile()
                                        }

                                        Button {
                                            text: "写入测试数据"
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            onClicked: writeAddressFile()
                                        }
                                    }
                                }

                                // 文件信息
                                GroupBox {
                                    title: "文件信息"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 160

                                    ScrollView {
                                        anchors.fill: parent
                                        clip: true

                                        Label {
                                            text: "已创建文件:\n• 文件 1: 温度数据\n  (256 记录)\n• 文件 2: 状态数据\n  (128 记录)\n\n地址存储:\n• 1000-1199\n  (200 寄存器)"
                                            wrapMode: Text.WordWrap
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                            }

                            // 简化的提示信息区域
                            Label {
                                Layout.fillWidth: true
                                Layout.topMargin: 5
                                text: "💡 提示：所有操作结果将显示在下方的操作日志中"
                                font.pixelSize: 11
                                color: "#7f8c8d"
                                wrapMode: Text.WordWrap
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
                                Layout.preferredHeight: 80
                                color: "#ecf0f1"
                                radius: 5
                                border.color: "#bdc3c7"
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 15
                                    spacing: 15

                                    Label {
                                        text: "📊 传感器配置管理"
                                        font.pixelSize: 14
                                        font.bold: true
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

                                    Item { Layout.fillWidth: true }

                                    Label {
                                        text: sensorManager ? ("传感器数: " + sensorManager.sensorCount) : "传感器数: 0"
                                        font.pixelSize: 13
                                        color: "#2980b9"
                                        font.bold: true
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
                                    spacing: 5

                                    // 表格标题
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 35
                                        color: "#34495e"
                                        radius: 3

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 5
                                            spacing: 0

                                            Label {
                                                text: "序号"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 60
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                            Label {
                                                text: "点名称"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 200
                                            }
                                            Label {
                                                text: "点类型"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 120
                                            }
                                            Label {
                                                text: "初始值"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 100
                                            }
                                            Label {
                                                text: "单位"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 80
                                            }
                                            Label {
                                                text: "起始值"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 100
                                            }
                                            Label {
                                                text: "最大值"
                                                color: "white"
                                                font.bold: true
                                                Layout.preferredWidth: 100
                                            }
                                            Label {
                                                text: "备注"
                                                color: "white"
                                                font.bold: true
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }

                                    // 数据显示区域
                                    ScrollView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        clip: true

                                        TextArea {
                                            id: sensorListDisplay
                                            readOnly: true
                                            wrapMode: TextEdit.NoWrap
                                            font.family: "Consolas, Monaco, monospace"
                                            font.pixelSize: 11
                                            text: "点击'导入 Excel/CSV'加载传感器配置...\n\n支持格式：\n• CSV 文件 (*.csv)\n• Tab 分隔文件 (*.txt)\n\n文件格式示例：\n序号    点名称    点类型    初始值    单位    起始值    最大值    备注\n0       流行达标  线圈      0         -       -         -         开关\n1       手自动模式 线圈     0         -       -         -         模式"
                                            background: Rectangle {
                                                color: "#fafafa"
                                                border.color: "#dcdcdc"
                                                border.width: 1
                                                radius: 3
                                            }
                                        }
                                    }
                                }
                            }

                            // 使用说明
                            Label {
                                Layout.fillWidth: true
                                text: "💡 提示：导入后点击'应用到服务器'将配置写入 Modbus 数据存储"
                                font.pixelSize: 11
                                color: "#7f8c8d"
                                wrapMode: Text.WordWrap
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
            font.bold: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: "📋 最近操作记录"
                        font.pixelSize: 12
                        color: "#7f8c8d"
                    }

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
    // 读取文件记录（模拟功能码 20）
    function readFileRecord() {
        var startRecord = recordNumberSpinBox.value
        var count = recordCountSpinBox.value
        var fileNum = fileNumberSpinBox.value

        addLog("===== 读取文件记录 (FC 20) =====")
        addLog("📁 文件号: " + fileNum)
        addLog("📋 起始记录: " + startRecord)
        addLog("📋 记录数: " + count + " (最多126个)")
        addLog("💾 数据量: " + (count * 2) + " 字节")
        addLog("ℹ️ 说明: 通过Modbus客户端发送请求查看实际响应")
        addLog("————————————————————")
    }

    // 写入文件记录（模拟功能码 21）
    function writeFileRecord() {
        var startRecord = recordNumberSpinBox.value
        var count = recordCountSpinBox.value
        var fileNum = fileNumberSpinBox.value

        addLog("===== 写入文件记录 (FC 21) =====")
        addLog("📁 文件号: " + fileNum)
        addLog("📋 起始记录: " + startRecord)
        addLog("📋 记录数: " + count)
        addLog("💾 数据量: " + (count * 2) + " 字节")
        addLog("✅ 测试数据: 0x1000, 0x1100, 0x1200...")
        addLog("ℹ️ 说明: 通过Modbus客户端发送写入请求")
        addLog("————————————————————")
    }

    // 查询文件内容
    function queryFileContent() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化")
            return
        }

        var fileNum = fileNumberSpinBox.value
        addLog("🔍 正在查询文件 " + fileNum + " 的内容...")
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
    function readAddressFile() {
        var startAddr = fileAddressSpinBox.value
        var count = fileRegisterCountSpinBox.value

        addLog("===== 读取地址文件 (FC 203) =====")
        addLog("📍 起始地址: " + startAddr)
        addLog("📋 寄存器数: " + count)
        addLog("💾 数据量: " + (count * 2) + " 字节")
        addLog("ℹ️ 说明: 类似功能码 3，通过地址直接访问")
        addLog("————————————————————")
    }

    // 写入地址文件（模拟功能码 204）
    function writeAddressFile() {
        var startAddr = fileAddressSpinBox.value
        var count = fileRegisterCountSpinBox.value

        addLog("===== 写入地址文件 (FC 204) =====")
        addLog("📍 起始地址: " + startAddr)
        addLog("📋 寄存器数: " + count)
        addLog("💾 数据量: " + (count * 2) + " 字节")
        addLog("✅ 测试数据: 0xAAAA, 0xBBBB, 0xCCCC...")
        addLog("ℹ️ 说明: 类似功能码 16，通过地址直接写入")
        addLog("————————————————————")
    }

    // 显示传感器列表
    function displaySensorList() {
        if (!sensorManager) {
            sensorListDisplay.text = "传感器管理器未初始化"
            return
        }

        var sensors = sensorManager.getSensorList()
        if (!sensors || sensors.length === 0) {
            sensorListDisplay.text = "没有传感器数据"
            return
        }

        var result = ""
        for (var i = 0; i < sensors.length; i++) {
            var sensor = sensors[i]
            var line = ""

            // 格式化每个字段，使用固定宽度
            line += String(sensor.index).padEnd(8, ' ')
            line += String(sensor.pointName).padEnd(25, ' ')
            line += String(sensor.pointType).padEnd(15, ' ')
            line += String(sensor.initialValue).padEnd(12, ' ')
            line += String(sensor.unit).padEnd(10, ' ')
            line += String(sensor.minValue).padEnd(12, ' ')
            line += String(sensor.maxValue).padEnd(12, ' ')
            line += String(sensor.note)

            result += line + "\n"
        }

        sensorListDisplay.text = result
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
