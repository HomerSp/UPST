import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0
import "qrc:/res/qml/components"

Window {
    id: deviceGuidewindow
    visible: false

    width: unit.dp(990)
    height: unit.dp(540)
    minimumHeight: unit.dp(540)

    flags: Qt.Window

    modality: Qt.NonModal
    title: ""

    Settings {
        category: "deviceguide"
        property alias x: deviceGuidewindow.x
        property alias y: deviceGuidewindow.y
        property alias width: deviceGuidewindow.width
        property alias height: deviceGuidewindow.height
    }

    Units {
        id: unit
    }

    FontLoader {
        id: openSansRegularFont;
        source: "qrc:/res/fonts/OpenSans-Regular.ttf"
    }

    function deviceChanged(index) {
        deviceGuide.text = deviceGuideListModel.deviceData(index);
    }

    GridLayout {
        anchors.fill: parent
        columns: 2
        flow: GridLayout.LeftToRight

        rowSpacing: 0
        columnSpacing: 0

        Rectangle {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            anchors.left: parent.left
            anchors.top: parent.top

            height: unit.dp(75)

            z: 2

            color: "#1976D2"

            Image {
                id: logoImage
                anchors.right: parent.right
                anchors.rightMargin: unit.dp(16)
                anchors.verticalCenter: parent.verticalCenter

                height: parent.height / 2

                source: "qrc:/res/images/logo.svg"
                fillMode: Image.PreserveAspectFit
                mipmap: true
            }

            ColumnLayout {
                Layout.fillWidth: true

                anchors.left: parent.left
                anchors.leftMargin: unit.dp(16)
                anchors.right: logoImage.left
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "UPST"
                    color: "#FFF"

                    font.pixelSize: unit.em(1.4)
                    font.family: openSansRegularFont.name
                }

                Text {
                    id: deviceGuideHeaderDevice
                    text: "Instructions - None"
                    color: "#FFF"

                    font.pixelSize: unit.em(1.3)
                    font.family: openSansRegularFont.name
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true

            id: deviceGuidesListRectangle
            implicitWidth: unit.dp(150)
            color: "#232b2e"
            z: 1

            ScrollView {
                id: deviceGuidesListScroll
                anchors.fill: parent

                implicitWidth: deviceGuidesListRectangle.implicitWidth

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

                ListView {
                    property bool scrollbarSet: false

                    signal refresh()
                    signal currentIndexChanged(int index)

                    id: deviceGuideList
                    objectName: "deviceGuideList"
                    anchors.fill: parent

                    implicitWidth: deviceGuidesListScroll.implicitWidth

                    orientation: ListView.Vertical
                    currentIndex: deviceGuideListModel.startIndex()

                    onCurrentItemChanged: {
                        deviceGuideHeaderDevice.text = "Instructions - " + this.currentItem.name
                        deviceChanged(this.currentIndex)
                    }

                    function updateWidth() {
                        var largestWidth = 0;
                        for(var i = 0; i < contentItem.children.length; i++) {
                            if(contentItem.children[i].realWidth > largestWidth) {
                                largestWidth = contentItem.children[i].realWidth;
                            }
                        }

                        for(i = 0; i < contentItem.children.length; i++) {
                            if(largestWidth > contentItem.children[i].implicitWidth) {
                                contentItem.children[i].implicitWidth = largestWidth;
                            }
                        }

                        largestWidth += (deviceGuidesListScroll.childrenRect.width - deviceGuidesListScroll.contentItem.width);
                        if(largestWidth > deviceGuidesListRectangle.implicitWidth) {
                            deviceGuidesListRectangle.implicitWidth = largestWidth;
                        }
                    }

                    Component.onCompleted: {
                        positionViewAtBeginning();

                        updateWidth();
                    }

                    delegate: Component {
                        id: deviceGuideListDelegate
                        Rectangle {
                            property variant realWidth
                            property string name: deviceMake + " " + deviceModel + " (" + deviceCodename + ")"

                            objectName: "deviceGuideListInfoWrapper"
                            id: deviceGuideListInfoWrapper

                            anchors.left: parent.left
                            realWidth: deviceGuideListInfoContainer.width
                            implicitWidth: realWidth
                            height: deviceGuideListInfoContainer.height
                            x: 0
                            color: "#00ffffff"

                            enabled: !deviceHeader

                            Component.onCompleted: {
                                deviceGuideList.updateWidth();
                            }

                            Behavior on color {
                                ColorAnimation {}
                            }

                            Item {
                                id: deviceGuideListInfoContainer

                                implicitWidth: deviceGuidesListInfoIcon.implicitWidth + deviceGuidesListInfoText.implicitWidth + unit.dp(40)
                                height: deviceGuidesListInfo.height + unit.dp(16)

                                RowLayout {
                                    id: deviceGuidesListInfo

                                    anchors.left: parent.left
                                    anchors.leftMargin: unit.dp(8)
                                    anchors.right: parent.right
                                    anchors.rightMargin: unit.dp(8)
                                    anchors.verticalCenter: parent.verticalCenter

                                    spacing: 0
                                    clip: true

                                    Image {
                                        id: deviceGuidesListInfoIcon

                                        Layout.fillHeight: true
                                        Layout.rowSpan: 2

                                        source: deviceIcon
                                        fillMode: Image.PreserveAspectFit
                                        mipmap: true
                                        sourceSize.height: deviceGuidesListInfoText.height
                                    }

                                    ColorOverlay {
                                        anchors.fill: deviceGuidesListInfoIcon
                                        source: deviceGuidesListInfoIcon
                                        color: "#ffffff"
                                    }

                                    Column {
                                        id: deviceGuidesListInfoText

                                        anchors.left: deviceGuidesListInfoIcon.right
                                        anchors.leftMargin: unit.dp(8)

                                        width: Math.max(deviceGuidesListInfoName.contentWidth, deviceGuidesListInfoCodename.contentWidth)

                                        Text {
                                            id: deviceGuidesListInfoName
                                            text: deviceMake + " " + deviceModel
                                            color: "#ffffff"
                                            font.pixelSize: unit.em(1.1)
                                            font.family: openSansRegularFont.name
                                            clip: true
                                        }

                                        Text {
                                            id: deviceGuidesListInfoCodename

                                            text: deviceCodename
                                            color: "#ffffff"
                                            font.pixelSize: unit.em(1.0)
                                            font.family: openSansBoldFont.name
                                            font.bold: true
                                            clip: true
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                id: mouser
                                anchors.fill: parent
                                onClicked: deviceGuideList.currentIndex = index
                                hoverEnabled: true
                            }

                            states: [
                                State {
                                    name: "normal"
                                    when: !mouser.containsMouse
                                    PropertyChanges {
                                        target: deviceGuideListInfoWrapper
                                        color: "#00ffffff"
                                    }
                                    PropertyChanges {
                                        target: deviceGuidesListInfo
                                        anchors.leftMargin: unit.dp(8)
                                    }
                                },
                                State {
                                    name: "hover"
                                    when: mouser.containsMouse && !mouser.pressed && !deviceHeader

                                    PropertyChanges {
                                        target: deviceGuideListInfoWrapper
                                        color: "#33ffffff"
                                    }
                                    PropertyChanges {
                                        target: deviceGuidesListInfo
                                        anchors.leftMargin: unit.dp(12)
                                    }
                                },
                                State {
                                    name: "click"
                                    when: mouser.pressed && !deviceHeader

                                    PropertyChanges {
                                        target: deviceGuideListInfoWrapper
                                        color: "#33000000"
                                    }
                                    PropertyChanges {
                                        target: deviceGuidesListInfo
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
                            width: deviceGuidesListRectangle.width
                            height: deviceGuideListDelegate.height
                        }
                    }

                    model: deviceGuideListModel
                    focus: true
                }
            }
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: deviceGuide
            objectName: "deviceGuide"

            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

            textFormat: TextEdit.RichText
            wrapMode: TextEdit.Wrap
            readOnly: true

            text: ""

            font.family: openSansRegularFont.name
            font.pixelSize: unit.em(1.2)
        }
    }
}
