import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtWebKit 3.0
import "qrc:/res/qml/components"

Dialog {
    property string guideUrl
    property string guideData

    id: userguideDialog
    visible: false
    width: unit.dp(1020)
    height: unit.dp(620)

    modality: Qt.ApplicationModal
    title: qsTr("User guide")
    standardButtons: StandardButton.Close

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    contentItem: TextArea {
        anchors.fill: parent

        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap

        text: guideData
        baseUrl: guideUrl

        font.family: openSansRegularFont.name
        font.pixelSize: unit.em(1.2)
    }

    onRejected: {
       close()
    }
}

