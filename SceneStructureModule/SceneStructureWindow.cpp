/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.cpp
 *  @brief  Window with tree view of contents of scene.
 *
 *          Detailed desc here.
 */

#include "StableHeaders.h"
#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"

#include "Framework.h"
#include "SceneManager.h"
#include "ECEditorWindow.h"
#include "UiServiceInterface.h"
#include "EC_Name.h"

using namespace Scene;

SceneStructureWindow::SceneStructureWindow(Foundation::Framework *fw) :
    framework(fw),
    showComponents(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(200,300);

    QCheckBox *cb = new QCheckBox(tr("Show components"), this);
    connect(cb, SIGNAL(toggled(bool)), this, SLOT(ShowComponents(bool)));
    layout->addWidget(cb);

    treeWidget = new SceneTreeWidget(fw, this);
    layout->addWidget(treeWidget);
}

SceneStructureWindow::~SceneStructureWindow()
{
    SetScene(ScenePtr());
}

void SceneStructureWindow::SetScene(const Scene::ScenePtr &s)
{
    if (!scene.expired() && (s == scene.lock()))
        return;

    if (s == 0)
    {
        disconnect(s.get());
        Clear();
        return;
    }

    scene = s;
    SceneManager *scenePtr = scene.lock().get();
    connect(scenePtr, SIGNAL(EntityCreated(Scene::Entity *, AttributeChange::Type)), SLOT(AddEntity(Scene::Entity *)));
    connect(scenePtr, SIGNAL(EntityRemoved(Scene::Entity *, AttributeChange::Type)), SLOT(RemoveEntity(Scene::Entity *)));
    connect(scenePtr, SIGNAL(ComponentAdded(Scene::Entity *, IComponent *, AttributeChange::Type)),
        SLOT(AddComponent(Scene::Entity *, IComponent *)));
    connect(scenePtr, SIGNAL(ComponentRemoved(Scene::Entity *, IComponent *, AttributeChange::Type)),
        SLOT(RemoveComponent(Scene::Entity *, IComponent *)));

    Populate();
}

void SceneStructureWindow::ShowComponents(bool show)
{
    showComponents = show;
    treeWidget->showComponents =show;
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        for(int j = 0; j < item->childCount(); ++j)
            item->child(j)->setHidden(!showComponents);
    }
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
    EntityTreeWidgetItem *item = new EntityTreeWidgetItem(entity->GetId());
    item->setText(0, QString("%1 %2").arg(entity->GetId()).arg(entity->GetName()));
    // Set local entity's font color blue
    if (entity->IsLocal())
        item->setTextColor(0, QColor(Qt::blue));
    treeWidget->addTopLevelItem(item);

    foreach(ComponentPtr c, entity->GetComponentVector())
        AddComponent(entity, c.get());
}

void SceneStructureWindow::RemoveEntity(Scene::Entity* entity)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityTreeWidgetItem *item = dynamic_cast<EntityTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item && (item->id == entity->GetId()))
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void SceneStructureWindow::AddComponent(Scene::Entity* entity, IComponent* comp)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityTreeWidgetItem *eItem = dynamic_cast<EntityTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->id == entity->GetId()))
        {
            ComponentTreeWidgetItem *cItem = new ComponentTreeWidgetItem(comp->TypeName(), comp->Name(), eItem);
            cItem->setText(0, QString("%1 %2").arg(comp->TypeName()).arg(comp->Name()));
            cItem->setHidden(!showComponents);
            eItem->addChild(cItem);

            // If name component exists, retrieve name from it. Also hook up change signal so that UI keeps synch with the name.
            if (comp->TypeName() == EC_Name::TypeNameStatic())
            {
                eItem->setText(0, QString("%1 %2").arg(entity->GetId()).arg(entity->GetName()));
                connect(comp, SIGNAL(OnAttributeChanged(IAttribute *, AttributeChange::Type)), SLOT(UpdateEntityName(IAttribute *)));
            }
        }
    }
}

void SceneStructureWindow::RemoveComponent(Scene::Entity* entity, IComponent* comp)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityTreeWidgetItem *eItem = dynamic_cast<EntityTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->id == entity->GetId()))
        {
            for (int j = 0; j < eItem->childCount(); ++j)
            {
                ComponentTreeWidgetItem *cItem = dynamic_cast<ComponentTreeWidgetItem *>(eItem->child(j));
                if (cItem && (cItem->typeName == comp->TypeName()) && (cItem->name == comp->Name()))
                {
                    eItem->removeChild(cItem);
                    SAFE_DELETE(cItem);
                    break;
                }
            }

            if (comp->TypeName() == EC_Name::TypeNameStatic())
                eItem->setText(0, QString("%1").arg(entity->GetId()));
        }
    }
}

void SceneStructureWindow::UpdateEntityName(IAttribute *attr)
{
    EC_Name *nameComp = dynamic_cast<EC_Name *>(sender());
    if (!nameComp || (attr != &nameComp->name) || (nameComp->GetParentEntity() == 0))
        return;

    Entity *entity = nameComp->GetParentEntity();
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityTreeWidgetItem *item = dynamic_cast<EntityTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item && (item->id == entity->GetId()))
            item->setText(0, QString("%1 %2").arg(entity->GetId()).arg(entity->GetName()));
    }
}

void SceneStructureWindow::UpdateComponentName(const QString &oldName, const QString &newName)
{
}

