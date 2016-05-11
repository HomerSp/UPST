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

    Text {
        id: loggingInLabel
        objectName: 'loggingInLabel'
        anchors.centerIn: parent

        color: '#FFF'

        font.family: openSansRegularFont.name
        font.pixelSize: unit.em(1.3)

        text: qsTr("Logging in...")
    }

    function setCheckForUpdates(b) {
        if(b) {
            loggingInLabel.text = qsTr("Checking for updates...");
        } else {
            loggingInLabel.text = qsTr("Logging in...");
        }
    }
}
