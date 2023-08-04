// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import Waylib.Server
import Tinywl

Item {
    id :root

    WaylandServer {
        id: server

        WaylandBackend {
            id: backend

            onOutputAdded: function(output) {
                output.forceSoftwareCursor = true // Test

                if (outputModel.count > 0)
                    output.scale = 2

                outputModel.append({waylandOutput: output, outputLayout: layout})
            }
            onOutputRemoved: function(output) {

            }
            onInputAdded: function(inputDevice) {
                seat0.addDevice(inputDevice)
            }
        }

        WaylandCompositor {
            id: compositor

            backend: backend
        }

        XdgShell {
            id: shell

            onSurfaceAdded: function(surface) {
                xdgSurfaceManager.create(surface, {waylandSurface: surface});
            }

            onRequestMove: function(surface, seat, serial) {
                globalHelper.startMove(surface, seat, serial)
            }
        }

        Seat {
            id: seat0
            name: "seat0"
            cursor: Cursor {
                id: cursor1

                layout: layout
            }

            eventFilter: globalHelper
        }

        WaylandSocket {
            id: masterSocket

            freezeClientWhenDisable: false

            Component.onCompleted: {
                console.info("Listing on:", socketFile)
                globalHelper.startDemoClient(socketFile)
            }
        }
    }

    WaylibComponent {
        id: xdgSurfaceManager

        Connections {
            required property WaylandSurface waylandSurface

            target: waylandSurface

            function doDestroy() {
                for (let i = 0; i < renderWindow.toplevelWindowModel.count; ++i) {
                    if (renderWindow.toplevelWindowModel.get(i).waylandSurface === waylandSurface) {
                        renderWindow.toplevelWindowModel.remove(i)
                    }
                }

                subsurfaceManager.destroyIfOwnerIs(waylandSurface)
            }

            function onMappedChanged() {
                if (waylandSurface.mapped) {
                    if (waylandSurface.isSubsurface) {
                        subsurfaceManager.create(null, waylandSurface, {waylandSurface: target})
                    } else {
                        renderWindow.toplevelWindowModel.append({waylandSurface: target, outputLayout: layout})
                    }
                } else {
                    doDestroy()
                }
            }

            function onNewSubsurface(surface) {
                xdgSurfaceManager.create(surface, {waylandSurface: surface});
            }

            Component.onCompleted: {
                onMappedChanged()
                Qt.callLater(function() {
                    waylandSurface.subsurfaces.forEach(onNewSubsurface)
                })
            }
            Component.onDestruction: {
                doDestroy()
            }
        }
    }

    WaylibComponent {
        id: subsurfaceManager

        SurfaceItem {
            required property WaylandSurface waylandSurface

            surface: waylandSurface
        }
    }

    Helper {
        id: globalHelper
    }

    OutputLayout {
        id: layout
    }

    OutputRenderWindow {
        id: renderWindow

        property ListModel toplevelWindowModel: ListModel {}

        compositor: compositor
        width: outputRowLayout.implicitWidth + outputRowLayout.x
        height: outputRowLayout.implicitHeight + outputRowLayout.y

        Row {
            id: outputRowLayout

            Repeater {
                model: ListModel {
                    id: outputModel
                }

                OutputPositioner {
                    required property WaylandOutput waylandOutput
                    required property OutputLayout outputLayout

                    output: waylandOutput
                    devicePixelRatio: waylandOutput.scale
                    layout: outputLayout

                    OutputViewport {
                        id: outputViewport

                        output: waylandOutput
                        devicePixelRatio: parent.devicePixelRatio
                        anchors.centerIn: parent
                        cursorDelegate: Item {
                            required property OutputCursor cursor

                            visible: cursor.visible && !cursor.isHardwareCursor

                            Image {
                                source: cursor.imageSource
                                x: -cursor.hotspot.x
                                y: -cursor.hotspot.y
                                cache: false
                                width: cursor.size.width
                                height: cursor.size.height
                                sourceClipRect: cursor.sourceRect
                            }
                        }

                        RotationAnimation {
                            id: rotationAnimator

                            target: outputViewport
                            duration: 200
                            alwaysRunToEnd: true
                        }

                        Timer {
                            id: setTransform

                            property var scheduleTransform
                            onTriggered: waylandOutput.orientation = scheduleTransform
                            interval: rotationAnimator.duration / 2
                        }

                        function rotationOutput(orientation) {
                            setTransform.scheduleTransform = orientation
                            setTransform.start()

                            switch(orientation) {
                            case WaylandOutput.R90:
                                rotationAnimator.to = -90
                                break
                            case WaylandOutput.R180:
                                rotationAnimator.to = 180
                                break
                            case WaylandOutput.R270:
                                rotationAnimator.to = 90
                                break
                            default:
                                rotationAnimator.to = 0
                                break
                            }

                            rotationAnimator.from = rotation
                            rotationAnimator.start()
                        }
                    }

                    Image {
                        id: background
                        source: "file:///usr/share/backgrounds/deepin/desktop.jpg"
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        anchors.fill: parent
                    }

                    Column {
                        anchors {
                            bottom: parent.bottom
                            right: parent.right
                            margins: 10
                        }

                        spacing: 10

                        Switch {
                            text: "Socket"
                            onCheckedChanged: {
                                masterSocket.enabled = checked
                            }
                            Component.onCompleted: {
                                checked = masterSocket.enabled
                            }
                        }

                        Button {
                            text: "1X"
                            onClicked: {
                                waylandOutput.scale = 1
                            }
                        }

                        Button {
                            text: "1.5X"
                            onClicked: {
                                waylandOutput.scale = 1.5
                            }
                        }

                        Button {
                            text: "Normal"
                            onClicked: {
                                outputViewport.rotationOutput(WaylandOutput.Normal)
                            }
                        }

                        Button {
                            text: "R90"
                            onClicked: {
                                outputViewport.rotationOutput(WaylandOutput.R90)
                            }
                        }

                        Button {
                            text: "R270"
                            onClicked: {
                                outputViewport.rotationOutput(WaylandOutput.R270)
                            }
                        }

                        Button {
                            text: "Quit"
                            onClicked: {
                                Qt.quit()
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "Qt Quick in a texture"
                        font.pointSize: 40
                        color: "white"

                        SequentialAnimation on rotation {
                            id: ani
                            running: true
                            PauseAnimation { duration: 1500 }
                            NumberAnimation { from: 0; to: 360; duration: 5000; easing.type: Easing.InOutCubic }
                            loops: Animation.Infinite
                        }
                    }
                }
            }
        }

        Repeater {
            anchors.fill: parent
            clip: false

            model: renderWindow.toplevelWindowModel

            SurfaceItem {
                required property WaylandSurface waylandSurface
                required property OutputLayout outputLayout

                surface: waylandSurface
                visible: waylandSurface && waylandSurface.WaylandSocket.rootSocket.enabled
                x: waylandSurface.parentSurface ? implicitPosition.x : 0
                y: waylandSurface.parentSurface ? implicitPosition.y : 0
                z: focus ? OutputLayout.ActiveToplevelSurface : OutputLayout.ToplevelSurface

                onFocusChanged: {
                    if (typeof waylandSurface.setActivate === "function")
                        waylandSurface.setActivate(focus)
                }

                OutputLayoutItem {
                    anchors.fill: parent
                    layout: outputLayout

                    onEnterOutput: function(output) {
                        waylandSurface.enterOutput(output);
                    }
                    onLeaveOutput: function(output) {
                        waylandSurface.leaveOutput(output);
                    }
                }

                Component.onCompleted: {
                    waylandSurface.shell = this
                    if (waylandSurface.parentSurface)
                        parent = waylandSurface.parentSurface.shell
                    forceActiveFocus()
                }
                Component.onDestruction: {
                    if (waylandSurface)
                        waylandSurface.shell = null
                }
            }
        }
    }
}
