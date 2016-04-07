import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import "../components"

Dialog {
    id: aboutDialog
    visible: false
    modality: Qt.ApplicationModal
    title: qsTr("About")
    standardButtons: StandardButton.Close
    width: aboutDialogText.width + unit.dp(30)

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    Column {
        id: aboutDialogText
        spacing: unit.dp(8.0)

        Text {
            text: "Ultimobile Product Support Tool v" + programVersion
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.1)
        }

        Text {
            id: aboutDialogPowered
            text: "Powered by Qt v" + qtVersion
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.1)
        }

        Text {
            text: "Icons made by Freepik from www.flaticon.com"
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.1)
        }
    }

    MouseArea {
        property int clickCount
        clickCount: 0
        anchors.fill: aboutDialogText

        onClicked: {
            clickCount = clickCount + 1;
            if(clickCount == 5) {
                aboutDialogPowered.text = qsTr("Powered by Hiking Vikings");
            }
        }
    }

    onRejected: {
       close()
    }
}

