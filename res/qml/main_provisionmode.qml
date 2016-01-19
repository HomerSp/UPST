import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2

Item {
    id: provisionView
    objectName: "provisionView"
    anchors.fill: parent

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
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                width: parent.width
                height: childrenRect.height
                color: "transparent"

                Text {
                    id: labelMDN
                    text: qsTr("MDN")
                    font.family: openSansRegularFont.name
                }

                TextField {
                    id: textMDN
                    objectName: "textMDN"
                    anchors.top: labelMDN.bottom
                    anchors.topMargin: 6
                    width: parent.width
                    height: 30
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
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                width: parent.width
                height: childrenRect.height
                color: "transparent"

                Text {
                    id: labelMIN
                    text: qsTr("MIN")
                    font.family: openSansRegularFont.name
                }

                TextField {
                    id: textMIN
                    objectName: "textMIN"
                    anchors.top: labelMIN.bottom
                    anchors.topMargin: 6
                    width: parent.width
                    height: 30
                }
            }

            Button {
                id: provisionButton
                objectName: "provisionButton"
                text: qsTr("Provision")
                width: 100
                height: 30
                anchors.top: minContainerRect.bottom
                anchors.right: minContainerRect.right
                anchors.topMargin: 20
            }
        }
    }
}

