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
    minimumWidth: 1100
    minimumHeight: 700
    color: "#D8DADF"

    property color pageBgTop: "#D7DADF"
    property color pageBgBottom: "#C4C9D0"
    property color panelBg: "#E3E5E9"
    property color panelBorder: "#A2A9B1"
    property color panelTitle: "#1F1F1F"
    property color accentColor: "#616872"
    property color accentSoft: "#CDD2D9"
    property color bodyText: "#262626"
    property color mutedText: "#5A5A5A"
    property color controlBg: "#E7EAEE"
    property color controlBorder: "#919AA5"
    property color controlHover: "#DCE1E7"
    property color controlPressed: "#C7CED6"
    property color controlText: "#1F252C"
    property int sensorTableMinWidth: 1500
    property int tcpPort: 502
    property string rtuPortName: "COM1"
    property int rtuBaudRate: 9600
    property var rtuBaudRates: [9600, 19200, 38400, 57600, 115200]

    font.family: "Microsoft YaHei UI"

    palette.windowText: bodyText
    palette.text: bodyText
    palette.buttonText: bodyText
    palette.base: "#E8EBEF"
    palette.button: "#D0D5DC"
    palette.highlight: "#B9C0C8"
    palette.highlightedText: "#222222"

    component IndustrialButton: Button {
        implicitHeight: 34
        font.pixelSize: 12
        contentItem: Text {
            text: parent.text
            font: parent.font
            color: root.controlText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: !parent.enabled ? "#D0D5DC" : (parent.down ? root.controlPressed : (parent.hovered ? root.controlHover : root.controlBg))
        }
    }

    component IndustrialSpinBox: SpinBox {
        implicitHeight: 34
        editable: true
        font.pixelSize: 12
        down.indicator: Rectangle {
            x: 0
            y: 0
            width: 34
            height: parent.height
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: parent.down.pressed ? root.controlPressed : (parent.down.hovered ? root.controlHover : root.controlBg)

            Text {
                anchors.centerIn: parent
                text: "-"
                color: root.controlText
                font.pixelSize: 18
                font.bold: true
            }
        }
        up.indicator: Rectangle {
            x: parent.width - width
            y: 0
            width: 34
            height: parent.height
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: parent.up.pressed ? root.controlPressed : (parent.up.hovered ? root.controlHover : root.controlBg)

            Text {
                anchors.centerIn: parent
                text: "+"
                color: root.controlText
                font.pixelSize: 18
                font.bold: true
            }
        }
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: root.controlBg
        }
        contentItem: TextInput {
            z: 2
            text: parent.textFromValue(parent.value, parent.locale)
            font: parent.font
            color: root.controlText
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            leftPadding: parent.down.indicator.width
            rightPadding: parent.up.indicator.width
            readOnly: !parent.editable
            validator: parent.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
        }
    }

    component IndustrialCheckBox: CheckBox {
        font.pixelSize: 12
        spacing: 8
        indicator: Rectangle {
            implicitWidth: 16
            implicitHeight: 16
            radius: 2
            border.color: root.controlBorder
            border.width: 1
            color: parent.checked ? "#4B545F" : "#F4F6F8"

            Text {
                anchors.centerIn: parent
                text: "✓"
                visible: parent.parent.checked
                color: "#F2F4F6"
                font.pixelSize: 11
                font.bold: true
            }
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            color: root.bodyText
            verticalAlignment: Text.AlignVCenter
            leftPadding: parent.indicator.width + parent.spacing
        }
    }

    component IndustrialTextField: TextField {
        implicitHeight: 34
        font.pixelSize: 12
        color: root.controlText
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: root.controlBg
        }
    }

    component IndustrialComboBox: ComboBox {
        implicitHeight: 34
        font.pixelSize: 12
        contentItem: Text {
            text: parent.displayText
            font: parent.font
            color: root.controlText
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
            rightPadding: 24
            elide: Text.ElideRight
        }
        indicator: Rectangle {
            x: parent.width - width - 8
            y: (parent.height - height) / 2
            width: 10
            height: 10
            color: "transparent"
            border.color: "transparent"

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    ctx.moveTo(1, 3);
                    ctx.lineTo(width - 1, 3);
                    ctx.lineTo(width / 2, height - 2);
                    ctx.closePath();
                    ctx.fillStyle = "#3D4650";
                    ctx.fill();
                }
            }
        }
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: root.controlBorder
            color: root.controlBg
        }
    }

    component IndustrialScrollBar: ScrollBar {
        policy: ScrollBar.AsNeeded
        contentItem: Rectangle {
            implicitWidth: 8
            implicitHeight: 8
            radius: 4
            color: parent.pressed ? "#7F8792" : "#969FAA"
        }
        background: Rectangle {
            color: "#C9CED5"
            radius: 4
        }
    }

    menuBar: MenuBar {
        background: Rectangle {
            color: "#BBC2CA"
            border.color: "#99A1AB"
            border.width: 1
        }
        Menu {
            title: "文件"

            Action {
                text: "导入传感器配置..."
                onTriggered: importFileDialog.open()
            }

            Action {
                text: "导出传感器配置..."
                enabled: sensorManager && sensorManager.sensorCount > 0
                onTriggered: exportFileDialog.open()
            }

            MenuSeparator {}

            Action {
                text: "清空日志"
                onTriggered: clearLog()
            }

            MenuSeparator {}

            Action {
                text: "退出"
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: "服务器"

            Menu {
                title: "TCP 配置"

                Action {
                    text: "设置端口..."
                    onTriggered: tcpConfigDialog.open()
                }

                Action {
                    text: "启动 TCP (端口 " + root.tcpPort + ")"
                    enabled: modbusServer && !modbusServer.running
                    onTriggered: startTcpServer()
                }
            }

            Menu {
                title: "RTU 配置"

                Action {
                    text: "设置串口与波特率..."
                    onTriggered: rtuConfigDialog.open()
                }

                Action {
                    text: "启动 RTU (" + root.rtuPortName + " / " + root.rtuBaudRate + ")"
                    enabled: modbusServer && !modbusServer.running
                    onTriggered: startRtuServer()
                }
            }

            MenuSeparator {}

            Action {
                text: "停止服务器"
                enabled: modbusServer && modbusServer.running
                onTriggered: stopServer()
            }

            Action {
                text: "初始化数据"
                enabled: modbusServer
                onTriggered: initializeServerData()
            }

            MenuSeparator {}

            Action {
                text: "内存报告"
                enabled: modbusServer
                onTriggered: showMemoryReport()
            }

            Action {
                text: "清理内存"
                enabled: modbusServer
                onTriggered: clearServerMemory()
            }
        }

        Menu {
            title: "视图"

            Action {
                text: "打开操作日志"
                onTriggered: {
                    logWindow.show();
                    logWindow.raise();
                    logWindow.requestActivate();
                    logDisplay.forceActiveFocus();
                    logDisplay.cursorPosition = logDisplay.length;
                }
            }

            Action {
                text: "刷新传感器列表"
                enabled: sensorManager
                onTriggered: displaySensorList()
            }
        }

        Menu {
            title: "帮助"

            Action {
                text: "关于"
                onTriggered: aboutDialog.open()
            }
        }
    }

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
                    radius: 6
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
                        color: root.bodyText
                        font.bold: false
                    }
                    Label {
                        id: runningLabel
                        text: modbusServer ? (modbusServer.running ? "运行中" : "已停止") : "未知"
                        color: root.bodyText
                        font.pixelSize: 14
                        font.bold: false
                    }

                    Label {
                        text: "模式:"
                        color: root.bodyText
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
                        color: root.bodyText
                        font.bold: false
                    }
                    Label {
                        id: requestCountLabel
                        text: modbusServer ? modbusServer.requestCount.toString() : "0"
                        color: root.bodyText
                        font.pixelSize: 14
                        font.bold: false
                    }

                    Label {
                        text: "最后功能码:"
                        color: root.bodyText
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
                        color: root.bodyText
                        font.pixelSize: 13
                        font.bold: false
                    }

                    Label {
                        text: "状态消息:"
                        color: root.bodyText
                        font.bold: false
                    }
                    Label {
                        id: statusLabel
                        text: modbusServer ? modbusServer.statusMessage : "未启动"
                        width: Math.max(180, serverStatusBox.width * 0.35)
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                        color: root.bodyText
                    }
                }
            }

            // 数据监控区域 - 使用 TabBar
            GroupBox {
                title: "文件寄存器与传感器配置"
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 500
                Layout.preferredHeight: root.height - serverStatusBox.height - 200
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
                    radius: 6
                    color: root.panelBg
                    border.color: root.panelBorder
                    border.width: 1
                }

                ColumnLayout {
                    id: contentLayout
                    anchors.fill: parent
                    // spacing: 8

                    TabBar {
                        id: dataTabBar
                        Layout.fillWidth: true
                        spacing: 8
                        background: Rectangle {
                            radius: 4
                            color: "#C4CAD2"
                            border.color: "#9FA7B1"
                            border.width: 1
                        }

                        TabButton {
                            text: "文件寄存器"
                            font.pixelSize: 13
                            font.bold: checked
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: parent.checked ? "#1F1F1F" : root.mutedText
                            }
                            background: Rectangle {
                                radius: 3
                                color: parent.checked ? "#AEB5BE" : "transparent"
                                border.width: parent.checked ? 1 : 0
                                border.color: "#8F98A3"
                            }
                        }
                        TabButton {
                            text: "传感器配置"
                            font.pixelSize: 13
                            font.bold: checked
                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                color: parent.checked ? "#1F1F1F" : root.mutedText
                            }
                            background: Rectangle {
                                radius: 3
                                color: parent.checked ? "#AEB5BE" : "transparent"
                                border.width: parent.checked ? 1 : 0
                                border.color: "#8F98A3"
                            }
                        }
                    }

                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
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
                                    columns: root.width >= 1200 ? 2 : 1
                                    rowSpacing: 10
                                    columnSpacing: 10

                                    // 标准文件记录
                                    GroupBox {
                                        title: "标准文件记录 (FC 20/21)"
                                        Layout.fillWidth: true
                                        Layout.fillHeight: false
                                        Layout.minimumHeight: 190
                                        font.bold: false
                                        padding: 10
                                        topPadding: 28

                                        label: Label {
                                            text: parent.title
                                            color: root.panelTitle
                                            font.pixelSize: 13
                                            font.bold: true
                                            leftPadding: 8
                                        }

                                        background: Rectangle {
                                            radius: 4
                                            color: "#D9DDE2"
                                            border.color: "#AAB1BA"
                                            border.width: 1
                                        }

                                        // 内容用 ColumnLayout + GridLayout 组合
                                        ColumnLayout {
                                            anchors.fill: parent
                                            spacing: 6

                                            GridLayout {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: false
                                                columns: 2
                                                rowSpacing: 6
                                                columnSpacing: 8

                                                Label {
                                                    text: "文件号:"
                                                }
                                                IndustrialSpinBox {
                                                    id: fileNumberSpinBox
                                                    from: 0
                                                    to: 65535
                                                    value: 1
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "记录号:"
                                                }
                                                IndustrialSpinBox {
                                                    id: recordNumberSpinBox
                                                    from: 0
                                                    to: 9999
                                                    value: 0
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "记录数:"
                                                }
                                                IndustrialSpinBox {
                                                    id: recordCountSpinBox
                                                    from: 1
                                                    to: 126
                                                    value: 10
                                                    Layout.fillWidth: true
                                                    ToolTip.visible: hovered
                                                    ToolTip.text: "Modbus标准限制：单次最多读取126个记录（252字节）"
                                                }
                                            }

                                            // 按钮区域（横排）
                                            RowLayout {
                                                Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignBottom
                                                spacing: 8

                                                IndustrialButton {
                                                    text: "查询文件内容"
                                                    Layout.fillWidth: true
                                                    onClicked: queryFileContent()
                                                }
                                                IndustrialButton {
                                                    text: "上传文件"
                                                    Layout.fillWidth: true
                                                    onClicked: uploadFile()
                                                }
                                                IndustrialButton {
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
                                        Layout.minimumHeight: 190
                                        font.bold: false
                                        padding: 10
                                        topPadding: 28

                                        label: Label {
                                            text: parent.title
                                            color: root.panelTitle
                                            font.pixelSize: 13
                                            font.bold: true
                                            leftPadding: 8
                                        }

                                        background: Rectangle {
                                            radius: 4
                                            color: "#D9DDE2"
                                            border.color: "#AAB1BA"
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
                                                IndustrialSpinBox {
                                                    id: fileAddressSpinBox
                                                    from: 0
                                                    to: 65535
                                                    value: 1000
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "寄存器数:"
                                                }
                                                IndustrialSpinBox {
                                                    id: fileRegisterCountSpinBox
                                                    from: 1
                                                    to: 125
                                                    value: 20
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            // 按钮横排
                                            RowLayout {
                                                Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignBottom
                                                spacing: 8

                                                IndustrialButton {
                                                    text: "查询保持寄存器"
                                                    Layout.fillWidth: true
                                                    onClicked: queryAddressFileContent()
                                                }
                                                IndustrialButton {
                                                    text: "上传保持寄存器"
                                                    Layout.fillWidth: true
                                                    onClicked: uploadHoldingRegisters()
                                                }
                                                IndustrialButton {
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
                            id: sensorConfigTabItem
                            implicitHeight: sensorConfigColumn.implicitHeight
                            ColumnLayout {
                                id: sensorConfigColumn
                                anchors.fill: parent
                                spacing: 10

                                // Excel 导入导出控制
                                Rectangle {
                                    id: sensorToolbar
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: Math.max(48, sensorToolbarFlow.implicitHeight + 16)
                                    color: "#D1D6DD"
                                    radius: 4
                                    border.color: "#A6AEB8"
                                    border.width: 1

                                    Flow {
                                        id: sensorToolbarFlow
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 8

                                        Label {
                                            text: "传感器配置管理"
                                            color: root.bodyText
                                            font.bold: false
                                        }

                                        Rectangle {
                                            width: 1
                                            height: 20
                                            color: "#A0A0A0"
                                        }

                                        IndustrialButton {
                                            text: "导入 Excel/CSV"
                                            onClicked: importFileDialog.open()
                                        }

                                        IndustrialButton {
                                            text: "导出配置"
                                            onClicked: exportFileDialog.open()
                                        }

                                        IndustrialButton {
                                            text: "应用到服务器"
                                            enabled: sensorManager && sensorManager.sensorCount > 0
                                            onClicked: applySensorsToServer()
                                        }

                                        IndustrialButton {
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
                                            color: root.bodyText
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

                                        Flickable {
                                            id: sensorTableFlick
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            clip: true
                                            // 1. 移除 AlwaysOff，改用自定义样式让它“悬浮”
                                            ScrollBar.vertical: ScrollBar {
                                                // 只有当内容真的超出时才显示
                                                policy: ScrollBar.AsNeeded 
                                                
                                                // 2. 关键：背景透明，这样它就不会占据 Layout 的高度
                                                background: Rectangle {
                                                    color: "transparent"
                                                    width: 10 // 这里的宽度仅影响鼠标感应区，视觉上不可见
                                                }
                                                
                                                // 3. 定义滑块的样子 (可选，保持美观)
                                                contentItem: Rectangle {
                                                    implicitWidth: 8
                                                    color: "#cdcdcd"
                                                    radius: width / 2
                                                    opacity: 0.0 // 默认隐藏
                                                    // 鼠标悬停或滚动时显示 (简单处理：始终半透明可见，或者根据需要加状态机)
                                                    states: State {
                                                        name: "active"
                                                        when: sensorTableFlick.ScrollBar.active
                                                        PropertyChanges { target: sensorTableFlick.ScrollBar.contentItem; opacity: 0.75 }
                                                    }
                                                }
                                            }
            
                                            ScrollBar.horizontal: ScrollBar {
                                                policy: ScrollBar.AsNeeded
                                                background: Rectangle { color: "transparent" }
                                                contentItem: Rectangle {
                                                    implicitHeight: 8
                                                    color: "#cdcdcd"
                                                    radius: height / 2
                                                    opacity: sensorTableFlick.ScrollBar.horizontal.active ? 0.75 : 0.0
                                                }
                                            }

                                            contentWidth: Math.max(width, root.sensorTableMinWidth)
                                            contentHeight: height
                                            flickableDirection: Flickable.HorizontalFlick

                                            // 表格标题行
                                            Rectangle {
                                                id: sensorHeaderRow
                                                width: sensorTableFlick.contentWidth
                                                height: 35
                                                color: "#e8e8e8"
                                                border.color: "#9DA5AF"
                                                border.width: 1

                                                Row {
                                                    anchors.fill: parent
                                                    spacing: 0

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
                                                                width: 0.08
                                                            },
                                                            {
                                                                text: "初始值",
                                                                width: 0.07
                                                            },
                                                            {
                                                                text: "当前值",
                                                                width: 0.07
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
                                                            border.color: "#9DA5AF"
                                                            border.width: 1

                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: modelData.text
                                                                font.bold: true
                                                                font.pixelSize: 12
                                                                color: modelData.color || root.bodyText
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            // 数据显示区域 - ListView
                                            ListView {
                                                id: sensorListView
                                                anchors.top: sensorHeaderRow.bottom
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                anchors.bottom: parent.bottom
                                                width: sensorTableFlick.contentWidth
                                                clip: true
                                                boundsBehavior: Flickable.StopAtBounds

                                                model: ListModel {
                                                    id: sensorListModel
                                                }

                                                delegate: Rectangle {
                                                    width: sensorListView.width
                                                    height: 30
                                                    color: index % 2 === 0 ? "#E5E8EC" : "#DDE1E6"
                                                    border.color: "#A6ADB7"
                                                    border.width: 1

                                                    Row {
                                                        anchors.fill: parent
                                                        spacing: 0

                                                        Rectangle {
                                                            width: parent.width * 0.05
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.address !== undefined ? model.address : ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.05
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.pointName || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.10
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.pointType || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.08
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.valueType || "UINT16"
                                                                font.pixelSize: 11
                                                                font.bold: true
                                                                color: root.bodyText
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.07
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.initialValue || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.07
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.currentValue || ""
                                                                font.pixelSize: 11
                                                                font.bold: true
                                                                color: root.bodyText
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.10
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.note || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.08
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.unit || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.12
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.minValue || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.12
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.maxValue || ""
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.06
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
                                                            border.width: 1
                                                            Label {
                                                                anchors.centerIn: parent
                                                                text: model.readOnly ? "是" : "否"
                                                                font.pixelSize: 11
                                                                elide: Text.ElideRight
                                                            }
                                                        }

                                                        Rectangle {
                                                            width: parent.width * 0.10
                                                            height: parent.height
                                                            color: "transparent"
                                                            border.color: "#A6ADB7"
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
                                                    color: root.mutedText
                                                    horizontalAlignment: Text.AlignHCenter
                                                }

                                                ScrollBar.vertical: IndustrialScrollBar {}
                                            }

                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 日志窗口
    Window {
        id: logWindow
        width: 800
        height: 500
        minimumWidth: 700
        minimumHeight: 420
        title: "操作日志"
        color: "#E9E9E9"

        onClosing: {
            hide();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                radius: 8
                color: "#F3F3F3"
                border.color: "#C8C8C8"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 10

                    IndustrialCheckBox {
                        id: packetLogCheckBox
                        text: "记录收发包"
                        checked: enablePacketLog
                        onCheckedChanged: enablePacketLog = checked
                        ToolTip.visible: hovered
                        ToolTip.text: "启用后会记录每个Modbus请求/响应的详细报文（高频操作，建议关闭以节省内存）"
                    }

                    IndustrialCheckBox {
                        id: requestLogCheckBox
                        text: "记录请求"
                        checked: enableRequestLog
                        onCheckedChanged: enableRequestLog = checked
                        ToolTip.visible: hovered
                        ToolTip.text: "记录每个Modbus请求的功能码"
                    }

                    Label {
                        text: "最大行数:"
                        color: root.bodyText
                        font.pixelSize: 11
                    }
                    IndustrialSpinBox {
                        id: maxLogLinesSpinBox
                        from: 100
                        to: 5000
                        value: maxLogLines
                        stepSize: 100
                        Layout.preferredWidth: 120
                        font.pixelSize: 11
                        onValueChanged: maxLogLines = value
                        ToolTip.visible: hovered
                        ToolTip.text: "日志超过此行数时自动清理旧日志"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    IndustrialButton {
                        text: "清空日志"
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        font.pixelSize: 11
                        onClicked: clearLog()
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                background: Rectangle {
                    radius: 8
                    color: "#FFFFFF"
                    border.color: "#C8C8C8"
                    border.width: 1
                }

                TextArea {
                    id: logDisplay
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    font.family: "Consolas, Monaco, monospace"
                    font.pixelSize: 12
                    selectByMouse: true
                    text: "服务器日志将显示在这里...\n"
                    background: Rectangle {
                        radius: 8
                        color: "#FAFAFA"
                        border.color: "#D0D0D0"
                        border.width: 1
                    }
                    color: "#202020"
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

    Dialog {
        id: aboutDialog
        title: "关于"
        modal: true
        standardButtons: Dialog.Ok
        width: 420

        contentItem: Label {
            text: "Qt6 Modbus Slave Simulator\n\n用于调试 Modbus TCP / RTU 从站、文件寄存器与传感器配置。\n\n新增菜单栏后，可直接从顶部访问导入导出、服务控制与日志操作。"
            wrapMode: Text.WordWrap
            color: root.bodyText
            padding: 14
        }
    }

    Dialog {
        id: tcpConfigDialog
        title: "TCP 配置"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 380

        onOpened: tcpPortFieldInDialog.text = root.tcpPort.toString()
        onAccepted: {
            var newPort = parseInt(tcpPortFieldInDialog.text);
            if (isNaN(newPort) || newPort < 1 || newPort > 65535) {
                addLog("TCP 端口无效，请输入 1-65535");
                return;
            }

            root.tcpPort = newPort;
            addLog("TCP 端口已更新为: " + root.tcpPort);
        }

        contentItem: ColumnLayout {
            spacing: 10

            Label {
                text: "TCP 监听端口"
                color: root.bodyText
            }

            IndustrialTextField {
                id: tcpPortFieldInDialog
                Layout.fillWidth: true
                placeholderText: "1 - 65535"
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }
    }

    Dialog {
        id: rtuConfigDialog
        title: "RTU 配置"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 420

        onOpened: {
            rtuPortFieldInDialog.text = root.rtuPortName;
            var baudText = root.rtuBaudRate.toString();
            var idx = baudRateComboInDialog.find(baudText);
            if (idx >= 0)
                baudRateComboInDialog.currentIndex = idx;
        }
        onAccepted: {
            if (rtuPortFieldInDialog.text.trim().length === 0) {
                addLog("RTU 串口名不能为空");
                return;
            }

            root.rtuPortName = rtuPortFieldInDialog.text.trim();
            root.rtuBaudRate = parseInt(baudRateComboInDialog.currentText);
            addLog("RTU 配置已更新: " + root.rtuPortName + " / " + root.rtuBaudRate);
        }

        contentItem: GridLayout {
            columns: 2
            rowSpacing: 10
            columnSpacing: 8

            Label {
                text: "串口"
            }
            IndustrialTextField {
                id: rtuPortFieldInDialog
                Layout.fillWidth: true
                placeholderText: "COM1"
            }

            Label {
                text: "波特率"
            }
            IndustrialComboBox {
                id: baudRateComboInDialog
                Layout.fillWidth: true
                model: root.rtuBaudRates
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

    function startTcpServer() {
        if (modbusServer) {
            addLog("尝试启动 TCP 服务器，端口: " + root.tcpPort);
            if (modbusServer.startTcp(root.tcpPort)) {
                statusLabel.text = "TCP 服务器已启动";
                addLog("TCP 服务器启动成功");
            } else {
                addLog("TCP 服务器启动失败");
            }
        } else {
            addLog("错误: ModbusServer 对象未初始化");
        }
    }

    function startRtuServer() {
        if (modbusServer) {
            addLog("尝试启动 RTU 服务器，串口: " + root.rtuPortName + ", 波特率: " + root.rtuBaudRate);
            if (modbusServer.startRtu(root.rtuPortName, root.rtuBaudRate)) {
                statusLabel.text = "RTU 服务器已启动";
                addLog("RTU 服务器启动成功");
            } else {
                addLog("RTU 服务器启动失败");
            }
        } else {
            addLog("错误: ModbusServer 对象未初始化");
        }
    }

    function clearLog() {
        logDisplay.clear();
        logLineCount = 0;
    }

    function stopServer() {
        if (modbusServer) {
            modbusServer.stop();
            statusLabel.text = "服务器已停止";
        }
    }

    function initializeServerData() {
        if (modbusServer) {
            modbusServer.initializeData();
            statusLabel.text = "数据已初始化";
        }
    }

    function showMemoryReport() {
        if (modbusServer) {
            var report = modbusServer.getMemoryUsageReport();
            addLog("\n" + report + "\n");
        }
    }

    function clearServerMemory() {
        if (modbusServer) {
            modbusServer.clearAllData();
            addLog("已清理所有数据");
            displaySensorList();
        }
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
