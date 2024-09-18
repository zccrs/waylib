// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include "surfacecontainer.h"

class SurfaceWrapper;
class Workspace;
class WorkspaceContainer : public SurfaceContainer
{
    friend class Workspace;
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit WorkspaceContainer(Workspace *parent);
};

class Workspace : public SurfaceContainer
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged FINAL)
    Q_PROPERTY(WorkspaceContainer* currentworkspace READ currentworkspace NOTIFY currentChanged FINAL)
    QML_ANONYMOUS

public:
    explicit Workspace(SurfaceContainer *parent);

    void addSurface(SurfaceWrapper *surface, int workspaceIndex = -1);
    void removeSurface(SurfaceWrapper *surface) override;
    int containerIndexOfSurface(SurfaceWrapper *surface) const;

    int createContainer(bool visible = false);
    void removeContainer(int index);
    WorkspaceContainer *container(int index) const;

    int currentIndex() const;
    void setCurrentIndex(int newCurrentIndex);
    void switchToNext();
    void switchToPrev();
    void switchTo(int index);

    WorkspaceContainer *currentworkspace() const;

signals:
    void currentChanged();

private:
    void updateSurfaceOwnsOutput(SurfaceWrapper *surface);
    void updateSurfacesOwnsOutput();

    int m_currentIndex = 0;
    QList<WorkspaceContainer*> m_containers;
};
