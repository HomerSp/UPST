import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

ColumnLayout {
    property string header
    property string value

    id: deviceInfoBox
    height: childrenRect.height
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
        id: deviceInfoBoxHeader
        Layout.fillWidth: true
        Layout.minimumHeight: childrenRect.height + unit.dp(8)
        color: "#000"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: unit.dp(8)
            anchors.verticalCenter: parent.verticalCenter
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.0)
            text: deviceInfoBox.header
            color: "#FFF"
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumHeight: deviceInfoBoxHeader.height
        Layout.maximumHeight: deviceInfoBoxHeader.height
        color: "#f1f1f1"

        TextArea {
            id: deviceInfoBoxText
            anchors.fill: parent
            anchors.leftMargin: unit.dp(8)
            anchors.verticalCenter: parent.verticalCenter
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.0)

            frameVisible: false
            backgroundVisible: false
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            readOnly: true
            selectByMouse: true
            selectByKeyboard: true
            tabChangesFocus: true

            text: deviceInfoBox.value
            textColor: '#000'
        }
    }
}

