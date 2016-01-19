import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

ApplicationWindow {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    minimumWidth: 640
    minimumHeight: 480

    function updateTitle() {
        if(mainPageLoader.currentView == "manual") {
            this.title = qsTr("UPST - Manual mode")
        } else {
            this.title = qsTr("UPST")
        }
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Exit")
                onTriggered: {
                    Qt.quit()
                }
            }
        }

        Menu {
            id: advancedMenu
            objectName: "advancedMenu"
            title: qsTr("Advanced")
            MenuItem {
                id: advancedMenuManual
                text: qsTr("Manual mode")
                onTriggered: {
                    if(mainPageLoader.currentView === "provision") {
                        this.text = qsTr("Provision mode")
                        mainPageLoader.currentView = "manual"
                    } else if(mainPageLoader.currentView === "manual") {
                        this.text = qsTr("Manual mode")
                        mainPageLoader.currentView = "provision"
                    }
                }
            }
        }
    }

    Rectangle {
        id: connectedDevices
        width: (childrenRect.width < 200)?200:childrenRect.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            id: connectedDevicesLabel
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            color: "#21242b"

            Text {
                color: "#ffffff"
                text: qsTr("Connected Devices")
                font.pointSize: 11
                font.family: openSansRegularFont.name
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }
        }

        Rectangle {
            id: connectedDevicesListRectangle
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: connectedDevicesLabel.bottom
            anchors.bottom: parent.bottom
            color: "#232b2e"

            ListView {
                signal currentIndexChanged(int index)

                id: connectedDevicesList
                objectName: "connectedDevicesList"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.top: parent.top

                onCurrentItemChanged: {
                    this.currentIndexChanged(this.currentIndex)
                }

                delegate: Component {
                    id: deviceListDelegate
                    Rectangle {
                        id: wrapper
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 50
                        x: 0
                        color: "#00ffffff"

                        Behavior on color {
                            ColorAnimation {}
                        }

                        Rectangle {
                            id: textContainer
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            height: childrenRect.height
                            color: "transparent"

                            Text {
                                id: contactInfo
                                text: name
                                color: "white"
                                font.pointSize: 9
                                font.family: openSansRegularFont.name
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                            }
                        }

                        MouseArea {
                            id: mouser
                            anchors.fill: parent
                            onClicked: connectedDevicesList.currentIndex = index
                            hoverEnabled: true
                        }

                        states: State {
                            name: "BackgroundState"
                            when: mouser.containsMouse

                            PropertyChanges {
                                target: wrapper
                                color: "#33ffffff"
                            }
                            PropertyChanges {
                                target: textContainer
                                anchors.leftMargin: 10
                            }
                        }

                        transitions: Transition {
                            NumberAnimation { properties: "anchors.leftMargin"; duration: 100 }
                        }
                    }
                }
                highlight: Component {
                    Rectangle {
                        color: "#5f92eb"
                        width: parent.width
                        height: 70
                    }
                }

                model: devicesModel
                focus: true
            }
        }
    }

    Rectangle {
        id: currentDeviceRect
        anchors.left: connectedDevices.right
        anchors.right: parent.right
        anchors.top: parent.top
        height: 50
        color: "#5f92eb"

        Text {
            id: currentDeviceLabel
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            font.family: openSansRegularFont.name
            font.pointSize: 11
            text: "Device"
        }

        Text {
            id: currentDevicePointLabel
            anchors.left: currentDeviceLabel.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            font.family: openSansRegularFont.name
            font.pointSize: 11
            text: ">"
        }

        Text {
            id: currentDeviceNameLabel
            objectName: "currentDeviceNameLabel"
            anchors.left: currentDevicePointLabel.right
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            font.family: openSansRegularFont.name
            font.pointSize: 11
            text: "None"
        }

        Image {
            id: currentDeviceRectLogo
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 15
            width: 20
            source: "qrc:/res/images/logo.png"
            fillMode: Image.PreserveAspectFit
        }
    }

    Loader {
        signal viewChanged()
        property string currentView

        id: mainPageLoader
        source: "main_provisionmode.qml"
        objectName: "mainPageLoader"
        anchors.left: connectedDevices.right
        anchors.top: currentDeviceRect.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentView: ""

        onCurrentViewChanged: {
            if(this.currentView === "manual") {
                this.source = "main_manualmode.qml"
            } else if(this.currentView === "provision") {
                this.source = "main_provisionmode.qml"
            }

            updateTitle()

            this.viewChanged()
        }

        Component.onCompleted: {
            mainPageLoader.currentView = "provision"
        }
    }
}

