/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"

#include "Framework.h"
#include "SceneManager.h"
#include "ECEditorWindow.h"
#include "UiServiceInterface.h"

using namespace Scene;

SceneStructureWindow::SceneStructureWindow(Foundation::Framework *fw) : framework(fw)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(200,300);

    treeWidget = new SceneTreeWidget(fw, this);
    layout->addWidget(treeWidget);
}

SceneStructureWindow::~SceneStructureWindow()
{
    Clear();
}

void SceneStructureWindow::SetScene(const Scene::ScenePtr &s)
{
    scene = s;
    SceneManager *scenePtr = scene.lock().get();
    assert(scenePtr);
    connect(scenePtr, SIGNAL(EntityCreated(Scene::Entity *, AttributeChange::Type)), SLOT(AddEntity(Scene::Entity *)));
    connect(scenePtr, SIGNAL(EntityRemoved(Scene::Entity *, AttributeChange::Type)), SLOT(RemoveEntity(Scene::Entity *)));
    Populate();
}

void SceneStructureWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange)
        setWindowTitle(tr("Scene Structure"));
    else
        QWidget::changeEvent(e);
}

void SceneStructureWindow::Populate()
{
    ScenePtr s = scene.lock();
    if (!s)
    {
        // warning print
        return;
    }

    SceneManager::iterator it = s->begin();
    while(it != s->end())
    {
        AddEntity((*it).get());
        ++it;
    }
}

void SceneStructureWindow::Clear()
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        SAFE_DELETE(item);
    }
}

void SceneStructureWindow::AddEntity(Scene::Entity* entity)
{
    SceneTreeWidgetItem *item = new SceneTreeWidgetItem(entity->GetId());
    item->setText(0, QString("%1 %2").arg(entity->GetId()).arg(entity->GetName()));
    // Set local entity's font color blue
    if (entity->GetId() & Scene::LocalEntity)
        item->setTextColor(0, QColor(Qt::blue));
    treeWidget->addTopLevelItem(item);
}

void SceneStructureWindow::RemoveEntity(Scene::Entity* entity)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        SceneTreeWidgetItem *item = static_cast<SceneTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item && (item->id == entity->GetId()))
        {
            SAFE_DELETE(item);
            break;
        }
    }
}
