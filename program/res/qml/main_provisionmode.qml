import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import "qrc:/res/qml/components"

Item {
    id: provisionView
    objectName: "provisionView"
    anchors.fill: parent

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }
    FontLoader {
        id: openSansBoldFont;
        source: "qrc:/res/fonts/OpenSans-Bold.ttf"
    }

    ColumnLayout {
        anchors {left: parent.left; right: parent.right;}
        anchors.margins: unit.dp(20)
        anchors.verticalCenter: parent.verticalCenter
        spacing: unit.dp(12)

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            height: childrenRect.height

            ColumnLayout {
                Layout.fillWidth: true
                id: spcLayout
                objectName: "spcLayout"
                anchors.right: parent.right
                spacing: unit.dp(12)
                width: (visible)?(unit.dp(64)):0
                visible: false

                Text {
                    Layout.fillWidth: true
                    id: labelSPC
                    text: qsTr("SPC")
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMDN.font.pixelSize
                }

                TextField {
                    Layout.fillWidth: true
                    id: textSPC
                    objectName: "textSPC"
                    height: labelSPC.height + unit.dp(12)
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelSPC.font.pixelSize
                    maximumLength: 6
                }
            }

            GridLayout {
                Layout.fillWidth: true
                anchors {left: parent.left; right: spcLayout.left}
                anchors.rightMargin: (spcLayout.visible)?unit.dp(20):0

                rowSpacing: unit.dp(12)
                columnSpacing: unit.dp(20)

                flow: GridLayout.LeftToRight
                columns: 2

                z: 99

                Text {
                    Layout.fillWidth: true
                    id: labelMDN
                    text: qsTr("MDN")
                    font.family: openSansRegularFont.name
                    font.pixelSize: unit.em(1.1)
                }

                Text {
                    Layout.fillWidth: true
                    id: labelMIN
                    text: qsTr("MIN")
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMDN.font.pixelSize
                }

                TextField {
                    Layout.fillWidth: true
                    id: textMDN
                    objectName: "textMDN"
                    height: labelMDN.height + unit.dp(12)
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMDN.font.pixelSize
                    maximumLength: 10
                }

                TextField {
                    Layout.fillWidth: true
                    id: textMIN
                    objectName: "textMIN"
                    height: labelMIN.height + unit.dp(12)
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMIN.font.pixelSize
                    maximumLength: 10
                }
            }
        }

        Item {
            Layout.fillWidth: true

            UPButton {
                id: provisionButton
                objectName: "provisionButton"
                anchors.right: parent.right
                text: qsTr("Provision")
                width: unit.dp(80)
                height: textMIN.height

                scheme: "light"
            }
        }
    }
}

