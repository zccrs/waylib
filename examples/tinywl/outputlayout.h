// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include <woutputlayout.h>

#include <QRegion>

WAYLIB_SERVER_USE_NAMESPACE

class OutputLayout : public WOutputLayout
{
public:
    OutputLayout(QObject *parent = nullptr);

    QPoint add(WOutput *output, const QPoint &expectPos);
    void remove(WOutput *output);

    QList<std::pair<WOutput*, QPoint>> tryMove(WOutput *output, QPoint &pos) const;
    QPoint move(WOutput *output, const QPoint &newPos);

private:
    QRegion region;
};
