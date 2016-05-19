import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import "qrc:/res/qml/components"

Dialog {
    id: logDialog
    visible: false
    modality: Qt.NonModal
    title: qsTr("Log")
    standardButtons: StandardButton.Close

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    contentItem: Item {
        anchors.fill: parent
        anchors.bottomMargin: unit.dp(6.0)

        implicitWidth: unit.dp(640)
        implicitHeight: unit.dp(480)

        ColumnLayout {
            anchors.fill: parent
            spacing: unit.dp(6.0)

            TextArea {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: logDialogText
                text: logText.getLogData()
                font.family: openSansRegularFont.name
                font.pixelSize: unit.em(1.1)
                wrapMode: TextEdit.Wrap
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Close")

                onClicked: {
                    reject();
                }
            }
        }
    }

    onRejected: {
       close()
    }

    Connections {
        target: logText
        onLogDataChanged: {
            logDialogText.text = logText.getLogData()
        }
    }
}

