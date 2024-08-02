// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wglobal.h>
#include <wqmlcreator.h>

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlComponent>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

WAYLIB_SERVER_BEGIN_NAMESPACE
class WServer;
class WOutputRenderWindow;
class WOutputLayout;
class WCursor;
class WSeat;
class WBackend;
class WOutputItem;
class WOutputViewport;
class WOutputLayer;
class WOutput;
class WXWayland;
class WInputMethodHelper;
class WXdgDecorationManager;
class WSocket;
WAYLIB_SERVER_END_NAMESPACE

QW_BEGIN_NAMESPACE
class qw_renderer;
class qw_allocator;
class qw_compositor;
QW_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE
QW_USE_NAMESPACE

class Q_DECL_HIDDEN Helper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Helper(QObject *parent = nullptr);
    ~Helper();

    void init();

private:
    WOutputItem *createOutputItem(WOutput *output) const;

    void allowNonDrmOutputAutoChangeMode(WOutput *output);
    void enableOutput(WOutput *output);

    int indexOfOutput(WOutput *output) const;
    Output *getOutput(WOutput *output) const;
    void setOutputPosition(Output *output, const QPoint &newPosition);
    void moveOutput(Output *output, const QPoint &requestPosition);

    void setOutputProxy(Output *output);

    // qml data
    QQmlApplicationEngine m_qmlEngine;
    QQmlComponent m_outputDelegate;
    QQmlComponent m_copyOutputDelegate;
    QQmlComponent m_xdgSurfaceDelegate;
    QQmlComponent m_xwaylandSurfaceDelegate;
    QQmlComponent m_layerSurfaceDelegate;

    // qtquick helper
    WOutputRenderWindow *m_renderWindow = nullptr;
    WOutputLayout *m_outputLayout = nullptr;

    // wayland helper
    WServer *m_server = nullptr;
    WSocket *m_socket = nullptr;
    WCursor *m_cursor = nullptr;
    WSeat *m_seat = nullptr;
    WBackend *m_backend = nullptr;
    qw_renderer *m_renderer = nullptr;
    qw_allocator *m_allocator = nullptr;

    // protocols
    qw_compositor *m_compositor = nullptr;
    WXWayland *m_xwayland = nullptr;
    WInputMethodHelper *m_inputMethodHelper = nullptr;
    WXdgDecorationManager *m_xdgDecorationManager = nullptr;

    // private data
    enum class OutputType {
        Primary,
        Proxy
    };

    struct Output {
        OutputType type;
        WOutput *output;
        WOutputItem *item;
        Output *proxy = nullptr;
    };
    QList<Output> m_outputList;
};
