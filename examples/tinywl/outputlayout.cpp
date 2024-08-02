// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "outputlayout.h"

#include <woutput.h>

OutputLayout::OutputLayout(QObject *parent)
    : WOutputLayout(parent)
{

}

QPoint OutputLayout::add(WOutput *output, const QPoint &expectPos)
{
    QPoint pos = QPoint(0, 0);

    if (region.isEmpty()) {
        region += QRect(pos, output->effectiveSize());
    } else {
        pos = expectPos;
        tryMove(output, pos);
    }

    WOutputLayout::add(output, pos);
    return pos;
}

void OutputLayout::remove(WOutput *output)
{
    WOutputLayout::remove(output);
}

QList<std::pair<WOutput*, QPoint>> OutputLayout::tryMove(WOutput *output, QPoint &pos) const
{

}

QPoint OutputLayout::move(WOutput *output, const QPoint &newPos)
{
    QPoint pos = newPos;
    tryMove(output, pos);
    WOutputLayout::move(output, pos);
    return pos;
}
