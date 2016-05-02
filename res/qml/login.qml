import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "components"

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

    Image {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: unit.dp(8)
        anchors.topMargin: unit.dp(8)

        source: "qrc:/res/images/logo.png"
        fillMode: Image.PreserveAspectFit
        sourceSize.height: unit.dp(32)
        mipmap: true
    }

    Column {
        anchors.centerIn: parent
        spacing: unit.dp(12)

        Text {
            id: usernameLabel
            anchors.horizontalCenter: parent.horizontalCenter

            text: "Username"

            color: '#FFF'

            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.2)
        }

        TextField {
            id: usernameText
            objectName: "usernameText"

            anchors.horizontalCenter: parent.horizontalCenter

            height: usernameLabel.height + unit.dp(12)

            font.family: openSansRegularFont.name
            font.pixelSize: usernameLabel.font.pixelSize

            Keys.onReturnPressed: {
                passwordText.forceActiveFocus();
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter

            text: "Password"

            color: '#FFF'

            font.family: openSansRegularFont.name
            font.pixelSize: usernameLabel.font.pixelSize
        }

        TextField {
            id: passwordText
            objectName: "passwordText"

            anchors.horizontalCenter: parent.horizontalCenter

            height: usernameLabel.height + unit.dp(12)

            font.family: openSansRegularFont.name
            font.pixelSize: usernameLabel.font.pixelSize

            echoMode: TextInput.Password

            Keys.onReturnPressed: {
                loginButton.clicked();
            }
        }

        UPButton {
            id: loginButton
            objectName: "loginButton"

            anchors.horizontalCenter: parent.horizontalCenter

            width: usernameText.width / 2
            height: usernameText.height

            text: "Login"

            scheme: "dark"
        }
    }
}
