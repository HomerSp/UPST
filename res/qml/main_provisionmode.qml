import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import "components"

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

    GridLayout {
        anchors.fill: parent
        columns: 2
        Item {
            id: mdnContainer
            Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.fillWidth: true

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: unit.dp(20)
                anchors.rightMargin: anchors.leftMargin
                width: parent.width
                height: childrenRect.height
                color: "transparent"

                Text {
                    id: labelMDN
                    text: qsTr("MDN")
                    font.family: openSansRegularFont.name
                    font.pixelSize: unit.em(1.1)
                }

                TextField {
                    id: textMDN
                    objectName: "textMDN"
                    anchors.top: labelMDN.bottom
                    anchors.topMargin: unit.dp(4)
                    width: parent.width
                    height: labelMDN.height + unit.dp(12)
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMDN.font.pixelSize
                    maximumLength: 10
                }
            }
        }

        Item {
            id: minContainer
            Layout.alignment: Qt.AlignRight
            Layout.fillHeight: true
            Layout.fillWidth: true

            Rectangle {
                id: minContainerRect
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: unit.dp(20)
                anchors.rightMargin: anchors.leftMargin
                width: parent.width
                height: childrenRect.height
                color: "transparent"

                Text {
                    id: labelMIN
                    text: qsTr("MIN")
                    font.family: openSansRegularFont.name
                    font.pixelSize: unit.em(1.1)
                }

                TextField {
                    id: textMIN
                    objectName: "textMIN"
                    anchors.top: labelMIN.bottom
                    anchors.topMargin: unit.dp(4)
                    width: parent.width
                    height: labelMIN.height + unit.dp(12)
                    font.family: openSansRegularFont.name
                    font.pixelSize: labelMIN.font.pixelSize
                    maximumLength: 10
                }
            }

            UPButton {
                id: provisionButton
                objectName: "provisionButton"
                text: qsTr("Provision")
                width: minContainerRect.width / 2
                height: textMIN.height
                anchors.top: minContainerRect.bottom
                anchors.right: minContainerRect.right
                anchors.topMargin: unit.dp(20)

                scheme: "light"
            }
        }
    }
}

