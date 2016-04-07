import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import "components"

Item {
    id: manualModeView
    objectName: "manualModeView"

    Units {
        id: unit
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: unit.dp(8)

        Text {
            anchors.left: parent.left
            anchors.right: parent.right

            text: qsTr("Input")
        }

        TextField {
            objectName: "manualRawInput"
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right

            text: qsTr("Output")
        }

        TextField {
            objectName: "manualRawOutput"
            anchors.left: parent.left
            anchors.right: parent.right
            readOnly: true
        }

        Button {
            objectName: "manualRawSend"
            text: qsTr("Send")
        }
    }
}
