import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtWebKit 3.0
import "qrc:/res/qml/components"

Dialog {
    id: userguideDialog
    visible: false
    height: unit.dp(620)
    width: unit.dp(990)

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

    contentItem: Rectangle {
        color: '#FFFFFF'

        ScrollView {
            anchors {left: parent.left; top: parent.top; bottom: parent.bottom}
            implicitWidth: unit.dp(990)

            WebView {
                id: userguideDialogContent
                anchors.fill: parent
            }
        }
    }

    function setData(data, url) {
        userguideDialogContent.loadHtml(data, url);
        /*userguideDialogContent.baseUrl = url;
        userguideDialogContent.text = data;*/
    }

    onRejected: {
       close()
    }
}

