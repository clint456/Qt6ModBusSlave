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
    minimumWidth: 860
    minimumHeight: 560
    color: "#F3F6FA"

    property color pageBgTop: "#F6FAFF"
    property color pageBgBottom: "#EAF1F8"
    property color panelBg: "#FFFFFF"
    property color panelBorder: "#D5E1EE"
    property color panelTitle: "#1D334A"
    property color accentColor: "#0E7490"
    property color accentSoft: "#DDF2F6"
    property color bodyText: "#27374A"

    font.family: "Microsoft YaHei UI"

    background: Rectangle {
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.pageBgTop
            }
            GradientStop {
                position: 1.0
                color: root.pageBgBottom
            }
        }

        Rectangle {
            x: -140
            y: -100
            width: 420
            height: 420
            radius: 210
            color: "#B7E4ED"
            opacity: 0.16
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: -110
            anchors.topMargin: 30
            width: 360
            height: 360
            radius: 180
            color: "#C8DBF6"
            opacity: 0.18
        }
    }

    // modbusServer 和 sensorManager 通过 C++ setContextProperty 注入
    // 不需要在这里声明

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        leftPadding: 18
        rightPadding: 18
        topPadding: 18
        bottomPadding: 18
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentWidth: availableWidth

        ColumnLayout {
            width: pageScroll.availableWidth
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                radius: 14
                border.width: 1
                border.color: root.panelBorder
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "#FFFFFF"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#F6FBFF"
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14

                    Rectangle {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        radius: 10
                        color: root.accentSoft
                        border.color: "#B9DFE8"
                        border.width: 1

                        Label {
                            anchors.centerIn: parent
                            text: "M"
                            font.bold: true
                            font.pixelSize: 20
                            color: root.accentColor
                        }
                    }

                    ColumnLayout {
                        spacing: 0
                        Label {
                            text: "Modbus 从站模拟控制台"
                            font.pixelSize: 18
                            font.bold: true
                            color: root.panelTitle
                        }
                        Label {
                            text: "TCP / RTU 实时调试、文件寄存器与传感器配置一体化"
                            font.pixelSize: 12
                            color: "#5E7286"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: Math.max(130, Math.min(220, runningBadgeLabel.implicitWidth + 30))
                        radius: 15
                        color: root.accentSoft
                        border.width: 1
                        border.color: "#B9DFE8"

                        Label {
                            id: runningBadgeLabel
                            anchors.centerIn: parent
                            text: modbusServer && modbusServer.running ? "服务运行中" : "服务未运行"
                            color: root.accentColor
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }
                }
            }

            // 服务器控制区域
            GroupBox {
                id: serverControlBox
                title: "服务器控制"
                Layout.fillWidth: true
                Layout.preferredHeight: root.width < 1220 ? 250 : 170
                Layout.minimumHeight: root.width < 1220 ? 240 : 160
                font.bold: false
                padding: 12
                topPadding: 30

                label: Label {
                    text: parent.title
                    color: root.panelTitle
                    font.pixelSize: 14
                    font.bold: true
                    leftPadding: 10
                }

                background: Rectangle {
                    radius: 12
                    color: root.panelBg
                    border.color: root.panelBorder
                    border.width: 1
                }

                GridLayout {
                    id: serverControlGrid
                    anchors.fill: parent
                    columns: root.width >= 1380 ? 3 : 2
                    rowSpacing: 12
                    columnSpacing: 12
                    // TCP 控制
                    GroupBox {
                        title: "TCP 模式"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        padding: 10
                        topPadding: 26

                        label: Label {
                            text: parent.title
                            color: "#35506A"
                            font.pixelSize: 13
                            font.bold: true
                            leftPadding: 8
                        }

                        background: Rectangle {
                            radius: 10
                            color: "#FAFDFF"
                            border.color: "#DDE8F3"
                            border.width: 1
                        }

                        GridLayout {
                            id: tcpControlGrid
                            anchors.fill: parent
                            columns: width >= 320 ? 3 : 2
                            rowSpacing: 8
                            columnSpacing: 10

                            Label {
                                text: "端口:"
                            }
                            TextField {
                                id: tcpPortField
                                text: "502"
                                placeholderText: "502"
                                Layout.fillWidth: true
                            }

                            Button {
                                id: startTcpButton
                                text: "启动 TCP"
                                Layout.fillWidth: true
                                Layout.columnSpan: tcpControlGrid.columns === 2 ? 2 : 1
                                onClicked: {
                                    if (modbusServer) {
                                        var port = parseInt(tcpPortField.text);
                                        addLog("尝试启动 TCP 服务器，端口: " + port);
                                        if (modbusServer.startTcp(port)) {
                                            statusLabel.text = "TCP 服务器已启动";
                                            addLog("TCP 服务器启动成功");
                                        } else {
                                            addLog("TCP 服务器启动失败");
                                        }
                                    } else {
                                        addLog("错误: ModbusServer 对象未初始化");
                                    }
                                }
                            }
                        }
                    }

                    // RTU 控制
                    GroupBox {
                        title: "RTU 模式"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        padding: 10
                        topPadding: 26

                        label: Label {
                            text: parent.title
                            color: "#35506A"
                            font.pixelSize: 13
                            font.bold: true
                            leftPadding: 8
                        }

                        background: Rectangle {
                            radius: 10
                            color: "#FAFDFF"
                            border.color: "#DDE8F3"
                            border.width: 1
                        }

                        GridLayout {
                            id: rtuControlGrid
                            anchors.fill: parent
                            columns: width >= 430 ? 5 : (width >= 320 ? 3 : 2)
                            rowSpacing: 8
                            columnSpacing: 10

                            Label {
                                text: "串口:"
                            }
                            TextField {
                                id: rtuPortField
                                text: "COM1"
                                placeholderText: "COM1"
                                Layout.fillWidth: true
                            }

                            Label {
                                text: "波特率:"
                            }
                            ComboBox {
                                id: baudRateCombo
                                model: ["9600", "19200", "38400", "57600", "115200"]
                                currentIndex: 0
                                Layout.fillWidth: true
                            }

                            Button {
                                id: startRtuButton
                                text: "启动 RTU"
                                Layout.fillWidth: true
                                Layout.columnSpan: rtuControlGrid.columns <= 3 ? rtuControlGrid.columns : 1
                                enabled: modbusServer && !modbusServer.running
                                onClicked: {
                                    if (modbusServer) {
                                        var baudRate = parseInt(baudRateCombo.currentText);
                                        addLog("尝试启动 RTU 服务器，串口: " + rtuPortField.text + ", 波特率:  " + baudRate);
                                        if (modbusServer.startRtu(rtuPortField.text, baudRate)) {
                                            statusLabel.text = "RTU 服务器已启动";
                                            addLog("RTU 服务器启动成功");
                                        } else {
                                            addLog("RTU 服务器启动失败");
                                        }
                                    } else {
                                        addLog("错误: ModbusServer 对象未初始化");
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
                        Layout.columnSpan: serverControlGrid.columns === 3 ? 1 : 2
                        padding: 10
                        topPadding: 26

                        label: Label {
                            text: parent.title
                            color: "#35506A"
                            font.pixelSize: 13
                            font.bold: true
                            leftPadding: 8
                        }

                        background: Rectangle {
                            radius: 10
                            color: "#FAFDFF"
                            border.color: "#DDE8F3"
                            border.width: 1
                        }

                        GridLayout {
                            anchors.fill: parent
                            columns: 2
                            rowSpacing: 6
                            columnSpacing: 8

                            Button {
                                text: "停止服务器"
                                Layout.fillWidth: true
                                enabled: modbusServer && modbusServer.running
                                onClicked: {
                                    if (modbusServer) {
                                        modbusServer.stop();
                                        statusLabel.text = "服务器已停止";
                                    }
                                }
                            }

                            Button {
                                text: "初始化数据"
                                Layout.fillWidth: true
                                onClicked: {
                                    if (modbusServer) {
                                        modbusServer.initializeData();
                                        statusLabel.text = "数据已初始化";
                                    }
                                }
                            }

                            Button {
                                text: "内存报告"
                                Layout.fillWidth: true
                                ToolTip.visible: hovered
                                ToolTip.text: "查看当前内存使用情况"
                                onClicked: {
                                    if (modbusServer) {
                                        var report = modbusServer.getMemoryUsageReport();
                                        addLog("\n" + report + "\n");
                                    }
                                }
                            }

                            Button {
                                text: "清理内存"
                                Layout.fillWidth: true
                                ToolTip.visible: hovered
                                ToolTip.text: "清空所有寄存器和文件数据（谨慎使用）"
                                onClicked: {
                                    if (modbusServer) {
                                        modbusServer.clearAllData();
                                        addLog("已清理所有数据");
                                        displaySensorList();  // 刷新显示
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 状态显示区域
            GroupBox {
                id: serverStatusBox
                title: "服务器状态"
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(58, statusFlow.implicitHeight + 36)
                Layout.minimumHeight: 56
                font.bold: false
                padding: 10
                topPadding: 28

                label: Label {
                    text: parent.title
                    color: root.panelTitle
                    font.pixelSize: 14
                    font.bold: true
                    leftPadding: 10
                }

                background: Rectangle {
                    radius: 12
                    color: root.panelBg
                    border.color: root.panelBorder
                    border.width: 1
                }

                Flow {
                    id: statusFlow
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
                            if (!modbusServer)
                                return "未知";
                            return modbusServer.mode === 0 ? "TCP" : "RTU";
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
                            if (!modbusServer || modbusServer.lastFunctionCode === 0)
                                return "无";
                            var fc = modbusServer.lastFunctionCode;
                            var fcName = getFunctionCodeName(fc);
                            return fc + " (0x" + fc.toString(16).toUpperCase() + ") - " + fcName;
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
                        width: Math.max(180, serverStatusBox.width * 0.35)
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
                padding: 10
                topPadding: 30

                label: Label {
                    text: parent.title
                    color: root.panelTitle
                    font.pixelSize: 14
                    font.bold: true
                    leftPadding: 10
                }

                background: Rectangle {
                    radius: 12
                    color: root.panelBg
                    border.color: root.panelBorder
                    border.width: 1
                }

                ColumnLayout {
                    anchors.fill: parent
                    // spacing: 8

                    TabBar {
                        id: dataTabBar
                        Layout.fillWidth: true
                        spacing: 8
                        background: Rectangle {
                            radius: 10
                            color: "#F4F8FD"
                            border.color: "#DCE7F3"
                            border.width: 1
                        }

                        TabButton {
                            text: "文件寄存器"
                            font.pixelSize: 13
                            font.bold: checked
                            background: Rectangle {
                                radius: 8
                                color: parent.checked ? "#DDF2F6" : "transparent"
                                border.width: parent.checked ? 1 : 0
                                border.color: "#9DD2DE"
                            }
                        }
                        TabButton {
                            text: "传感器配置"
                            font.pixelSize: 13
                            font.bold: checked
                            background: Rectangle {
                                radius: 8
                                color: parent.checked ? "#DDF2F6" : "transparent"
                                border.width: parent.checked ? 1 : 0
                                border.color: "#9DD2DE"
                            }
                        }
                    }

                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: false
                        Layout.preferredHeight: dataTabBar.currentIndex === 0 ? fileTransferTabItem.implicitHeight : Math.max(420, root.height * 0.45)
                        currentIndex: dataTabBar.currentIndex
                        // Tab 1: 文件寄存器
                        Item {
                            id: fileTransferTabItem
                            implicitHeight: fileTransferColumn.implicitHeight

                            ColumnLayout {
                                id: fileTransferColumn
                                width: parent.width
                                spacing: 8

                                // 使用 RowLayout 实现三栏
                                GridLayout {
                                    id: fileTransferGrid
                                    Layout.fillWidth: true
                                    Layout.fillHeight: false
                                    columns: root.width >= 1380 ? 2 : 1
                                    rowSpacing: 10
                                    columnSpacing: 10

                                    // 标准文件记录
                                    GroupBox {
                                        title: "标准文件记录 (FC 20/21)"
                                        Layout.fillWidth: true
                                        Layout.fillHeight: false
                                        Layout.minimumHeight: 230
                                        font.bold: false
                                        padding: 10
                                        topPadding: 28

                                        label: Label {
                                            text: parent.title
                                            color: "#35506A"
                                            font.pixelSize: 13
                                            font.bold: true
                                            leftPadding: 8
                                        }

                                        background: Rectangle {
                                            radius: 10
                                            color: "#FAFDFF"
                                            border.color: "#DDE8F3"
                                            border.width: 1
                                        }

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

                                                Label {
                                                    text: "文件号:"
                                                }
                                                SpinBox {
                                                    id: fileNumberSpinBox
                                                    from: 0
                                                    to: 65535
                                                    value: 1
                                                    editable: true
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "记录号:"
                                                }
                                                SpinBox {
                                                    id: recordNumberSpinBox
                                                    from: 0
                                                    to: 9999
                                                    value: 0
                                                    editable: true
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "记录数:"
                                                }
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
                                            }

                                            // 按钮区域（靠底）
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignBottom

                                                Button {
                                                    text: "查询文件内容"
                                                    Layout.fillWidth: true
                                                    onClicked: queryFileContent()
                                                }
                                                Button {
                                                    text: "上传文件"
                                                    Layout.fillWidth: true
                                                    onClicked: uploadFile()
                                                }
                                                Button {
                                                    text: "下载文件"
                                                    Layout.fillWidth: true
                                                    onClicked: downloadFile()
                                                }
                                            }
                                        }
                                    }

                                    // 保持寄存器查询
                                    GroupBox {
                                        title: "保持寄存器(文件传输) (FC 203/204)"
                                        Layout.fillWidth: true
                                        Layout.fillHeight: false
                                        Layout.minimumHeight: 230
                                        font.bold: false
                                        padding: 10
                                        topPadding: 28

                                        label: Label {
                                            text: parent.title
                                            color: "#35506A"
                                            font.pixelSize: 13
                                            font.bold: true
                                            leftPadding: 8
                                        }

                                        background: Rectangle {
                                            radius: 10
                                            color: "#FAFDFF"
                                            border.color: "#DDE8F3"
                                            border.width: 1
                                        }

                                        ColumnLayout {
                                            anchors.fill: parent
                                            spacing: 6

                                            GridLayout {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                columns: 2
                                                rowSpacing: 6
                                                columnSpacing: 8

                                                Label {
                                                    text: "起始地址:"
                                                }
                                                SpinBox {
                                                    id: fileAddressSpinBox
                                                    from: 0
                                                    to: 65535
                                                    value: 1000
                                                    editable: true
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "寄存器数:"
                                                }
                                                SpinBox {
                                                    id: fileRegisterCountSpinBox
                                                    from: 1
                                                    to: 125
                                                    value: 20
                                                    editable: true
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            // 按钮靠底
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignBottom

                                                Button {
                                                    text: "查询保持寄存器"
                                                    Layout.fillWidth: true
                                                    onClicked: queryAddressFileContent()
                                                }
                                                Button {
                                                    text: "上传保持寄存器"
                                                    Layout.fillWidth: true
                                                    onClicked: uploadHoldingRegisters()
                                                }
                                                Button {
                                                    text: "下载保持寄存器"
                                                    Layout.fillWidth: true
                                                    onClicked: downloadHoldingRegisters()
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
                                    color: "#F5F9FD"
                                    radius: 5
                                    border.color: "#D6E3EF"
                                    border.width: 1

                                    Flow {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 8

                                        Label {
                                            text: "传感器配置管理"
                                            font.bold: false
                                        }

                                        Rectangle {
                                            width: 1
                                            height: 20
                                            color: "#bdc3c7"
                                        }

                                        Button {
                                            text: "导入 Excel/CSV"
                                            icon.name: "document-import"
                                            onClicked: importFileDialog.open()
                                        }

                                        Button {
                                            text: "导出配置"
                                            icon.name: "document-export"
                                            onClicked: exportFileDialog.open()
                                        }

                                        Button {
                                            text: "应用到服务器"
                                            icon.name: "application-x-executable"
                                            enabled: sensorManager && sensorManager.sensorCount > 0
                                            onClicked: applySensorsToServer()
                                        }

                                        Button {
                                            text: "测试更新"
                                            onClicked: {
                                                addLog("===== 测试数据更新 =====");
                                                if (modbusServer && modbusServer.dataStore) {
                                                    // 测试写入线圈地址0
                                                    addLog("测试写入线圈 地址0 值false");
                                                    modbusServer.dataStore.writeCoil(0, false);

                                                    // 测试写入保持寄存器地址0
                                                    addLog("测试写入保持寄存器 地址0 值999");
                                                    modbusServer.dataStore.writeHoldingRegister(0, 111);
                                                } else {
                                                    addLog("错误：无法访问数据存储");
                                                }
                                            }
                                        }

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

                                                // 使用比例分配宽度，而不是固定宽度
                                                Repeater {
                                                    model: [
                                                        {
                                                            text: "地址",
                                                            width: 0.05
                                                        },
                                                        {
                                                            text: "点位名称",
                                                            width: 0.05
                                                        },
                                                        {
                                                            text: "寄存器类型",
                                                            width: 0.10
                                                        },
                                                        {
                                                            text: "值类型",
                                                            width: 0.08,
                                                            color: "#FF6B35"
                                                        },
                                                        {
                                                            text: "初始值",
                                                            width: 0.07
                                                        },
                                                        {
                                                            text: "当前值",
                                                            width: 0.07,
                                                            color: "#2196F3"
                                                        },
                                                        {
                                                            text: "描述",
                                                            width: 0.10
                                                        },
                                                        {
                                                            text: "单位",
                                                            width: 0.08
                                                        },
                                                        {
                                                            text: "最小值",
                                                            width: 0.12
                                                        },
                                                        {
                                                            text: "最大值",
                                                            width: 0.12
                                                        },
                                                        {
                                                            text: "只读",
                                                            width: 0.06
                                                        },
                                                        {
                                                            text: "占用寄存器数",
                                                            width: 0.10
                                                        }
                                                    ]

                                                    Rectangle {
                                                        width: parent.width * modelData.width
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#c0c0c0"
                                                        border.width: 1

                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: modelData.text
                                                            font.bold: true
                                                            font.pixelSize: 12
                                                            color: modelData.color || "#000000"
                                                        }
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

                                                    // 地址 - 5%
                                                    Rectangle {
                                                        width: parent.width * 0.05
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.address !== undefined ? model.address : ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 点位名称 - 5%
                                                    Rectangle {
                                                        width: parent.width * 0.05
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.pointName || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 寄存器类型 - 10%
                                                    Rectangle {
                                                        width: parent.width * 0.10
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.pointType || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 值类型 - 8%
                                                    Rectangle {
                                                        width: parent.width * 0.08
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.valueType || "UINT16"
                                                            font.pixelSize: 11
                                                            font.bold: true
                                                            color: "#FF6B35"
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 初始值 - 7%
                                                    Rectangle {
                                                        width: parent.width * 0.07
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.initialValue || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 当前值 - 7%
                                                    Rectangle {
                                                        width: parent.width * 0.07
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
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 描述 - 10%
                                                    Rectangle {
                                                        width: parent.width * 0.10
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.note || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 单位 - 8%
                                                    Rectangle {
                                                        width: parent.width * 0.08
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.unit || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 最小值 - 12%
                                                    Rectangle {
                                                        width: parent.width * 0.12
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.minValue || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 最大值 - 12%
                                                    Rectangle {
                                                        width: parent.width * 0.12
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.maxValue || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }

                                                    // 只读 - 6%
                                                    Rectangle {
                                                        width: parent.width * 0.06
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.readOnly ? "是" : "否"
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
                                                        }
                                                    }
                                                    // 占用寄存器数 - 10%
                                                    Rectangle {
                                                        width: parent.width * 0.10
                                                        height: parent.height
                                                        color: "transparent"
                                                        border.color: "#e0e0e0"
                                                        border.width: 1
                                                        Label {
                                                            anchors.centerIn: parent
                                                            text: model.registerCount || ""
                                                            font.pixelSize: 11
                                                            elide: Text.ElideRight
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
                Layout.preferredHeight: Math.max(220, Math.min(340, root.height * 0.30))
                Layout.minimumHeight: 200
                font.bold: false
                padding: 10
                topPadding: 30

                label: Label {
                    text: parent.title
                    color: root.panelTitle
                    font.pixelSize: 14
                    font.bold: true
                    leftPadding: 10
                }

                background: Rectangle {
                    radius: 12
                    color: root.panelBg
                    border.color: root.panelBorder
                    border.width: 1
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        CheckBox {
                            id: packetLogCheckBox
                            text: "记录收发包"
                            checked: enablePacketLog
                            font.pixelSize: 11
                            onCheckedChanged: enablePacketLog = checked
                            ToolTip.visible: hovered
                            ToolTip.text: "启用后会记录每个Modbus请求/响应的详细报文（高频操作，建议关闭以节省内存）"
                        }

                        CheckBox {
                            id: requestLogCheckBox
                            text: "记录请求"
                            checked: enableRequestLog
                            font.pixelSize: 11
                            onCheckedChanged: enableRequestLog = checked
                            ToolTip.visible: hovered
                            ToolTip.text: "记录每个Modbus请求的功能码"
                        }

                        Label {
                            text: "最大行数:"
                            font.pixelSize: 11
                        }
                        SpinBox {
                            id: maxLogLinesSpinBox
                            from: 100
                            to: 5000
                            value: maxLogLines
                            stepSize: 100
                            editable: true
                            Layout.preferredWidth: root.width < 980 ? 80 : 100
                            font.pixelSize: 11
                            onValueChanged: maxLogLines = value
                            ToolTip.visible: hovered
                            ToolTip.text: "日志超过此行数时自动清理旧日志"
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "清空日志"
                            Layout.preferredWidth: root.width < 980 ? 64 : 80
                            Layout.preferredHeight: 25
                            font.pixelSize: 11
                            onClicked: {
                                logDisplay.clear();
                                logLineCount = 0;
                            }
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
                                radius: 8
                                gradient: Gradient {
                                    GradientStop {
                                        position: 0.0
                                        color: "#1C2A39"
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: "#15202C"
                                    }
                                }
                                border.color: "#32485D"
                                border.width: 1
                            }
                            color: "#ecf0f1"
                        }
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
                var filePath = currentFile.toString();
                addLog("正在导入文件: " + filePath);
                if (sensorManager.importFromExcel(filePath)) {
                    addLog("成功导入 " + sensorManager.sensorCount + " 个传感器配置");
                    displaySensorList();
                } else {
                    addLog("导入失败");
                }
            } else {
                addLog("错误: 传感器管理器未初始化");
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
                var filePath = currentFile.toString();
                addLog("正在导出文件: " + filePath);
                if (sensorManager.exportToExcel(filePath)) {
                    addLog("成功导出 " + sensorManager.sensorCount + " 个传感器配置");
                } else {
                    addLog("导出失败");
                }
            } else {
                addLog("错误: 传感器管理器未初始化");
            }
        }
    }

    // 文件寄存器上传/下载使用的文件号
    property int pendingFileNumber: 0
    property int pendingHoldingStartAddress: 0
    property int pendingHoldingCount: 0

    // 文件寄存器上传对话框
    FileDialog {
        id: uploadFileDialog
        title: "选择要上传的二进制文件"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Binary files (*.bin)", "All files (*)"]
        onAccepted: {
            var path = currentFile.toString();
            addLog("正在读取文件并上传至文件号: " + pendingFileNumber);
            var success = modbusServer.importFileFromLocal(pendingFileNumber, path);

            if (success) {
                addLog("文件上传成功，记录已存入 FileStore");
            } else {
                addLog("错误: 文件上传失败，请检查文件号或文件路径");
            }
        }
    }

    // 文件寄存器下载对话框
    FileDialog {
        id: downloadFileDialog
        title: "保存文件为 .bin"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Binary files (*.bin)", "All files (*)"]
        defaultSuffix: "bin"
        onAccepted: {
            var path = currentFile.toString();
            addLog("正在导出文件 " + pendingFileNumber + " 到: " + path);
            var success = modbusServer.exportFileToLocal(pendingFileNumber, path);

            if (success) {
                addLog("文件导出成功！");
            } else {
                addLog("错误: 导出失败，可能文件号内无数据");
            }
        }
    }

    // 保持寄存器上传对话框
    FileDialog {
        id: uploadHoldingDialog
        title: "选择保持寄存器导入文件"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Binary files (*.bin)", "All files (*)"]
        onAccepted: {
            var path = currentFile.toString();
            addLog("正在导入保持寄存器，起始地址: " + pendingHoldingStartAddress + "，最大数量: " + pendingHoldingCount);
            var success = modbusServer.importHoldingRegistersFromLocal(pendingHoldingStartAddress, pendingHoldingCount, path);

            if (success) {
                addLog("保持寄存器导入成功");
            } else {
                addLog("错误: 保持寄存器导入失败，请检查地址范围、数量或文件内容");
            }
        }
    }

    // 保持寄存器下载对话框
    FileDialog {
        id: downloadHoldingDialog
        title: "保存保持寄存器为 .bin"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Binary files (*.bin)", "All files (*)"]
        defaultSuffix: "bin"
        onAccepted: {
            var path = currentFile.toString();
            addLog("正在导出保持寄存器，起始地址: " + pendingHoldingStartAddress + "，数量: " + pendingHoldingCount + " 到: " + path);
            var success = modbusServer.exportHoldingRegistersToLocal(pendingHoldingStartAddress, pendingHoldingCount, path);

            if (success) {
                addLog("保持寄存器导出成功");
            } else {
                addLog("错误: 保持寄存器导出失败，请检查地址范围或路径权限");
            }
        }
    }

    // 上传文件：将本地 .bin 文件读取并填充到后端的 FileStore 中
    function uploadFile() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        pendingFileNumber = fileNumberSpinBox.value;
        uploadFileDialog.open();
    }

    // 下载文件：将后端的 FileStore 内容导出为本地 .bin 文件
    function downloadFile() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        pendingFileNumber = fileNumberSpinBox.value;
        downloadFileDialog.open();
    }

    function uploadHoldingRegisters() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        pendingHoldingStartAddress = fileAddressSpinBox.value;
        pendingHoldingCount = fileRegisterCountSpinBox.value;
        uploadHoldingDialog.open();
    }

    function downloadHoldingRegisters() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        pendingHoldingStartAddress = fileAddressSpinBox.value;
        pendingHoldingCount = fileRegisterCountSpinBox.value;
        downloadHoldingDialog.open();
    }
    // 刷新数据显示
    // 查询文件内容
    function queryFileContent() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        var fileNum = fileNumberSpinBox.value;
        addLog("正在查询文件 " + fileNum + " 的内容...");
        addLog("");

        // 调用C++后端查询
        var content = modbusServer.queryFileContent(fileNum, 50);

        // 将内容按行输出到日志
        var lines = content.split('\n');
        for (var i = 0; i < lines.length; i++) {
            if (lines[i].trim() !== "") {
                addLog(lines[i]);
            }
        }

        addLog("");
    }

    // 读取地址文件（模拟功能码 203）
    // 查询地址文件内容
    function queryAddressFileContent() {
        if (!modbusServer) {
            addLog("错误: 服务器未初始化");
            return;
        }

        var startAddr = fileAddressSpinBox.value;
        var count = fileRegisterCountSpinBox.value;
        addLog("正在查询保持寄存器...");
        addLog("");

        // 调用C++后端查询
        var content = modbusServer.queryAddressFile(startAddr, count);

        // 将内容按行输出到日志
        var lines = content.split('\n');
        for (var i = 0; i < lines.length; i++) {
            if (lines[i].trim() !== "") {
                addLog(lines[i]);
            }
        }

        addLog("");
    }

    // 显示传感器列表
    function displaySensorList() {
        console.log("===== displaySensorList 开始 =====");
        if (!sensorManager) {
            console.log("错误：sensorManager 不存在");
            sensorListModel.clear();
            return;
        }

        var sensors = sensorManager.getSensorList();
        console.log("获取到传感器列表，数量:", sensors ? sensors.length : 0);
        sensorListModel.clear();

        if (!sensors || sensors.length === 0) {
            console.log("传感器列表为空");
            return;
        }

        var startTime = new Date().getTime();

        for (var i = 0; i < sensors.length; i++) {
            var sensor = sensors[i];
            var currentVal = "";

            // 获取当前值
            if (modbusServer && modbusServer.dataStore) {
                try {
                    if (sensor.pointType === "线圈") {
                        currentVal = modbusServer.dataStore.readCoil(sensor.index) ? "1" : "0";
                    } else if (sensor.pointType === "离散输入") {
                        currentVal = modbusServer.dataStore.readDiscreteInput(sensor.index) ? "1" : "0";
                    } else if (sensor.pointType === "保持寄存器") {
                        currentVal = readRegisterValue(sensor.index, sensor.valueType, true);
                    } else if (sensor.pointType === "输入寄存器") {
                        currentVal = readRegisterValue(sensor.index, sensor.valueType, false);
                    }
                } catch (e) {
                    console.log("读取传感器值出错 - 地址:", sensor.index, "错误:", e);
                    currentVal = "错误";
                }
            }

            sensorListModel.append({
                "address": sensor.index,
                "pointName": sensor.pointName,
                "pointType": sensor.pointType,
                "valueType": sensor.valueType || "UINT16",
                "initialValue": sensor.initialValue,
                "currentValue": currentVal,
                "note": sensor.note,
                "unit": sensor.unit,
                "minValue": sensor.minValue,
                "maxValue": sensor.maxValue,
                "readOnly": sensor.readOnly,
                "registerCount": sensor.registerCount
            });
        }

        var endTime = new Date().getTime();
        console.log("===== displaySensorList 完成，添加了", sensorListModel.count, "条记录，耗时:", (endTime - startTime), "ms =====");
    }

    // 根据值类型读取寄存器值
    function readRegisterValue(address, valueType, isHolding) {
        if (!modbusServer || !modbusServer.dataStore) {
            return "";
        }

        var readFunc = isHolding ? modbusServer.dataStore.readHoldingRegister : modbusServer.dataStore.readInputRegister;

        switch (valueType) {
        case "UINT16":
            return String(readFunc(address));
        case "INT16":
            var val = readFunc(address);
            // 转换为有符号整数
            return String(val > 32767 ? val - 65536 : val);
        case "FLOAT32":
        case "FLOAT":
            var high = readFunc(address);
            var low = readFunc(address + 1);
            return registersToFloat(high, low).toFixed(6);
        case "FLOAT64":
        case "DOUBLE":
            var r0 = readFunc(address);
            var r1 = readFunc(address + 1);
            var r2 = readFunc(address + 2);
            var r3 = readFunc(address + 3);
            return registersToDouble(r0, r1, r2, r3).toFixed(12);
        case "UINT64":
            var r0 = readFunc(address);
            var r1 = readFunc(address + 1);
            var r2 = readFunc(address + 2);
            var r3 = readFunc(address + 3);
            // 避免位运算的有符号转换问题，使用乘法
            var high32 = r0 * 65536 + r1;  // 前32位
            var low32 = r2 * 65536 + r3;   // 后32位
            // JavaScript 精度限制，大于 2^53 会丢失精度
            var result = high32 * 4294967296 + low32;
            return String(Math.round(result));
        case "INT64":
            var r0 = readFunc(address);
            var r1 = readFunc(address + 1);
            var r2 = readFunc(address + 2);
            var r3 = readFunc(address + 3);

            // 先组合高32位
            var high32 = r0 * 65536 + r1;
            // 检查符号位，如果是负数需要转换为有符号的32位数
            if (r0 >= 32768) {
                high32 = high32 - 4294967296;  // 转换为负数
            }

            // 组合低32位（始终是正数）
            var low32 = r2 * 65536 + r3;

            // 最终计算：有符号的高32位 * 2^32 + 无符号的低32位
            var result = high32 * 4294967296 + low32;

            console.log("INT64读取 - 地址:", address, "寄存器:[", r0, r1, r2, r3, "] 高32位:", high32, "低32位:", low32, "结果:", result);

            return String(Math.round(result));
        case "UINT32":
            var high = readFunc(address);
            var low = readFunc(address + 1);
            // 使用乘法避免位运算的有符号问题
            return String(high * 65536 + low);
        case "INT32":
            var high = readFunc(address);
            var low = readFunc(address + 1);
            var uint32Val = high * 65536 + low;
            // 转换为有符号整数
            return String(uint32Val > 2147483647 ? uint32Val - 4294967296 : uint32Val);
        default:
            return String(readFunc(address));
        }
    }

    // 辅助函数：将两个寄存器转换为 float
    function registersToFloat(high, low) {
        var uint32Val = (high << 16) | low;
        var buffer = new ArrayBuffer(4);
        var intView = new Uint32Array(buffer);
        var floatView = new Float32Array(buffer);
        intView[0] = uint32Val;
        return floatView[0];
    }

    // 辅助函数：将四个寄存器转换为 double
    function registersToDouble(r0, r1, r2, r3) {
        // 构造 64 位整数（JavaScript 使用两个 32 位数）
        var high32 = (r0 << 16) | r1;
        var low32 = (r2 << 16) | r3;

        var buffer = new ArrayBuffer(8);
        var intView = new Uint32Array(buffer);
        var floatView = new Float64Array(buffer);

        // 大端序：高位在前
        intView[1] = high32;  // 高32位
        intView[0] = low32;   // 低32位

        return floatView[0];
    }

    // 更新线圈值
    function updateCoilValue(address, value) {
        console.log("收到线圈变化信号 - 地址:", address, "值:", value);
        updateSensorValue(address, "线圈", value ? "1" : "0");
    }

    // 更新离散输入值
    function updateDiscreteInputValue(address, value) {
        console.log("收到离散输入变化信号 - 地址:", address, "值:", value);
        updateSensorValue(address, "离散输入", value ? "1" : "0");
    }

    // 更新保持寄存器值
    function updateHoldingRegisterValue(address, value) {
        console.log("收到保持寄存器变化信号 - 地址:", address, "值:", value);
        updateRegisterValue(address, "保持寄存器", value, true);
    }

    // 更新输入寄存器值
    function updateInputRegisterValue(address, value) {
        console.log("收到输入寄存器变化信号 - 地址:", address, "值:", value);
        updateRegisterValue(address, "输入寄存器", value, false);
    }

    // 更新寄存器值（根据数据类型处理）
    function updateRegisterValue(address, pointType, rawValue, isHolding) {
        console.log("更新寄存器 - 地址:", address, "类型:", pointType, "原始值:", rawValue);
        var found = false;
        for (var i = 0; i < sensorListModel.count; i++) {
            var item = sensorListModel.get(i);
            if (item.address === address && item.pointType === pointType) {
                var displayValue = readRegisterValue(address, item.valueType || "UINT16", isHolding);
                console.log("找到匹配项，索引:", i, "值类型:", item.valueType, "显示值:", displayValue);
                sensorListModel.setProperty(i, "currentValue", displayValue);
                found = true;
                break;
            }
        }
        if (!found) {
            console.log("警告：未找到匹配的传感器 - 地址:", address, "类型:", pointType);
        }
    }

    // 通用更新函数
    function updateSensorValue(address, pointType, value) {
        console.log("开始更新传感器值 - 地址:", address, "类型:", pointType, "值:", value);
        var found = false;
        for (var i = 0; i < sensorListModel.count; i++) {
            var item = sensorListModel.get(i);
            if (item.address === address && item.pointType === pointType) {
                console.log("找到匹配项，索引:", i, "更新前值:", item.currentValue);
                sensorListModel.setProperty(i, "currentValue", value);
                console.log("更新后值:", sensorListModel.get(i).currentValue);
                found = true;
                break;
            }
        }
        if (!found) {
            console.log("警告：未找到匹配的传感器 - 地址:", address, "类型:", pointType);
        }
    }

    // 应用传感器配置到服务器
    function applySensorsToServer() {
        if (!sensorManager || !modbusServer) {
            addLog("错误: 传感器管理器或服务器未初始化");
            return;
        }

        addLog("正在应用传感器配置到服务器...");

        // 临时断开信号连接，避免批量更新时卡死
        disconnectDataStoreSignals();

        if (sensorManager.applyToServer(modbusServer)) {
            addLog("成功应用 " + sensorManager.sensorCount + " 个传感器配置");
            // 重新加载显示（一次性更新）
            displaySensorList();
        } else {
            addLog("应用失败: " + sensorManager.getLastError());
        }

        // 重新连接信号
        connectDataStoreSignals();
    }

    // 断开数据存储信号
    function disconnectDataStoreSignals() {
        if (modbusServer && modbusServer.dataStore) {
            try {
                modbusServer.dataStore.coilChanged.disconnect(updateCoilValue);
                modbusServer.dataStore.discreteInputChanged.disconnect(updateDiscreteInputValue);
                modbusServer.dataStore.holdingRegisterChanged.disconnect(updateHoldingRegisterValue);
                modbusServer.dataStore.inputRegisterChanged.disconnect(updateInputRegisterValue);
                console.log("已断开数据存储信号");
            } catch (e) {
                console.log("断开信号时出错:", e);
            }
        }
    }

    // 连接数据存储信号
    function connectDataStoreSignals() {
        if (modbusServer && modbusServer.dataStore) {
            try {
                modbusServer.dataStore.coilChanged.connect(updateCoilValue);
                modbusServer.dataStore.discreteInputChanged.connect(updateDiscreteInputValue);
                modbusServer.dataStore.holdingRegisterChanged.connect(updateHoldingRegisterValue);
                modbusServer.dataStore.inputRegisterChanged.connect(updateInputRegisterValue);
                console.log("已重新连接数据存储信号");
            } catch (e) {
                console.log("连接信号时出错:", e);
            }
        }
    }

    // 日志配置
    property int maxLogLines: 500  // 最大日志行数，超过时自动清理旧日志
    property bool enablePacketLog: false  // 是否记录收发包详情（高频操作，建议关闭以节省内存）
    property bool enableRequestLog: true  // 是否记录请求日志
    property int logLineCount: 0  // 当前日志行数计数

    // 添加日志（自动滚动到底部，带行数限制）
    function addLog(message) {
        var timestamp = Qt.formatDateTime(new Date(), "hh:mm:ss");
        logDisplay.append("[" + timestamp + "] " + message);
        logLineCount++;

        // 超过最大行数时清理旧日志（保留一半）
        if (logLineCount > maxLogLines) {
            var text = logDisplay.text;
            var lines = text.split('\n');
            // 保留后一半的日志
            var keepLines = Math.floor(maxLogLines / 2);
            var newText = lines.slice(-keepLines).join('\n');
            logDisplay.text = newText;
            logLineCount = keepLines;
        }

        // 自动滚动到底部
        logDisplay.cursorPosition = logDisplay.length;
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
        };
        return fcNames[fc] || "未知功能";
    }

    Component.onCompleted: {
        // modbusServer 和 sensorManager 已经通过 setContextProperty 注入
        // 直接使用即可，但需要验证它们是否存在

        addLog("QML 界面已加载");

        if (modbusServer) {
            addLog("服务器已初始化");

            // 连接信号
            modbusServer.requestReceived.connect(function (fc) {
                if (enableRequestLog) {
                    addLog("收到请求，功能码: " + fc);
                }
            });

            modbusServer.errorOccurred.connect(function (error) {
                addLog("错误: " + error);
            });

            // 连接报文收发信号（可关闭以节省内存）
            modbusServer.packetReceived.connect(function (packet) {
                if (enablePacketLog) {
                    addLog(packet);
                }
            });

            modbusServer.packetSent.connect(function (packet) {
                if (enablePacketLog) {
                    addLog(packet);
                }
            });

            // 连接数据变化信号
            if (modbusServer.dataStore) {
                console.log("连接数据变化信号...");
                connectDataStoreSignals();
                addLog("数据变化信号已连接");
            }
        } else {
            addLog("警告: 无法获取 ModbusServer 对象");
        }

        if (sensorManager) {
            addLog("传感器管理器已初始化");

            // 连接传感器管理器信号
            sensorManager.sensorsLoaded.connect(function (count) {
                addLog("已加载 " + count + " 个传感器配置");
                displaySensorList();
            });

            sensorManager.errorOccurred.connect(function (error) {
                addLog("传感器错误: " + error);
            });
        } else {
            addLog("警告: 无法获取 SensorManager 对象");
        }
    }
}
