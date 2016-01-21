import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import "components"

ApplicationWindow {
    id: mainWindow
    visible: true
    minimumWidth: unit.dp(640)
    minimumHeight: unit.dp(480)
    color: "#f1f1f1"

    Units {
        id: unit
    }

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

    GridLayout {
        anchors.fill: parent
        columnSpacing: 0
        rowSpacing: 0
        flow: GridLayout.LeftToRight
        columns: 2

        Rectangle {
            id: connectedDevicesLabel
            implicitWidth: childrenRect.width + unit.dp(16)
            implicitHeight: connectedDevicesLabelText.height + unit.dp(22)
            color: "#21242b"

            Text {
                id: connectedDevicesLabelText
                color: "#ffffff"
                text: qsTr("Connected Devices")
                font.pixelSize: unit.em(1.3)
                font.family: openSansRegularFont.name
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: unit.dp(8)
            }
        }


        Rectangle {
            id: currentDeviceRect

            Layout.fillWidth: true
            implicitHeight: connectedDevicesLabel.height

            color: "#5f92eb"

            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: unit.dp(8)
                anchors.rightMargin: anchors.leftMargin
                spacing: anchors.leftMargin

                Text {
                    id: currentDeviceLabel
                    color: "#ffffff"
                    font.family: openSansRegularFont.name
                    font.pixelSize: unit.em(1.3)
                    text: "Device"
                }

                Text {
                    id: currentDevicePointLabel
                    color: "#ffffff"
                    font.family: openSansRegularFont.name
                    font.pixelSize: currentDeviceLabel.font.pixelSize
                    text: ">"
                }

                Text {
                    id: currentDeviceNameLabel
                    objectName: "currentDeviceNameLabel"
                    color: "#ffffff"
                    font.family: openSansRegularFont.name
                    font.pixelSize: currentDeviceLabel.font.pixelSize
                    text: "None"
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Image {
                        id: currentDeviceRectLogo
                        anchors.right: parent.right
                        height: parent.height
                        source: "qrc:/res/images/logo.png"
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                    }
                }
            }
        }

        Rectangle {
            id: connectedDevicesListRectangle
            implicitWidth: connectedDevicesLabel.width
            Layout.fillHeight: true
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
                        id: deviceInfoWrapper
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: deviceInfo.height + unit.dp(18)
                        x: 0
                        color: "#00ffffff"

                        Behavior on color {
                            ColorAnimation {}
                        }

                        Text {
                            id: deviceInfo

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: unit.dp(8)
                            anchors.verticalCenter: parent.verticalCenter

                            text: name
                            color: "white"
                            font.pixelSize: unit.em(1.1)
                            font.family: openSansRegularFont.name
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
                                    target: deviceInfoWrapper
                                    color: "#33ffffff"
                                }
                                PropertyChanges {
                                    target: deviceInfo
                                    anchors.leftMargin: unit.dp(12)
                                }
                            },
                            State {
                                name: "click"
                                when: mouser.pressed

                                PropertyChanges {
                                    target: deviceInfoWrapper
                                    color: "#33000000"
                                }
                                PropertyChanges {
                                    target: deviceInfo
                                    anchors.leftMargin: unit.dp(12)
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
                        height: deviceListDelegate.height
                    }
                }

                model: devicesModel
                focus: true
            }
        }

        Item {
            id: mainPageRect
            Layout.fillWidth: true
            Layout.fillHeight: true
            z: 2

            Loader {
                signal viewChanged()
                property string currentView

                id: mainPageLoader
                source: "main_provisionmode.qml"
                objectName: "mainPageLoader"

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: mainPageBottomTabRect.top

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
                height: currentDeviceRect.height * 0.7
                width: height
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: mainPageBottomTabRect.top
                z: 3

                Image {
                    id: mainPageBottomTabArrow
                    anchors.fill: parent
                    anchors.margins: unit.dp(4)
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

            RectangularGlow {
                id: mainPageBottomTabShadow
                anchors.fill: mainPageBottomTabRect
                color: "#30000000"
                glowRadius: unit.dp(4)
                cornerRadius: mainPageBottomTabRect.radius + glowRadius
                spread: 0.1
                z: 1
            }

            Rectangle {
                id: mainPageBottomTabRect
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: mainPageBottomTab.childrenRect.height + (unit.dp(8) * 2)
                Layout.maximumHeight: mainPageBottomTab.childrenRect.height + (unit.dp(8) * 2)
                color: "white"
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
                    anchors.margins: unit.dp(8)
                    flow: GridLayout.TopToBottom
                    rows: 2
                    columnSpacing: 0
                    rowSpacing: 0

                    DeviceInfoBox {
                        id: currentDeviceMake
                        header: "Make"
                        value: "Samsung"
                    }

                    DeviceInfoBox {
                        id: currentDeviceESN
                        header: "ESN"
                        value: "80512b74"
                    }

                    DeviceInfoBox {
                        id: currentDeviceModel
                        header: "Model"
                        value: "GT-i9100"
                    }

                    DeviceInfoBox {
                        id: currentDeviceMEID
                        header: "MEID"
                        value: "99000033839416"
                    }

                    DeviceInfoBox {
                        id: currentDeviceMIN
                        header: "MIN"
                        value: "0000000000"
                    }

                    DeviceInfoBox {
                        id: currentDeviceIMEI
                        Layout.columnSpan: 2
                        header: "IMEI"
                        value: "99000033839416"
                    }

                    DeviceInfoBox {
                        id: currentDeviceMDN
                        header: "MDN"
                        value: "0000000000"
                    }
                }
            }
        }
    }
}

