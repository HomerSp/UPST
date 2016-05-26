import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "qrc:/res/qml/components"

Rectangle {
    id: loggingInView
    objectName: "loggingInView"

    color: '#5f92eb'

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    Image {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: unit.dp(8)
        anchors.topMargin: unit.dp(8)

        source: "qrc:/res/images/logo.svg"
        fillMode: Image.PreserveAspectFit
        sourceSize.height: unit.dp(32)
        mipmap: true
    }

    Column {
        anchors.centerIn: parent
        spacing: unit.dp(16.0)

        Text {
            id: loggingInLabel
            objectName: 'loggingInLabel'

            color: '#FFF'

            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.3)

            horizontalAlignment: Text.Center

            text: qsTr("Logging in...")
        }

        UPButton {
            id: updateFailedContinueButton
            objectName: "updateFailedContinueButton"
            anchors.horizontalCenter: parent.horizontalCenter

            width: loggingInLabel.width / 2
            height: unit.dp(32)

            visible: false

            text: "Continue"

            scheme: "dark"
        }
    }

    function setCheckForUpdates(b) {
        if(b) {
            if(updateFailed) {
                loggingInLabel.text = qsTr("The update failed.\nPlease report this to Ultimobile.");
                updateFailedContinueButton.visible = true;
            } else {
                loggingInLabel.text = qsTr("Checking for updates...");
                updateFailedContinueButton.visible = false;
            }
        } else {
            loggingInLabel.text = qsTr("Logging in...");
        }
    }
}
