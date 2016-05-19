import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import "qrc:/res/qml/components"

ApplicationWindow {
    id: mainWindow
    visible: true
    minimumWidth: unit.dp(640)
    minimumHeight: unit.dp(480)
    color: "#f1f1f1"
    title: qsTr("UPST") + " v" + programVersion

    Units {
        id: unit
    }

    function updateTitle() {
        var t = qsTr("UPST") + " v" + programVersion;
        if(mainPageLoader.currentView == "manual") {
            this.title = t + qsTr(" - Manual mode")
        } else if(mainPageLoader.currentView == "deviceinfo") {
            this.title = t + qsTr(" - Device info");
        } else {
            this.title = t
        }
    }

    Component.onCompleted: {
        updateTitle();
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
                objectName: "fileMenuLogout"
                text: qsTr("Logout")
                visible: false
            }

            MenuItem {
                text: qsTr("Exit")
                onTriggered: {
                    Qt.quit()
                }
            }
        }

        Menu {
            objectName: "editMenu"
            title: qsTr("Edit")
            visible: false
            MenuItem {
                text: qsTr("Refresh")
                onTriggered: {
                    connectedDevicesList.refresh();
                }
            }
        }

        Menu {
            id: advancedMenu
            objectName: "advancedMenu"
            title: qsTr("Advanced")
            visible: false
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

            MenuItem {
                id: advancedMenuLog
                text: qsTr("Log")
                onTriggered: {
                    var component = Qt.createComponent("dialog/dialog_log.qml");
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(mainWindow);
                        dialog.open();
                    } else {
                        console.error("Could not load log dialog: " + component.errorString());
                    }
                }
            }
        }

        Menu {
            id: helpMenu
            title: qsTr("Help")
            MenuItem {
                id: helpMenuUserguide
                text: qsTr("User guide")
                onTriggered: {
                    var component = Qt.createComponent("dialog/dialog_userguide.qml");
                    if (component.status === Component.Ready) {
                        var url = "http://upst.ultimobile.net/userguide/";
                        var data = downloader.download(url + "/index.html");
                        var dialog = component.createObject(mainWindow, {"guideData": data, "guideUrl": url});
                        dialog.open();
                    } else {
                        console.error("Could not load user guide dialog: " + component.errorString());
                    }
                }
            }
            MenuItem {
                id: helpMenuAbout
                text: qsTr("About")
                onTriggered: {
                    var component = Qt.createComponent("dialog/dialog_about.qml");
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(mainWindow);
                        dialog.modality = Qt.ApplicationModal;
                        dialog.open();
                    } else {
                        console.error("Could not load about dialog: " + component.errorString());
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
                font.pixelSize: unit.em(1.2)
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
                    font.pixelSize: unit.em(1.2)
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
                        source: "qrc:/res/images/logo.svg"
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
                signal refresh()
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

                    currentItem.updateProgress();
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
                        property bool isProvisioned
                        property bool haveError
                        property bool haveChildren
                        property int progressCurrent
                        property int progressMax
                        property int progressMin
                        property int progressStatus
                        property string progressError

                        objectName: "deviceInfoWrapper"
                        id: deviceInfoWrapper
                        anchors.left: parent.left
                        Layout.fillWidth: true
                        width: deviceInfoContainer.width + (deviceInfoText.height / 2)
                        height: deviceInfoContainer.height + deviceInfoStatusProgress.height
                        x: 0
                        color: "#00ffffff"

                        isProvisioned: progressStatus == 2
                        haveError: progressError.length > 0
                        haveChildren: deviceChildrenSize > 0
                        progressCurrent: provisionProgressCurrent
                        progressMax: provisionProgressMax
                        progressMin: provisionProgressMin
                        progressStatus: provisionProgressStatus
                        progressError: provisionProgressError

                        onProgressCurrentChanged: {
                            deviceInfoStatusProgress.value = progressCurrent;
                        }

                        Behavior on color {
                            ColorAnimation {}
                        }

                        Item {
                            id: deviceInfoContainer

                            width: deviceInfoIcon.width + deviceInfoText.width + unit.dp(40)
                            height: deviceInfo.height + unit.dp(16)

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

                                    source: deviceIcon
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

                                        text: deviceName
                                        color: (deviceAvailable)?"#ffffff":"#eeeeee"
                                        font.pixelSize: unit.em(1.1)
                                        font.family: openSansRegularFont.name
                                        clip: true
                                    }

                                    Text {
                                        id: deviceInfoPort

                                        text: devicePort
                                        color: deviceInfoName.color
                                        font.pixelSize: unit.em(1.0)
                                        font.family: openSansBoldFont.name
                                        font.bold: true
                                        clip: true
                                    }
                                }
                            }
                        }

                        Image {
                            id: deviceInfoStatus

                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: unit.dp(8)

                            opacity: (progressStatus > 0)?1.0:0.0

                            source: "qrc:/res/images/icons/" + ((progressStatus == 4)?"queued":(progressStatus == 3)?"failed":(progressStatus == 2)?(deviceFlagManualReboot?"restart":"done"):"download") + ".svg"
                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                            sourceSize.height: deviceInfoText.height / 2

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
                                }
                            }
                        }

                        ProgressBar {
                            id: deviceInfoStatusProgress
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom

                            minimumValue: progressMin
                            maximumValue: progressMax
                            value: 0

                            opacity: (progressStatus > 0)?1.0:0.0

                            style: ProgressBarStyle {
                                background: Rectangle {
                                    color: 'transparent'

                                    implicitHeight: unit.dp(4)
                                }
                                progress: Rectangle {
                                    color: '#FFF'
                                }
                            }

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
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

                        onIsProvisionedChanged: {
                            updateProgress();
                        }
                        onHaveErrorChanged: {
                            updateProgress();
                        }
                        onHaveChildrenChanged: {
                            updateProgress();
                        }

                        function updateProgress() {
                            if(haveError && connectedDevicesList.currentIndex == index) {
                                provisionFailedContainer.show(progressError);
                                return;
                            } else {
                                provisionFailedContainer.hide();
                            }

                            if(isProvisioned && connectedDevicesList.currentIndex == index) {
                                if(deviceFlagManualReboot) {
                                    provisionSuccessContainer.show(qsTr("Please reboot the phone manually to finish the process."));
                                } else {
                                    provisionSuccessContainer.show();
                                }
                                return;
                            } else {
                                provisionSuccessContainer.hide();
                            }

                            if(deviceFlagMultiPort && connectedDevicesList.currentIndex == index && !haveChildren) {
                                deviceWaitingPortContainer.show();
                                return;
                            } else {
                                deviceWaitingPortContainer.hide();
                            }
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

            UPErrorBox {
                objectName: 'provisionFailedContainer'
                id: provisionFailedContainer

                headerColor: '#915858'
                headerTextColor: '#FFFFFF'
                header: "Failed to provision device"
                errorLine1: qsTr("An error has ocurred while provisioning the device:")
                errorLine3: qsTr("A log has been sent to Ultimobile automatically,\nif you require immediate assistance, please call\nour support team at 720-433-3028.")
                errorTextColor: '#EF9A9A'
            }

            UPErrorBox {
                objectName: 'provisionSuccessContainer'
                id: provisionSuccessContainer

                headerColor: '#729158'
                headerTextColor: '#FFFFFF'
                header: "Device has been provisioned"
                errorLine1: qsTr("The device was provisioned successfully!")
                errorTextColor: '#C5E1A5'
            }

            UPErrorBox {
                id: deviceWaitingPortContainer

                closeable: false
                headerColor: '#a78b4e'
                headerTextColor: '#FFFFFF'
                header: "Please wait"
                errorLine1: qsTr("Awaiting connection...")
            }

           Rectangle {
               id: noDeviceOverlay
               objectName: "noDeviceOverlay"
               anchors.fill: parent

               color: "#f1f1f1"
               visible: true
               opacity: 1.0

               z: 999

               Text {
                   anchors.centerIn: parent

                   text: qsTr("No devices attached")
                   font.family: openSansRegularFont.name
                   font.pixelSize: unit.em(1.2)
               }

               MouseArea {
                   anchors.fill: parent
                   hoverEnabled: true
               }

               Behavior on opacity {
                   NumberAnimation {
                       duration: 300
                       onRunningChanged: {
                           if(running && noDeviceOverlay.opacity < 1.0) {
                               noDeviceOverlay.visible = true;
                           } else if(!running && noDeviceOverlay.opacity === 0.0) {
                               noDeviceOverlay.visible = false;
                           }
                       }
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
                font.pixelSize: unit.em(1.2)
                text: "STATUS"
            }
        }
    }

    Rectangle {
        id: loginOverlay
        objectName: "loginOverlay"
        anchors.fill: parent

        color: "#f1f1f1"
        visible: true
        opacity: 1.0

        z: 1000

        function show() {
            visible = true;
            opacity = 1.0;
            loginOverlayAnimation.complete();
        }
        function hide() {
            visible = false;
            opacity = 0.0;
            loginOverlayAnimation.complete();
        }

        onOpacityChanged: {
            if(opacity == 1.0) {
                loggingInOverlay.opacity = 0.0;
            }
        }

        Loader {
            anchors.fill: parent
            z: 1001

            source: "login.qml"
        }

        MouseArea {
            z: 1000

            anchors.fill: parent
            hoverEnabled: true
        }

        Behavior on opacity {
            NumberAnimation {
                id: loginOverlayAnimation
                duration: 300
                onRunningChanged: {
                    if(running && loginOverlay.opacity < 1.0) {
                        loginOverlay.visible = true;
                    } else if(!running && loginOverlay.opacity === 0.0) {
                        loginOverlay.visible = false;
                    }
                }
            }
        }
    }

    Rectangle {
        id: loggingInOverlay
        objectName: "loggingInOverlay"
        anchors.fill: parent

        color: "#f1f1f1"
        visible: userTokenSet
        opacity: (userTokenSet)?1.0:0.0

        z: 1002

        onOpacityChanged: {
            if(opacity == 1.0) {
                loginOverlay.hide();
            } else if(opacity == 0.0) {
                loggingInLoader.item.setCheckForUpdates(false);
            }
        }

        Component.onCompleted: {
            if(userTokenSet) {
                loginOverlay.hide();
            }
        }

        Loader {
            id: loggingInLoader
            anchors.fill: parent
            z: 1001

            source: "logging_in.qml"
        }

        MouseArea {
            z: 1000

            anchors.fill: parent
            hoverEnabled: true
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 300
                onRunningChanged: {
                    if(running && loggingInOverlay.opacity < 1.0) {
                        loggingInOverlay.visible = true;
                    } else if(!running && loggingInOverlay.opacity === 0.0) {
                        loggingInOverlay.visible = false;
                    }
                }
            }
        }
    }

    UPErrorBox {
        anchors.fill: loginOverlay
        z: 1003

        id: loginFailedContainer
        objectName: 'loginFailedContainer'

        header: qsTr("Could not login")
    }

}

