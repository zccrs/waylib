// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import QtQuick
import Waylib.Server

Item {
    property WaylandSocket masterSocket
    property DynamicCreator layerSurfaceManager: layerSurfaceManager
    property DynamicCreator inputPopupSurfaceManager: inputPopupSurfaceManager

    function printStructureObject(obj) {
        var json = ""
        for (var prop in obj){
            if (!obj.hasOwnProperty(prop)) {
                continue;
            }
            let value = obj[prop]
            try {
                json += `    ${prop}: ${value},\n`
            } catch (err) {
                json += `    ${prop}: unknown,\n`
            }
        }

        return '{\n' + json + '}'
    }

    DynamicCreator {
        id: layerSurfaceManager
        onObjectAdded: function(delegate, obj, properties) {
            console.info(`New Layer surface item ${obj} from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }

        onObjectRemoved: function(delegate, obj, properties) {
            console.info(`Layer surface item ${obj} is removed, it's create from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }
    }

    DynamicCreator {
        onObjectAdded: function(delegate, obj, properties) {
            console.info(`New X11 surface item ${obj} from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }

        onObjectRemoved: function(delegate, obj, properties) {
            console.info(`X11 surface item ${obj} is removed, it's create from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }
    }

    DynamicCreator {
        id: inputPopupSurfaceManager
        onObjectAdded: function (delegate, obj, properties) {
            console.info(`New input popup surface item ${obj} from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }

        onObjectRemoved: function (delegate, obj, properties) {
            console.info(`Input popup surface item ${obj} is removed, it's create from delegate ${delegate} with initial properties:`,
                         `\n${printStructureObject(properties)}`)
        }
    }
}
