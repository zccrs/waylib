import QtQuick
import QtQuick.Controls
import Waylib.Server

Item {
    WaylandServer {
        id: server

        WaylandBackend {
            id: backend

            onOutputAdded: function(output) {
                outputModel.add({waylandOutput: output})
            }
            onOutputRemoved: function(output) {
                outputModel.removeIf(function(props) {
                    return props.waylandOutput === output
                })
            }

            onInputAdded: function(inputDevice) {
                seat0.addDevice(inputDevice)
            }
            onInputRemoved: function(inputDevice) {
                seat0.removeDevice(inputDevice)
            }
        }

        WaylandCompositor {
            id: compositor
            backend: backend
        }

        Seat {
            id: seat0
            name: "seat0"
            cursor: Cursor {
                layout: outputLayout
            }
        }
    }

    OutputLayout {
        id: outputLayout
    }

    DynamicCreator {
        id: outputModel
    }

    OutputRenderWindow {
        compositor: compositor
        width: outputRowLayout.implicitWidth
        height: outputRowLayout.implicitHeight

        Row {
            id: outputRowLayout

            DynamicCreatorComponent {
                creator: outputModel
                OutputPositioner {
                    required property WaylandOutput waylandOutput

                    layout: outputLayout
                    output: waylandOutput

                    OutputViewport {
                        output: waylandOutput
                        anchors.centerIn: parent
                    }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            text: "Hello XUPT"
            font.pointSize: 40
        }

        Button {
            text: "Quit"
            onClicked: Qt.quit()
            anchors {
                right: parent.right
                bottom: parent.bottom
                margins: 10
            }
        }
    }
}

//// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
//// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//import QtQuick
//import QtQuick.Controls
//import QtQuick.Layouts
//import Waylib.Server
//import Tinywl

//Item {
//    id :root

//    WaylandServer {
//        id: server

//        WaylandBackend {
//            id: backend

//            onOutputAdded: function(output) {
//                output.forceSoftwareCursor = true // Test

//                if (QmlHelper.outputManager.count > 0)
//                    output.scale = 2

//                Helper.allowNonDrmOutputAutoChangeMode(output)
//                QmlHelper.outputManager.add({waylandOutput: output})
//            }
//            onOutputRemoved: function(output) {
//                QmlHelper.outputManager.removeIf(function(prop) {
//                    return prop.waylandOutput === output
//                })
//            }
//            onInputAdded: function(inputDevice) {
//                seat0.addDevice(inputDevice)
//            }
//            onInputRemoved: function(inputDevice) {
//                seat0.removeDevice(inputDevice)
//            }
//        }

//        WaylandCompositor {
//            id: compositor

//            backend: backend
//        }

//        XdgShell {
//            id: shell

//            onSurfaceAdded: function(surface) {
//                let type = surface.isPopup ? "popup" : "toplevel"
//                QmlHelper.xdgSurfaceManager.add({type: type, waylandSurface: surface})
//            }
//            onSurfaceRemoved: function(surface) {
//                QmlHelper.xdgSurfaceManager.removeIf(function(prop) {
//                    return prop.waylandSurface === surface
//                })
//            }
//        }

//        Seat {
//            id: seat0
//            name: "seat0"
//            cursor: Cursor {
//                id: cursor1

//                layout: QmlHelper.layout
//            }

//            eventFilter: Helper
//            keyboardFocus: Helper.getFocusSurfaceFrom(renderWindow.activeFocusItem)
//        }

//        WaylandSocket {
//            id: masterSocket

//            freezeClientWhenDisable: false

//            Component.onCompleted: {
//                console.info("Listing on:", socketFile)
//                Helper.startDemoClient(socketFile)
//            }
//        }

//        // TODO: add attached property for XdgSurface
//        XdgDecorationManager {
//            id: decorationManager
//        }

//        XWayland {
//            compositor: compositor.compositor
//            seat: seat0.seat
//            lazy: true

//            onReady: masterSocket.addClient(client())

//            onSurfaceAdded: function(surface) {
//                QmlHelper.xwaylandSurfaceManager.add({waylandSurface: surface})
//            }
//            onSurfaceRemoved: function(surface) {
//                QmlHelper.xwaylandSurfaceManager.removeIf(function(prop) {
//                    return prop.waylandSurface === surface
//                })
//            }
//        }
//    }

//    OutputRenderWindow {
//        id: renderWindow

//        compositor: compositor
//        width: outputRowLayout.implicitWidth + outputRowLayout.x
//        height: outputRowLayout.implicitHeight + outputRowLayout.y

//        EventJunkman {
//            anchors.fill: parent
//        }

//        Row {
//            id: outputRowLayout

//            DynamicCreatorComponent {
//                creator: QmlHelper.outputManager

//                OutputDelegate {
//                    property real topMargin: topbar.height
//                }
//            }
//        }

//        ColumnLayout {
//            anchors.fill: parent

//            TabBar {
//                id: topbar

//                Layout.fillWidth: true

//                TabButton {
//                    text: qsTr("Stack Layout")
//                    onClicked: {
//                        decorationManager.mode = XdgDecorationManager.PreferClientSide
//                    }
//                }
//                TabButton {
//                    text: qsTr("Tiled Layout")
//                    onClicked: {
//                        decorationManager.mode = XdgDecorationManager.PreferServerSide
//                    }
//                }
//            }

//            Item {
//                Layout.fillWidth: true
//                Layout.fillHeight: true

//                StackWorkspace {
//                    visible: topbar.currentIndex === 0
//                    anchors.fill: parent
//                }

//                TiledWorkspace {
//                    visible: topbar.currentIndex === 1
//                    anchors.fill: parent
//                }
//            }
//        }
//    }
//}
