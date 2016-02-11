import QtQuick 2.0
import "components"

Item {
    id: manualModeView
    objectName: "manualModeView"

    Units {
        id: unit
    }

    TextEdit {
        anchors.fill: parent
        anchors.margins: unit.dp(8)
    }
}
