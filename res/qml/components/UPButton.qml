import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

Button {
    property string scheme

    scheme: "light"

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    style: ButtonStyle {
        background: Rectangle {
            implicitWidth: control.width
            implicitHeight: control.height
            border.width: control.activeFocus ? 2 : 0
            border.color: "#ccc"
            color: control.pressed ? (control.scheme === "light")?"#4c75bc":"#ddd" : (control.hovered ? (control.scheme === "light")?"#7fa8ef":"#eee" : (control.scheme === "light")?"#5f92eb":"#fff")
            radius: unit.dp(4)

            Behavior on color {
                ColorAnimation { duration: 100 }
            }

            transitions: Transition {
                NumberAnimation { properties: "border.width"; duration: 100 }
            }
        }
        label: Text {
            color: (control.scheme === "light")?"white":"black"
            text: control.text
            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.2)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}

