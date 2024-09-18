// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Tinywl

Rectangle {
    required property Item workspace
    required property QtObject output

    width: output.outputItem.width
    height: output.outputItem.height
    clip: true

    Repeater {
        model: workspace.model
        delegate: Loader {
            id: loader

            required property SurfaceWrapper surface

            x: surface.x - output.outputItem.x
            y: surface.y - output.outputItem.y
            active: surface.ownsOutput === otuput
                    && surface.surfaceState !== SurfaceWrapper.State.Minimized
            sourceComponent: SurfaceProxy {
                surface: loader.surface
            }

            Component.onCompleted: console.log("-----------------", surface)
        }
    }
}
