import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import "qrc:/res/qml/components"

Window {
    property string guideData

    id: guideWindow
    visible: false

    width: unit.dp(990)
    height: unit.dp(540)
    minimumHeight: unit.dp(540)

    flags: Qt.Window

    modality: Qt.NonModal
    title: ""

    Settings {
        category: "userguide"
        property alias x: guideWindow.x
        property alias y: guideWindow.y
        property alias width: guideWindow.width
        property alias height: guideWindow.height
    }

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            anchors.left: parent.left
            anchors.top: parent.top

            height: unit.dp(75)

            z: 2

            color: "#1976D2"

            Image {
                id: logoImage
                anchors.right: parent.right
                anchors.rightMargin: unit.dp(16)
                anchors.verticalCenter: parent.verticalCenter

                height: logoText.height

                source: "qrc:/res/images/logo.svg"
                fillMode: Image.PreserveAspectFit
                mipmap: true
            }

            ColumnLayout {
                id: logoText
                Layout.fillWidth: true

                anchors.left: parent.left
                anchors.leftMargin: unit.dp(16)
                anchors.right: logoImage.left
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "UPST"
                    color: "#FFF"

                    font.pixelSize: unit.em(1.4)
                    font.family: openSansRegularFont.name
                }

                Text {
                    id: deviceGuideHeaderDevice
                    text: "User guide"
                    color: "#FFF"

                    font.pixelSize: unit.em(1.3)
                    font.family: openSansRegularFont.name
                }
            }
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: userguideLayout

            z: 1

            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

            textFormat: TextEdit.RichText
            wrapMode: TextEdit.Wrap
            readOnly: true

            text: guideData

            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.2)
        }
    }
}

