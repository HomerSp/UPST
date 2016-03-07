import QtQuick 2.0
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

        TextInput {
            objectName: "manualRawInput"
            anchors.fill: parent
        }

        TextInput {
            objectName: "manualRawOutput"
            anchors.fill: parent
            readOnly: true
        }
    }
}
