import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import "qrc:/res/qml/components"

ApplicationWindow {
    property string processStatus

    id: mainWindow
    objectName: "mainWindow"
    visible: true
    minimumWidth: unit.dp(640)
    minimumHeight: unit.dp(480)
    color: "#f1f1f1"

    processStatus: ""

    Component.onCompleted: {
        raise();
        requestActivate();
    }

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    FontLoader {
        id: openSansBoldFont;
        source: "qrc:/res/fonts/OpenSans-Bold.ttf"
    }

    Image {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: unit.dp(8)
        anchors.topMargin: unit.dp(8)
        z: 2

        source: "qrc:/res/images/logo.svg"
        fillMode: Image.PreserveAspectFit
        sourceSize.height: unit.dp(32)
        mipmap: true
    }

    Rectangle {
        anchors.fill: parent
        color: '#5f92eb'
        z: 1

        ColumnLayout {
            anchors.centerIn: parent

            spacing: unit.dp(16)
            width: unit.dp(240)

            Text {
                id: updatingLabel
                anchors.horizontalCenter: parent.horizontalCenter
                text: (processStatus == "")?"Checking for update...":(processStatus == "download")?"Downloading update...":(processStatus == "install")?"Installing update...":(processStatus == "error")?"Update failed...":""
                color: '#ffffff'

                font.family: openSansRegularFont.name
                font.pixelSize: unit.em(1.3)
            }

            ProgressBar {
                id: updatingProgress
                objectName: 'updatingProgress'
                anchors {left: parent.left; right: parent.right}

                minimumValue: 0
                maximumValue: 100
                value: 0

                style: ProgressBarStyle {
                    background: Rectangle {
                        color: 'transparent'

                        implicitHeight: unit.dp(8)
                    }
                    progress: Rectangle {
                        color: '#FFF'
                    }
                }
            }
        }
    }
}

