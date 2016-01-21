import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

ColumnLayout {
    property string header
    property string value

    id: deviceInfoBox
    Layout.minimumHeight: childrenRect.height
    spacing: 0

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumHeight: childrenRect.height + unit.dp(8)
        color: "#000"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: unit.dp(8)
            anchors.verticalCenter: parent.verticalCenter
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1)
            text: deviceInfoBox.header
            color: "#FFF"
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumHeight: childrenRect.height + unit.dp(8)
        color: "#f1f1f1"

        TextEdit {
            anchors.left: parent.left
            anchors.leftMargin: unit.dp(8)
            anchors.verticalCenter: parent.verticalCenter
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1)
            readOnly: false
            selectByMouse: true
            focus: true
            persistentSelection: true
            text: deviceInfoBox.value
            color: "#000"
        }
    }
}

