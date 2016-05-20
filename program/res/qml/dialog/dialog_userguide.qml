import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0
import "qrc:/res/qml/components"

Window {
    property string guideUrl
    property string guideData

    id: userguideDialog
    visible: false

    width: unit.dp(990)
    height: unit.dp(540)
    minimumHeight: unit.dp(540)

    modality: Qt.NonModal
    title: qsTr("User guide")

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
            userguideDialog.minimumWidth = unit.dp(990) + (width - contentItem.width);
            userguideDialog.maximumWidth = userguideDialog.minimumWidth;
        }
    }
}

