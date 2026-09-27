import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * 固定左侧的控制面板：可折叠，可拖动调整宽度。
 */
Item {
    id: panel

    property real panelWidth: 220
    property real minPanelWidth: 160
    property real maxPanelWidth: 420
    property bool collapsed: false
    property bool connected: false
    property string portType: "UART"
    readonly property var portTypeOptions: ["UART", "MQTT", "USB"]

    property string uartPortName: "COM1"
    property var uartPortOptions: ["COM1", "COM2", "COM3", "COM4", "COM5",
                                   "COM6", "COM7", "COM8", "COM9", "COM10"]
    property int uartBaudRate: 115200
    // 允许空串（编辑中）；正式值：1–9999999 的正整数
    readonly property string uartBaudRegex: "^(?:[1-9][0-9]{0,6})?$"

    property color panelColor: "#242830"
    property color borderColor: "#3a414d"
    property color handleColor: "#3d4554"

    readonly property string uiFontFamily: "Microsoft YaHei UI"
    readonly property int uiFontSize: 15
    readonly property color uiTextColor: "#ffffff"

    readonly property real dragHandleSize: 6
    readonly property real collapseBarHeight: 40
    readonly property real statusBarHeight: 40
    readonly property real portTypeBarHeight: 48
    readonly property real portConfigHeight: 160
    readonly property real connectBarHeight: 48
    readonly property real collapsedWidth: 28

    signal connectRequested()
    signal disconnectRequested()

    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: collapsed ? collapsedWidth : panelWidth
    z: 10

    Behavior on width {
        NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
    }

    Rectangle {
        anchors.fill: parent
        color: panel.panelColor
        border.color: panel.borderColor
        border.width: 1
    }

    // 端口类型选择：面板最上方
    Rectangle {
        id: portTypeBar
        visible: !panel.collapsed
        anchors.left: parent.left
        anchors.right: resizeHandle.left
        anchors.top: parent.top
        height: panel.portTypeBarHeight
        color: "#1e2229"
        border.color: panel.borderColor
        border.width: 1
        z: 3

        DarkComboBox {
            id: portTypeCombo
            anchors.fill: parent
            anchors.margins: 6
            enabled: !panel.connected
            opacity: enabled ? 1.0 : 0.45
            fontFamily: panel.uiFontFamily
            fontPixelSize: panel.uiFontSize
            textColor: panel.uiTextColor
            borderColor: panel.borderColor
            model: panel.portTypeOptions
            currentIndex: {
                var i = panel.portTypeOptions.indexOf(panel.portType)
                return i >= 0 ? i : 0
            }
            onActivated: panel.portType = currentText
        }
    }

    // 端口配置：内容由端口类型决定
    Rectangle {
        id: portConfigBar
        visible: !panel.collapsed
        anchors.left: parent.left
        anchors.right: resizeHandle.left
        anchors.top: portTypeBar.bottom
        height: panel.portConfigHeight
        color: "#1a1e25"
        border.color: panel.borderColor
        border.width: 1
        z: 2
        clip: true

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8
            visible: panel.portType === "UART"
            enabled: !panel.connected
            opacity: enabled ? 1.0 : 0.45

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("串口")
                    color: panel.uiTextColor
                    font.family: panel.uiFontFamily
                    font.pixelSize: panel.uiFontSize
                    font.weight: Font.Medium
                    Layout.preferredWidth: 56
                }

                DarkComboBox {
                    id: uartPortCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    fontFamily: panel.uiFontFamily
                    fontPixelSize: panel.uiFontSize
                    textColor: panel.uiTextColor
                    borderColor: panel.borderColor
                    model: panel.uartPortOptions
                    currentIndex: {
                        var i = panel.uartPortOptions.indexOf(panel.uartPortName)
                        return i >= 0 ? i : 0
                    }
                    onActivated: panel.uartPortName = currentText
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("波特率")
                    color: panel.uiTextColor
                    font.family: panel.uiFontFamily
                    font.pixelSize: panel.uiFontSize
                    font.weight: Font.Medium
                    Layout.preferredWidth: 56
                }

                TextField {
                    id: uartBaudInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    readOnly: panel.connected
                    text: String(panel.uartBaudRate)
                    color: panel.uiTextColor
                    font.family: panel.uiFontFamily
                    font.pixelSize: panel.uiFontSize
                    font.weight: Font.Medium
                    selectByMouse: true
                    leftPadding: 10
                    rightPadding: 10
                    verticalAlignment: TextInput.AlignVCenter
                    placeholderText: "115200"
                    placeholderTextColor: "#7a8494"

                    validator: RegularExpressionValidator {
                        regularExpression: new RegExp(panel.uartBaudRegex)
                    }

                    background: Rectangle {
                        color: "#2a303a"
                        border.color: uartBaudInput.activeFocus && !panel.connected
                                      ? "#5a9aef" : panel.borderColor
                        border.width: 1
                        radius: 3
                    }

                    onEditingFinished: {
                        if (panel.connected)
                            return
                        if (text.length === 0) {
                            text = String(panel.uartBaudRate)
                            return
                        }
                        panel.uartBaudRate = Number(text)
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // 连接 / 断开连接
    Rectangle {
        id: connectBar
        visible: !panel.collapsed
        anchors.left: parent.left
        anchors.right: resizeHandle.left
        anchors.top: portConfigBar.bottom
        height: panel.connectBarHeight
        color: "#1e2229"
        border.color: panel.borderColor
        border.width: 1
        z: 2

        Button {
            id: connectBtn
            anchors.fill: parent
            anchors.margins: 6
            text: panel.connected ? qsTr("断开连接") : qsTr("连接")
            font.family: panel.uiFontFamily
            font.pixelSize: panel.uiFontSize
            font.weight: Font.Medium

            contentItem: Text {
                text: connectBtn.text
                font: connectBtn.font
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 3
                color: {
                    if (connectBtn.down)
                        return panel.connected ? "#b33a3a" : "#1e6fd9"
                    if (connectBtn.hovered)
                        return panel.connected ? "#e05555" : "#2f81f7"
                    return panel.connected ? "#c94a4a" : "#3a7bd5"
                }
                border.color: panel.connected ? "#ff8a8c" : "#5a9aef"
                border.width: 1
            }

            onClicked: {
                if (panel.connected) {
                    panel.disconnectRequested()
                    panel.connected = false
                } else {
                    panel.connectRequested()
                    panel.connected = true
                }
            }
        }
    }

    // 内容区（暂空）
    Item {
        id: contentArea
        anchors.fill: parent
        anchors.margins: 1
        anchors.rightMargin: panel.dragHandleSize + 1
        anchors.topMargin: panel.portTypeBarHeight + panel.portConfigHeight
                           + panel.connectBarHeight + 1
        anchors.bottomMargin: panel.collapseBarHeight + panel.statusBarHeight + 1
        visible: !panel.collapsed
        clip: true
    }

    // 连接状态条：位于折叠按钮上方
    Rectangle {
        id: statusBar
        visible: !panel.collapsed
        anchors.left: parent.left
        anchors.right: resizeHandle.left
        anchors.bottom: collapseBtn.top
        height: panel.statusBarHeight
        color: "#1e2229"
        border.color: panel.borderColor
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 10

            Rectangle {
                width: 12
                height: 12
                radius: 6
                anchors.verticalCenter: parent.verticalCenter
                color: panel.connected ? "#2ee56d" : "#ff4d4f"
                border.color: panel.connected ? "#7dff9f" : "#ff8a8c"
                border.width: 1
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: panel.connected ? qsTr("已连接") : qsTr("未连接")
                font.family: panel.uiFontFamily
                font.pixelSize: panel.uiFontSize
                font.weight: Font.Medium
                color: panel.connected ? "#7dff9f" : "#ff8a8c"
            }
        }
    }

    // 折叠按钮：展开时贴底；折叠时为整高竖条。仅图标，无文字。
    Rectangle {
        id: collapseBtn
        anchors.left: parent.left
        anchors.right: panel.collapsed ? parent.right : resizeHandle.left
        anchors.top: panel.collapsed ? parent.top : undefined
        anchors.bottom: parent.bottom
        height: panel.collapsed ? undefined : panel.collapseBarHeight
        color: collapseMouse.pressed ? "#1e6fd9"
             : collapseMouse.containsMouse ? "#2f81f7"
             : "#3a7bd5"
        border.color: "#5a9aef"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: panel.collapsed ? "»" : "«"
            font.family: panel.uiFontFamily
            font.pixelSize: 22
            font.weight: Font.Bold
            color: "#ffffff"
        }

        MouseArea {
            id: collapseMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: panel.collapsed = !panel.collapsed
        }
    }

    // 右侧拖动手柄：调整宽度
    Rectangle {
        id: resizeHandle
        visible: !panel.collapsed
        width: panel.dragHandleSize
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        color: resizeArea.containsMouse || resizeArea.pressed ? "#4a5568" : panel.handleColor

        Rectangle {
            anchors.centerIn: parent
            width: 2
            height: Math.min(40, parent.height * 0.2)
            radius: 1
            color: "#c5ccd8"
        }

        MouseArea {
            id: resizeArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SizeHorCursor
            preventStealing: true

            property real startX: 0
            property real startWidth: 0

            onPressed: {
                startX = mapToItem(panel.parent, mouse.x, mouse.y).x
                startWidth = panel.panelWidth
            }

            onPositionChanged: {
                if (!pressed)
                    return
                var curX = mapToItem(panel.parent, mouse.x, mouse.y).x
                var next = startWidth + (curX - startX)
                panel.panelWidth = Math.max(panel.minPanelWidth,
                                            Math.min(panel.maxPanelWidth, next))
            }
        }
    }
}
