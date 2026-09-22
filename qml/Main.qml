import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 960
    height: 640
    visible: true
    title: qsTr("ZhaZha Upper")

    color: "#1a1d23"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("ZhaZha Upper")
            color: "#f0f2f5"
            font.pixelSize: 36
            font.weight: Font.DemiBold
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Qt Quick + CMake 基础工程已就绪")
            color: "#9aa3b2"
            font.pixelSize: 16
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            text: qsTr("点我试试")
            onClicked: greeting.text = qsTr("Hello, Qt Quick!")
        }

        Text {
            id: greeting
            Layout.alignment: Qt.AlignHCenter
            text: ""
            color: "#6cb6ff"
            font.pixelSize: 14
        }
    }
}
