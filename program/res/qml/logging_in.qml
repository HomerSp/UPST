import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "qrc:/res/qml/components"

Rectangle {
    id: loginView
    objectName: "loginView"

    color: '#5f92eb'

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    Text {
        anchors.centerIn: parent

        color: '#FFF'

        font.family: openSansRegularFont.name
        font.pixelSize: unit.em(1.3)

        text: "Logging in..."
    }
}
