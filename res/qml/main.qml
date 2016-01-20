import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

ApplicationWindow {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    minimumWidth: 800
    minimumHeight: 480
    color: "#f1f1f1"

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

                        states: [
                            State {
                                name: "hover"
                                when: mouser.containsMouse && !mouser.pressed

                                PropertyChanges {
                                    target: wrapper
                                    color: "#33ffffff"
                                }
                                PropertyChanges {
                                    target: textContainer
                                    anchors.leftMargin: 10
                                }
                            },
                            State {
                                name: "click"
                                when: mouser.pressed

                                PropertyChanges {
                                    target: wrapper
                                    color: "#33000000"
                                }
                                PropertyChanges {
                                    target: textContainer
                                    anchors.leftMargin: 10
                                }
                            }
                        ]

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
            mipmap: true
        }
    }

    Item {
        id: columnLayout1
        anchors.left: connectedDevices.right
        anchors.top: currentDeviceRect.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 1

        RectangularGlow {
            id: mainPageBottomTabShadow
            anchors.fill: mainPageBottomTabRect
            color: "#30000000"
            glowRadius: 4
            cornerRadius: mainPageBottomTabRect.radius + glowRadius
            spread: 0.1
            z: 1
        }

        Rectangle {
            id: mainPageBottomTabRect
            height: 100
            Layout.maximumHeight: 100
            Layout.alignment: Qt.AlignBottom
            color: "white"
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            z: 3

            states: [
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: mainPageBottomTabRect
                        height: 0
                    }
                }
            ]
            Behavior on height { SpringAnimation { spring: 2; damping: 0.4; duration: 300 } }

            GridLayout {
                id: mainPageBottomTab
                anchors.fill: parent
                anchors.margins: 6
                flow: GridLayout.TopToBottom
                rows: 2
                columnSpacing: 0
                rowSpacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "Make"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "Samsung"
                        color: "#000"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "Model"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "GT-i9100"
                        color: "#000"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "MIN"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "0000000000"
                        color: "#000"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "MDN"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "0000000000"
                        color: "#000"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "ESN"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "80512b74"
                        color: "#000"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "IMEI/MEID"
                        color: "#FFF"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f1f1f1"

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        font.family: openSansRegularFont.name
                        text: "99000033839416"
                        color: "#000"
                    }
                }
            }
        }

        Item {
            id: mainPageRect
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: mainPageBottomTabRect.top
            anchors.top: parent.top
            z: 2

            Loader {
                signal viewChanged()
                property string currentView

                id: mainPageLoader
                source: "main_provisionmode.qml"
                objectName: "mainPageLoader"

                anchors.fill: parent

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

            RectangularGlow {
                anchors.fill: mainPageBottomTabArrowRect
                color: mainPageBottomTabShadow.color
                glowRadius: mainPageBottomTabShadow.glowRadius
                cornerRadius: mainPageBottomTabArrowRect.radius + glowRadius
                spread: mainPageBottomTabShadow.spread
                z: 1
            }

            Rectangle {
                id: mainPageBottomTabArrowRect
                width: 30
                height: 30
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                z: 3

                Image {
                    id: mainPageBottomTabArrow
                    anchors.fill: parent
                    anchors.margins: 5
                    source: "qrc:/res/images/up_arrow.png"
                    smooth: true
                    mipmap: true
                    rotation: 180

                    states: [
                        State {
                            name: "hidden"
                            PropertyChanges {
                                target: mainPageBottomTabArrow
                                rotation: 0
                            }
                        }
                    ]
                    Behavior on rotation { SpringAnimation { spring: 2; damping: 0.4; duration: 200 } }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        mainPageBottomTabRect.state == 'hidden' ? mainPageBottomTabRect.state = '' : mainPageBottomTabRect.state = 'hidden'
                        mainPageBottomTabArrow.state == 'hidden' ? mainPageBottomTabArrow.state = '' : mainPageBottomTabArrow.state = 'hidden'
                    }
                }
            }
        }
    }
}

