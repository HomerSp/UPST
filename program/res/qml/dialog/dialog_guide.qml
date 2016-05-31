import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import "qrc:/res/qml/components"

Window {
    property string name
    property string guideUrl
    property string guideData

    id: guideWindow
    visible: false

    width: unit.dp(990)
    height: unit.dp(540)
    minimumHeight: unit.dp(540)

    flags: Qt.Window

    modality: Qt.NonModal
    title: ""

    Settings {
        category: name
        property alias x: guideWindow.x
        property alias y: guideWindow.y
        property alias width: guideWindow.width
        property alias height: guideWindow.height
    }

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    TextArea {
        id: userguideLayout
        anchors.fill: parent
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap
        readOnly: true

        text: guideData
        baseUrl: guideUrl

        font.family: openSansRegularFont.name
        font.pixelSize: unit.em(1.2)

        Component.onCompleted: {
            guideWindow.minimumWidth = unit.dp(990) + (width - contentItem.width);
            guideWindow.maximumWidth = guideWindow.minimumWidth;
        }

        onLinkActivated: {
            var data = downloader.download(guideUrl + link);
            userguideLayout.text = data;
        }
    }
}

