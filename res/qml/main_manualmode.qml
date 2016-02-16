import QtQuick 2.0
import "components"

Item {
    id: manualModeView
    objectName: "manualModeView"

    Units {
        id: unit
    }

    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"

        TextEdit {
            objectName: "manualModeOutput"
            anchors.fill: parent
            anchors.margins: unit.dp(8)
            selectByMouse: true
        }
    }
}
