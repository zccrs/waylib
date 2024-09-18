// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Tinywl

Item {
    id: root

    required property Item target
    required property QtObject output

    anchors.fill: parent

    WorkspaceProxy {
        workspace: target
        output: root.output
    }
}
