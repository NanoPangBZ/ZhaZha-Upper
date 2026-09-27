import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 1440
    height: 960
    visible: true
    title: qsTr("ZhaZha Upper")

    color: "#1a1d23"

    readonly property string uiFontFamily: "Microsoft YaHei UI"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("ZhaZha Upper")
            color: "#ffffff"
            font.family: root.uiFontFamily
            font.pixelSize: 40
            font.weight: Font.Bold
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Qt Quick + CMake 基础工程已就绪")
            color: "#d7dde8"
            font.family: root.uiFontFamily
            font.pixelSize: 18
            font.weight: Font.Medium
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            text: qsTr("ComHost 测试")
            font.family: root.uiFontFamily
            font.pixelSize: 15
            font.weight: Font.Medium
            onClicked: {
                resultText.text = bridge.ComHostTest()
            }
        }

        Text {
            id: resultText
            Layout.alignment: Qt.AlignHCenter
            text: bridge.lastTestResult
            color: "#8ec8ff"
            font.family: root.uiFontFamily
            font.pixelSize: 16
            font.weight: Font.Medium
        }
    }

    ControlPanel {
        id: controlPanel
        panelWidth: 220
    }
}
