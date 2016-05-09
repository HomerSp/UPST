import QtQuick 2.5
import "qrc:/res/qml/components"

Item {
    id: deviceInfoView
    objectName: "deviceInfoView"

    Units {
        id: unit
    }

    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"

        TextEdit {
            objectName: "deviceInfoText"
            anchors.fill: parent
            anchors.margins: unit.dp(8)
            selectByMouse: true
        }
    }
}
