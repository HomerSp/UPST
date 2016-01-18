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
}

