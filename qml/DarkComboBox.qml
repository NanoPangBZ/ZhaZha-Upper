import QtQuick 2.15
import QtQuick.Controls 2.15

ComboBox {
    id: control

    property color textColor: "#ffffff"
    property color borderColor: "#3a414d"
    property color bgColor: "#2a303a"
    property color accentColor: "#5a9aef"
    property color highlightColor: "#3a7bd5"
    property string fontFamily: "Microsoft YaHei UI"
    property int fontPixelSize: 15

    font.family: fontFamily
    font.pixelSize: fontPixelSize
    font.weight: Font.Medium

    background: Rectangle {
        color: control.bgColor
        border.color: control.pressed || control.popup.visible
                      ? control.accentColor : control.borderColor
        border.width: 1
        radius: 3
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: control.indicator.width + 8
        text: control.displayText
        font: control.font
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: control.width - width - 10
        anchors.verticalCenter: parent.verticalCenter
        text: "▾"
        font.family: control.fontFamily
        font.pixelSize: control.fontPixelSize
        color: control.textColor
    }

    popup: Popup {
        y: control.height
        width: control.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
        }

        background: Rectangle {
            color: control.bgColor
            border.color: control.accentColor
            border.width: 1
            radius: 3
        }
    }

    delegate: ItemDelegate {
        width: control.width
        height: 36
        highlighted: control.highlightedIndex === index

        contentItem: Text {
            text: modelData
            font: control.font
            color: control.textColor
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
        }

        background: Rectangle {
            color: parent.highlighted ? control.highlightColor : "transparent"
        }
    }
}
