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
        } else if(mainPageLoader.currentView == "deviceinfo") {
            this.title = qsTr("UPST - Device info");
        } else {
            this.title = qsTr("UPST")
        }
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    FontLoader {
        id: openSansBoldFont;
        source: "qrc:/res/fonts/OpenSans-Bold.ttf"
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
                id: advancedMenuDeviceInfo
                text: qsTr("Device info")
                onTriggered: {
                    mainPageLoader.currentView = "deviceinfo"
                }
            }
            MenuItem {
                id: advancedMenuProvision
                text: qsTr("Provision mode")
                onTriggered: {
                    mainPageLoader.currentView = "provision"
                }
            }
            MenuItem {
                id: advancedMenuManual
                text: qsTr("Manual mode")
                onTriggered: {
                    mainPageLoader.currentView = "manual"
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
            anchors.left: parent.left
            anchors.top: parent.top
            Layout.minimumWidth: Math.max(connectedDevicesListRectangle.width, connectedDevicesLabelText.width + connectedDevicesLabelIcon.width + unit.dp(24))
            implicitHeight: connectedDevicesLabelText.height + unit.dp(32)
            color: "#21242b"
            z: 5

            Image {
                id: connectedDevicesLabelIcon

                anchors.left: parent.left
                anchors.leftMargin: unit.dp(8)
                anchors.verticalCenter: parent.verticalCenter

                source: "qrc:/res/images/icons/computer.svg"
                fillMode: Image.PreserveAspectFit
                mipmap: true
                sourceSize.height: connectedDevicesLabelText.height
            }

            ColorOverlay {
                anchors.fill: connectedDevicesLabelIcon
                source: connectedDevicesLabelIcon
                color: "#ffffff"
            }

            Text {
                id: connectedDevicesLabelText
                color: "#ffffff"
                text: qsTr("Connected Devices")
                font.pixelSize: unit.em(1.35)
                font.family: openSansRegularFont.name
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: connectedDevicesLabelIcon.right
                anchors.leftMargin: unit.dp(8)
            }
        }


        Rectangle {
            id: currentDeviceRect

            Layout.fillWidth: true
            implicitHeight: connectedDevicesLabel.height

            color: "#5f92eb"
            z: 5

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
                    text: qsTr("None")

                    function reset() {
                        text = qsTr("None")
                    }
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
            width: connectedDevicesList.contentItem.childrenRect.width
            Layout.minimumWidth: connectedDevicesLabel.Layout.minimumWidth
            Layout.fillHeight: true
            color: "#232b2e"
            z: 4

            ListView {
                signal currentIndexChanged(int index)

                id: connectedDevicesList
                objectName: "connectedDevicesList"
                anchors.fill: parent
                orientation: ListView.Vertical
                currentIndex: 0

                function updateWidth() {
                    for(var i = 0; i < contentItem.children.length; i++) {
                        var w = contentItem.children[i].width;
                        if(w < width) {
                            contentItem.children[i].width = width;
                        }
                    }
                }

                onCurrentItemChanged: {
                    this.currentIndexChanged(this.currentIndex)
                }

                onCountChanged: {
                    updateWidth();
                }

                onWidthChanged: {
                    updateWidth();
                }

                delegate: Component {
                    id: deviceListDelegate
                    Rectangle {
                        objectName: "deviceInfoWrapper"
                        id: deviceInfoWrapper
                        anchors.left: parent.left
                        Layout.fillWidth: true
                        width: deviceInfoIcon.width + deviceInfoText.width + unit.dp(28)
                        height: deviceInfo.height + unit.dp(18)
                        x: 0
                        color: "#00ffffff"

                        Behavior on color {
                            ColorAnimation {}
                        }

                        RowLayout {
                            id: deviceInfo

                            anchors.left: parent.left
                            anchors.leftMargin: unit.dp(8)
                            anchors.right: parent.right
                            anchors.rightMargin: unit.dp(8)
                            anchors.verticalCenter: parent.verticalCenter

                            spacing: 0
                            clip: true

                            Image {
                                id: deviceInfoIcon

                                Layout.fillHeight: true
                                Layout.rowSpan: 2

                                source: "qrc:/res/images/icons/phone.svg"
                                fillMode: Image.PreserveAspectFit
                                mipmap: true
                                sourceSize.height: deviceInfoText.height
                            }

                            ColorOverlay {
                                anchors.fill: deviceInfoIcon
                                source: deviceInfoIcon
                                color: "#ffffff"
                            }

                            Column {
                                id: deviceInfoText

                                anchors.left: deviceInfoIcon.right
                                anchors.leftMargin: unit.dp(8)

                                Text {
                                    id: deviceInfoName

                                    text: name
                                    color: "white"
                                    font.pixelSize: unit.em(1.1)
                                    font.family: openSansRegularFont.name
                                    clip: true
                                }

                                Text {
                                    id: deviceInfoPort

                                    text: port
                                    color: "white"
                                    font.pixelSize: unit.em(1.1)
                                    font.family: openSansBoldFont.name
                                    font.bold: true
                                    clip: true
                                }
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
                        width: connectedDevicesListRectangle.width
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
                    } else if(this.currentView === "deviceinfo") {
                        this.source = "main_deviceinfo.qml"
                    }

                    updateTitle()

                    this.viewChanged()
                }

                Component.onCompleted: {
                    mainPageLoader.currentView = "provision"
                }
            }

            Rectangle {
                id: noDeviceOverlay
                objectName: "noDeviceOverlay"
                anchors.fill: mainPageLoader

                color: "#f1f1f1"
                visible: true
                opacity: 1.0

                Text {
                    anchors.centerIn: parent

                    text: qsTr("No devices attached")
                    font.family: openSansRegularFont.name
                    font.pixelSize: unit.em(1.4)
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        onRunningChanged: {
                            if(running && noDeviceOverlay.opacity < 1.0) {
                                noDeviceOverlay.visible = true;
                            } else if(!running && noDeviceOverlay.opacity == 0.0) {
                                noDeviceOverlay.visible = false;
                            }
                        }
                    }
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
                property bool enabled
                property bool hidden

                objectName: "mainPageBottomTabArrow"
                id: mainPageBottomTabArrowRect
                height: currentDeviceRect.height * 0.7
                width: height
                color: "white"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: mainPageBottomTabRect.top
                z: 3
                enabled: false
                hidden: true

                onEnabledChanged: {
                    if(!enabled) {
                        hidden = true;
                    } else {
                        hidden = hidden;
                    }
                }

                onHiddenChanged: {
                    if(!enabled) {
                        return;
                    }

                    if(hidden) {
                        mainPageBottomTabRect.state = 'hidden';
                        mainPageBottomTabArrow.state = 'hidden';
                    } else {
                        mainPageBottomTabRect.state = '';
                        mainPageBottomTabArrow.state = '';
                    }
                }

                Image {
                    id: mainPageBottomTabArrow
                    anchors.fill: parent
                    anchors.margins: unit.dp(4)
                    source: "qrc:/res/images/up_arrow.png"
                    smooth: true
                    mipmap: true
                    rotation: 180
                    state: "hidden"

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
                    cursorShape: mainPageBottomTabArrowRect.enabled?Qt.PointingHandCursor:Qt.ArrowCursor

                    onClicked: {
                        mainPageBottomTabArrowRect.hidden = !mainPageBottomTabArrowRect.hidden;
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
                state: "hidden"

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
                        objectName: "currentDeviceMake"
                        header: qsTr("Make")
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceESN
                        objectName: "currentDeviceESN"
                        header: "ESN"
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceModel
                        objectName: "currentDeviceModel"
                        header: "Model"
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceMEID
                        objectName: "currentDeviceMEID"
                        header: "MEID"
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceMIN
                        objectName: "currentDeviceMIN"
                        header: "MIN"
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceIMEI
                        objectName: "currentDeviceIMEI"
                        Layout.columnSpan: 2
                        header: "IMEI"
                        value: ""
                    }

                    DeviceInfoBox {
                        id: currentDeviceMDN
                        objectName: "currentDeviceMDN"
                        header: "MDN"
                        value: ""
                    }
                }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.columnSpan: 2

            height: statusBarText.height + unit.dp(16)

            z: 10

            color: '#5f92eb'

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: unit.dp(8)

                id: statusBarText
                objectName: "statusBarText"
                color: "#ffffff"
                font.family: openSansRegularFont.name
                font.pixelSize: unit.em(1.3)
                text: "STATUS"
            }
        }
    }
}

