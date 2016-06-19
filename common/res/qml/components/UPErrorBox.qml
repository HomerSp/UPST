import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

Rectangle {
    property bool closeable
    property string header
    property string errorLine1
    property string errorLine2
    property string errorLine3

    property color headerColor
    property color headerTextColor
    property color errorTextColor

    signal closed()

    id: root
    anchors.fill: parent
    z: 100

    color: '#33000000'
    visible: false
    opacity: (errorLine2.length > 0)?1.0:0.0

    closeable: true

    header: ""
    errorLine1: ""
    errorLine2: ""
    errorLine3: ""

    headerColor: '#5f92eb'
    headerTextColor: "#ffffff"
    errorTextColor: '#ffffff'

    function hide() {
        root.opacity = 0.0;
    }

    function show() {
        root.opacity = 1.0;
    }

    function showError(error) {
        root.errorLine2 = (error.length > 0)?error:"";
        root.opacity = 1.0;
    }

    onClosed: {
        root.opacity = 0.0;
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

    Behavior on opacity {
       NumberAnimation {
           duration: 300
           onRunningChanged: {
               if(running && root.opacity < 1.0) {
                   root.visible = true;
                   root.forceActiveFocus();
               } else if(!running && root.opacity === 0.0) {
                   root.visible = false;
                   root.errorLine2 = "";
               }
           }
       }
    }

    MouseArea {
       anchors.fill: parent

       z: 101

       hoverEnabled: true
    }


    Column {
       anchors.fill: parent
       anchors.margins: unit.dp(8)
       z: 102

       opacity: 1.0

       Rectangle {
           id: headerContainer
           anchors {left: parent.left; right: parent.right}
           height: headerCloseButton.height + unit.dp(16)
           color: headerColor

           RowLayout {
               anchors.fill: parent
               anchors.margins: {left: unit.dp(8); right: unit.dp(8)}

               Text {
                   Layout.fillWidth: true
                   id: headerContentText
                   color: headerTextColor
                   font.family: openSansBoldFont.name
                   font.pixelSize: unit.em(1.2)
                   text: header
               }

               UPButton {
                   id: headerCloseButton
                   text: "X"
                   height: headerContentText.height
                   width: height
                   buttonColor: headerColor
                   visible: closeable?true:false

                   onClicked: {
                       closed();
                   }
               }
           }
       }

       Rectangle {
           anchors {left: parent.left; right: parent.right}
           height: errorLine1Text.height + errorLine2Text.height + errorLine3Text.height + unit.dp(32)
           color: '#232b2e'

           ColumnLayout {
               id: container
               anchors.fill: parent
               anchors.margins: unit.dp(16)
               height: parent.height
               spacing: 0

               TextArea {
                   id: errorLine1Text
                   anchors {left: parent.left; right: parent.right}
                   implicitHeight: (errorLine1.length > 0)?contentHeight:0
                   textColor: "#ffffff"
                   font.family: openSansRegularFont.name
                   font.pixelSize: unit.em(1.2)
                   text: errorLine1
                   visible: errorLine1.length > 0
                   wrapMode: Text.Wrap
                   readOnly: true
                   backgroundVisible: false
                   frameVisible: false
               }
               TextArea {
                   id: errorLine2Text
                   anchors {left: parent.left; right: parent.right}
                   implicitHeight: (errorLine2.length > 0)?contentHeight:0
                   textColor: errorTextColor
                   font.family: openSansBoldFont.name
                   font.pixelSize: unit.em(1.2)
                   font.weight: Font.Bold
                   text: errorLine2
                   visible: errorLine2.length > 0
                   wrapMode: Text.Wrap
                   readOnly: true
                   backgroundVisible: false
                   frameVisible: false
               }
               TextArea {
                   id: errorLine3Text
                   anchors {left: parent.left; right: parent.right}
                   implicitHeight: (errorLine3.length > 0)?contentHeight:0
                   textColor: "#ffffff"
                   font.family: openSansRegularFont.name
                   font.pixelSize: unit.em(1.2)
                   text: errorLine3
                   visible: errorLine3.length > 0
                   wrapMode: Text.Wrap
                   readOnly: true
                   backgroundVisible: false
                   frameVisible: false
               }
           }
       }
    }
}
