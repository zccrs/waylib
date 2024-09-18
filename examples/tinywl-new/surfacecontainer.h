// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include "surfacewrapper.h"

#include <QQuickItem>
#include <QAbstractListModel>
#include <QProperty>
#include <QPropertyNotifier>

#include <functional>

class SurfaceListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit SurfaceListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    virtual void addSurface(SurfaceWrapper *surface);
    virtual void removeSurface(SurfaceWrapper *surface);
    bool hasSurface(SurfaceWrapper *surface) const;
    const QList<SurfaceWrapper*> &surfaces() const {
        return m_surfaces;
    }

signals:
    void surfaceAdded(SurfaceWrapper *surface);
    void surfaceRemoved(SurfaceWrapper *surface);

private:
    QList<SurfaceWrapper*> m_surfaces;
};

class SurfaceFilterModel : public SurfaceListModel
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit SurfaceFilterModel(SurfaceListModel *parent);

    inline SurfaceListModel *parent() const {
        auto op = QObject::parent();
        auto p = qobject_cast<SurfaceListModel*>(op);
        Q_ASSERT(p);
        return p;
    }
    void setFilter(std::function<bool(SurfaceWrapper*)> filter);

private:
    using SurfaceListModel::addSurface;
    using SurfaceListModel::removeSurface;

    void initForSourceSurface(SurfaceWrapper *surface);
    void makeBindingForSourceSurface(SurfaceWrapper *surface);
    void updateSurfaceVisibility(SurfaceWrapper *surface);

    std::function<bool(SurfaceWrapper*)> m_filter;

    struct Property {
        Property()
            : prop(false)
        {

        }

        template <typename F>
        inline void setBinding(F f) {
            prop.setBinding(f);
        }

        bool init = false;
        QProperty<bool> prop;
        QPropertyNotifier notifier;
    };

    std::map<SurfaceWrapper*, Property> m_properties;
};

class Output;
class RootSurfaceContainer;
class SurfaceContainer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(SurfaceListModel* model READ model CONSTANT FINAL)
    QML_ANONYMOUS

public:
    explicit SurfaceContainer(QQuickItem *parent = nullptr);
    explicit SurfaceContainer(SurfaceContainer *parent);
    ~SurfaceContainer() override;

    RootSurfaceContainer *rootContainer() const;
    SurfaceContainer *parentContainer() const;
    QList<SurfaceContainer*> subContainers() const;
    void setQmlEngine(QQmlEngine *engine);

    virtual void addSurface(SurfaceWrapper *surface);
    virtual void removeSurface(SurfaceWrapper *surface);

    virtual void addOutput(Output *output);
    virtual void removeOutput(Output *output);

    const QList<SurfaceWrapper*> &surfaces() const {
        return m_model->surfaces();
    }

    SurfaceListModel *model() const {
        return m_model;
    }

signals:
    void surfaceAdded(SurfaceWrapper *surface);
    void surfaceRemoved(SurfaceWrapper *surface);

protected:
    friend class SurfaceWrapper;

    void geometryChange(const QRectF &newGeo, const QRectF &oldGeo) override;

    bool doAddSurface(SurfaceWrapper *surface, bool setContainer);
    bool doRemoveSurface(SurfaceWrapper *surface, bool setContainer);

    virtual void addBySubContainer(SurfaceContainer *sub, SurfaceWrapper *surface);
    virtual void removeBySubContainer(SurfaceContainer *sub, SurfaceWrapper *surface);

    virtual bool filterSurfaceGeometryChanged(SurfaceWrapper *surface, QRectF &newGeometry, const QRectF &oldGeometry);
    virtual bool filterSurfaceStateChange(SurfaceWrapper *surface, SurfaceWrapper::State newState, SurfaceWrapper::State oldState);

    SurfaceListModel *m_model = nullptr;
};
