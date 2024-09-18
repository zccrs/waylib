// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "workspace.h"
#include "surfacewrapper.h"
#include "output.h"
#include "helper.h"
#include "rootsurfacecontainer.h"

WorkspaceContainer::WorkspaceContainer(Workspace *parent)
    : SurfaceContainer(parent)
{

}

Workspace::Workspace(SurfaceContainer *parent)
    : SurfaceContainer(parent)
{
    createContainer(true);
    createContainer(false);
}

void Workspace::addSurface(SurfaceWrapper *surface, int workspaceIndex)
{
    doAddSurface(surface, false);
    auto container = workspaceIndex >= 0
                         ? m_containers.at(workspaceIndex)
                         : m_containers.at(m_currentIndex);

    if (container->m_model->hasSurface(surface))
        return;

    for (auto c : std::as_const(m_containers)) {
        if (c == container)
            continue;
        if (c->surfaces().contains(surface)) {
            c->removeSurface(surface);
            break;
        }
    }

    container->addSurface(surface);
    if (!surface->ownsOutput())
        surface->setOwnsOutput(Helper::instance()->rootContainer()->primaryOutput());
}

void Workspace::removeSurface(SurfaceWrapper *surface)
{
    if (!doRemoveSurface(surface, false))
        return;

    for (auto container : std::as_const(m_containers)) {
        if (container->surfaces().contains(surface)) {
            container->removeSurface(surface);
            break;
        }
    }
}

int Workspace::containerIndexOfSurface(SurfaceWrapper *surface) const
{
    for (int i = 0; i < m_containers.size(); ++i) {
        if (m_containers.at(i)->m_model->hasSurface(surface))
            return i;
    }

    return -1;
}

int Workspace::createContainer(bool visible)
{
    m_containers.append(new WorkspaceContainer(this));
    m_containers.last()->setVisible(visible);
    return m_containers.size() - 1;
}

void Workspace::removeContainer(int index)
{
    if (m_containers.size() == 1)
        return;
    if (index < 0 || index >= m_containers.size())
        return;

    auto container = m_containers.at(index);
    m_containers.removeAt(index);
    m_currentIndex = qMin(m_currentIndex, m_containers.size() - 1);
    auto current = m_containers.at(m_currentIndex);

    const auto tmp = container->surfaces();
    for (auto s : tmp) {
        container->removeSurface(s);
        current->addSurface(s);
    }

    container->deleteLater();
    emit currentChanged();
}

WorkspaceContainer *Workspace::container(int index) const
{
    if (index < 0 || index >= m_containers.size())
        return nullptr;
    return m_containers.at(index);
}

int Workspace::currentIndex() const
{
    return m_currentIndex;
}

void Workspace::setCurrentIndex(int newCurrentIndex)
{
    if (newCurrentIndex < 0 || newCurrentIndex >= m_containers.size())
        return;

    if (m_currentIndex == newCurrentIndex)
        return;
    m_currentIndex = newCurrentIndex;

    for (int i = 0; i < m_containers.size(); ++i) {
        m_containers.at(i)->setVisible(i == m_currentIndex);
    }

    emit currentChanged();
}

void Workspace::switchToNext()
{
    if (m_currentIndex < m_containers.size() - 1)
        switchTo(m_currentIndex + 1);
}

void Workspace::switchToPrev()
{
    if (m_currentIndex > 0)
        switchTo(m_currentIndex - 1);
}

void Workspace::switchTo(int index)
{
    Q_ASSERT(index >= 0 && index < m_containers.size());
    auto container = m_containers.at(index);
    auto engine = Helper::instance()->qmlEngine();

    for (auto o : rootContainer()->outputs()) {
        auto switcher = engine->createWorkspaceSwitcher(this, container, o);
    }
}

WorkspaceContainer *Workspace::currentworkspace() const
{
    return m_containers.at(m_currentIndex);
}

void Workspace::updateSurfaceOwnsOutput(SurfaceWrapper *surface)
{
    auto outputs = surface->surface()->outputs();
    if (surface->ownsOutput() && outputs.contains(surface->ownsOutput()->output()))
        return;

    Output *output = nullptr;
    if (!outputs.isEmpty())
        output = Helper::instance()->getOutput(outputs.first());
    if (!output)
        output = Helper::instance()->rootContainer()->cursorOutput();
    if (!output)
        output = Helper::instance()->rootContainer()->primaryOutput();
    if (output)
        surface->setOwnsOutput(output);
}

void Workspace::updateSurfacesOwnsOutput()
{
    for (auto surface : this->surfaces()) {
        updateSurfaceOwnsOutput(surface);
    }
}
