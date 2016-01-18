import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: provisionView
    objectName: "provisionView"
    anchors.fill: parent

    Text {
        id: labelMDN
        text: qsTr("MDN")
        anchors.horizontalCenter: parent.horizontalCenter
    }

    TextField {
        id: textMDN
        objectName: "textMDN"
        anchors.top: labelMDN.bottom
        width: 100
        height: 24
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Button {
        id: provisionButton
        objectName: "provisionButton"
        text: qsTr("Provision")
        width: 100
        height: 30
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }
}

