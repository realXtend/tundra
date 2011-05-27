/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.cpp
 *  @brief  Entity-component editor window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <OgreHardwareBuffer.h>

#include "ECEditorWindow.h"
#include "ECBrowser.h"
#include "EntityPlacer.h"
#include "AddComponentDialog.h"
#include "EntityActionDialog.h"
#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "FunctionInvoker.h"
#include "ECEditorModule.h"
#include "TransformEditor.h"

#include "Scene.h"
#include "Entity.h"
#include "Application.h"
#include "Profiler.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "EC_Name.h"
#include "EC_Placeable.h"
#include "InputAPI.h"
#include "LoggingFunctions.h"
#ifdef EC_Highlight_ENABLED
#include "EC_Highlight.h"
#endif

#include <QUiLoader>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

const QString cEcEditorHighlight("EcEditorHighlight");

EntityListWidgetItem::EntityListWidgetItem(const QString &name, QListWidget *list, Entity *entity):
    QListWidgetItem(name, list),
    entity(entity->shared_from_this())
{
}

uint AddUniqueListItem(Entity *entity, QListWidget* list, const QString& name)
{
    for(int i = 0; i < list->count(); ++i)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(list->item(i));
        if (item && item->GetEntity() && item->GetEntity().get() == entity)
            return i;
    }

    new EntityListWidgetItem(name, list, entity);
    return list->count() - 1;
}

/// Function that is used by std::sort algorithm to sort entities by their ids.
bool CmpEntityById(EntityPtr a, EntityPtr b)
{
    return a->GetId() < b->GetId();
}

ECEditorWindow::ECEditorWindow(Framework* fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    toggleEntitiesButton(0),
    entityList(0),
    ecBrowser(0),
    hasFocus(true),
    transformEditor(new TransformEditor(fw->Scene()->GetDefaultScene()))
{
    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile file(Application::InstallationDirectory() + "data/ui/eceditor.ui");
    file.open(QFile::ReadOnly);
    if (!file.exists())
    {
        LogError("Cannot find " + Application::InstallationDirectory() + "data/ui/eceditor.ui file.");
        return;
    }

    QWidget *contents = loader.load(&file, this);
    if (!contents)
    {
        LogError("Could not load editor layout");
        return;
    }
    contents->installEventFilter(this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(contents);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
    setWindowTitle(contents->windowTitle());
    resize(contents->size());

    toggleEntitiesButton = findChild<QPushButton *>("but_show_entities");
    entityList = findChild<QListWidget*>("list_entities");
    QWidget *entity_widget = findChild<QWidget*>("entity_widget");
    if(entity_widget)
        entity_widget->hide();

    QWidget *browserWidget = findChild<QWidget*>("browser_widget");
    if (browserWidget)
    {
        ecBrowser = new ECBrowser(framework, browserWidget);
        ecBrowser->setMinimumWidth(100);
        QVBoxLayout *property_layout = dynamic_cast<QVBoxLayout *>(browserWidget->layout());
        if (property_layout)
            property_layout->addWidget(ecBrowser);
    }

    if (ecBrowser)
    {
        // signals from attribute browser to editor window.
        connect(ecBrowser, SIGNAL(ShowXmlEditorForComponent(const QString &)), SLOT(ShowXmlEditorForComponent(const QString &)));
        connect(ecBrowser, SIGNAL(CreateNewComponent()), SLOT(CreateComponent()));
        connect(ecBrowser, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)), SLOT(HighlightEntities(const QString&, const QString&)));
        connect(ecBrowser, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)),
                SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)), Qt::UniqueConnection);

        ecBrowser->SetItemExpandMemory(framework->GetModule<ECEditorModule>()->ExpandMemory());
    }

    if (entityList)
    {
        entityList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        connect(entityList, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshPropertyBrowser()));
        connect(entityList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowEntityContextMenu(const QPoint &)));
    }

    if (toggleEntitiesButton)
        connect(toggleEntitiesButton, SIGNAL(pressed()), this, SLOT(ToggleEntityList()));

    // Default world scene is not added yet, so we need to listen when framework will send a DefaultWorldSceneChanged signal.
    connect(framework->Scene(), SIGNAL(DefaultWorldSceneChanged(Scene *)), SLOT(OnDefaultSceneChanged(Scene *)));

    connect(this, SIGNAL(FocusChanged(ECEditorWindow *)), framework->GetModule<ECEditorModule>(), SLOT(ECEditorFocusChanged(ECEditorWindow*)));

    Scene *scene = framework->Scene()->GetDefaultScene().get();
    if (scene)
    {
        connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), SLOT(RemoveEntity(Entity*)), Qt::UniqueConnection);
        connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionTypeField)),
            SLOT(OnActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);
    }
}

ECEditorWindow::~ECEditorWindow()
{
    SAFE_DELETE(transformEditor);
    //DeselectAllEntities(); the list is already cleared here
}

EntityListWidgetItem *ECEditorWindow::AddEntity(entity_id_t entity_id, bool udpate_ui)
{
    EntityListWidgetItem *item = 0;
    PROFILE(AddEntity);
    if (entityList)
    {
        entityList->blockSignals(true);
        //If entity don't have EC_Name then entity_name is same as it's id.
        QString entity_name = QString::number(entity_id);
        EntityPtr entity = framework->Scene()->GetDefaultScene()->GetEntity(entity_id);
        if (entity && entity->GetComponent<EC_Name>())
            entity_name = entity->GetName();

        int row = AddUniqueListItem(entity.get(), entityList, entity_name);
        item = dynamic_cast<EntityListWidgetItem *>(entityList->item(row));
        assert(item);

        entityList->blockSignals(false);
    }

    if (udpate_ui)
        RefreshPropertyBrowser();

    return item;
}

void ECEditorWindow::AddEntities(const QList<entity_id_t> &entities, bool select_all)
{
    ClearEntities();

    // SetEntitySelected() will block entity list's signals, no need to do it here.

    foreach(entity_id_t id, entities)
    {
        EntityListWidgetItem *item = AddEntity(id);
        if (select_all)
            SetEntitySelected(item, true);
    }

    RefreshPropertyBrowser();
}

void ECEditorWindow::RemoveEntity(entity_id_t entity_id, bool udpate_ui)
{
    PROFILE(RemoveEntity);
    if (!entityList)
        return;

    entityList->blockSignals(true);
    EntityPtr entity = framework->Scene()->GetDefaultScene()->GetEntity(entity_id);
    if (!entity)
    {
        LogError("Fail to remove entity, since scene don't contain entity by ID:" + ToString<entity_id_t>(entity_id));
        return;
    }

    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->GetEntity() && item->GetEntity().get() == entity.get())
        {
            entityList->removeItemWidget(item);
            SAFE_DELETE(item)
            break;
        }
    }

    entityList->blockSignals(false);
    if (udpate_ui)
        RefreshPropertyBrowser();
}

void ECEditorWindow::SetSelectedEntities(const QList<entity_id_t> &ids)
{
    PROFILE(SetSelectedEntities);
    if (!entityList)
        return;

    entityList->blockSignals(true);
    foreach(entity_id_t id, ids)
        for(uint i = 0; i < (uint)entityList->count(); ++i)
        {
            QListWidgetItem *item = entityList->item(i);
            if (id == (entity_id_t)item->text().toInt())
            {
                item->setSelected(true);
                break;
            }
        }

    entityList->blockSignals(false);
    RefreshPropertyBrowser();
}

void ECEditorWindow::ClearEntities()
{
    DeselectAllEntities();
    if (entityList)
        entityList->clear();
    RefreshPropertyBrowser();
}

QObjectList ECEditorWindow::GetSelectedComponents() const
{
    if (ecBrowser)
        return ecBrowser->GetSelectedComponents();
    return QObjectList();
}

QList<EntityPtr> ECEditorWindow::GetSelectedEntities() const
{
    QList<EntityPtr> ret;

    if (!entityList)
        return ret;

    ScenePtr scene = framework->Scene()->GetDefaultScene();
    if (!scene)
        return ret;

    for(uint i = 0; i < (uint)entityList->count(); ++i)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->GetEntity() && item->isSelected())
        {
            EntityPtr entity = scene->GetEntity(item->GetEntity()->GetId());
            if (entity)
                ret.push_back(entity);
        }
    }
    return ret;
}

void ECEditorWindow::SetEntitySelected(EntityListWidgetItem *item, bool select)
{
    entityList->blockSignals(true);
    item->setSelected(select);
    entityList->blockSignals(false);
    HighlightEntity(item->GetEntity(), select);
}

EntityListWidgetItem *ECEditorWindow::FindItem(entity_id_t id) const
{
    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->GetEntity() && item->GetEntity()->GetId() == id)
            return item;
    }

    return 0;
}

void ECEditorWindow::DeleteEntitiesFromList()
{
    if (entityList && entityList->hasFocus())
        for(int i = entityList->count() - 1; i >= 0; --i)
            if (entityList->item(i)->isSelected())
            {
                QListWidgetItem* item = entityList->takeItem(i);
                delete item;
            }
}

void ECEditorWindow::DeleteComponent(const QString &componentType, const QString &name)
{
    if(componentType.isEmpty())
        return;

    foreach(const EntityPtr &entity, GetSelectedEntities())
    {
        ComponentPtr component = entity->GetComponent(componentType, name);
        if (component)
            entity->RemoveComponent(component, AttributeChange::Default);
    }
}

void ECEditorWindow::CreateComponent()
{
    QList<entity_id_t> ids;
    foreach(EntityPtr e, GetSelectedEntities())
        ids.push_back(e->GetId());

    if (ids.size())
    {
        AddComponentDialog *dialog = new AddComponentDialog(framework, ids, NULL);
        dialog->SetComponentList(framework->Scene()->GetComponentTypes());
        connect(dialog, SIGNAL(finished(int)), this, SLOT(AddComponentDialogFinished(int)));
        dialog->show();
    }
}

void ECEditorWindow::EntityActionDialogFinished(int result)
{
    EntityActionDialog *dialog = qobject_cast<EntityActionDialog *>(sender());
    if (!dialog)
        return;

    if (result == QDialog::Rejected)
        return;

    foreach(const EntityWeakPtr &e, dialog->Entities())
        if (e.lock())
            e.lock()->Exec(dialog->ExecutionType(), dialog->Action(), dialog->Parameters());
}

void ECEditorWindow::FunctionDialogFinished(int result)
{
    FunctionDialog *dialog = qobject_cast<FunctionDialog *>(sender());
    if (!dialog)
        return;

    if (result == QDialog::Rejected)
        return;

    // Get the list of parameters we will pass to the function we are invoking,
    // and update the latest values to them from the editor widgets the user inputted.
    QVariantList params;
    foreach(IArgumentType *arg, dialog->Arguments())
    {
        arg->UpdateValueFromEditor();
        params << arg->ToQVariant();
    }

    // Clear old return value from the dialog.
    dialog->SetReturnValueText("");

    foreach(const QObjectWeakPtr &o, dialog->Objects())
        if (o.lock())
        {
            QObject *obj = o.lock().get();

            QString objName = obj->metaObject()->className();
            QString objNameWithId = objName;
            {
                Entity *e = dynamic_cast<Entity *>(obj);
                IComponent *c = dynamic_cast<IComponent *>(obj);
                if (e)
                    objNameWithId.append('(' + QString::number((uint)e->GetId()) + ')');
                else if (c)
                    objNameWithId.append('(' + c->Name() + ')');
            }

            QString errorMsg;
            QVariant ret;
            FunctionInvoker invoker;
            invoker.Invoke(obj, dialog->Function(), params, &ret, &errorMsg);

            if (errorMsg.isEmpty())
                dialog->AppendReturnValueText(objNameWithId + ' ' + ret.toString());
            else
                dialog->AppendReturnValueText(objNameWithId + ' ' + errorMsg);
        }
}

void ECEditorWindow::OnActionTriggered(Entity *entity, const QString &action, const QStringList &params)
{
    if (action == "MousePress" && params.size())
    {
        MouseEvent::MouseButton button = static_cast<MouseEvent::MouseButton>(params[0].toUInt());
        if (hasFocus && isVisible() && button == MouseEvent::LeftButton)
        {
            /// @todo This will now work if we lose windows focus and previous key state stays, replace this with InputContext.
            // If Ctrl is down, we're appending selection. If not, clear the old selection completely.
            if (!framework->Input()->IsKeyDown(Qt::Key_Control))
                DeselectAllEntities();

            EntityListWidgetItem *item = FindItem(entity->GetId());
            if (item && item->isSelected())
                SetEntitySelected(item, false);
            else
            {
                item = AddEntity(entity->GetId());
                SetEntitySelected(item, true);
            }
        }

        RefreshPropertyBrowser();
    }
}

void ECEditorWindow::DeleteEntity()
{
    ScenePtr scene = framework->Scene()->GetDefaultScene();
    if (!scene)
        return;

    QList<EntityPtr> entities = GetSelectedEntities();
    for(uint i = 0; i < (uint)entities.size(); ++i)
        scene->RemoveEntity(entities[i]->GetId(), AttributeChange::Default);
}

void ECEditorWindow::CopyEntity()
{
    /// @todo will only take a copy of first entity of the selection. 
    /// should we support multi entity copy and paste functionality.
    QDomDocument temp_doc;

    foreach(const EntityPtr &entity, GetSelectedEntities())
        if (entity)
        {
            QDomElement entity_elem = temp_doc.createElement("entity");
            entity_elem.setAttribute("id", QString::number((int)entity->GetId()));

            foreach(const ComponentPtr &comp, entity->Components())
                comp->SerializeTo(temp_doc, entity_elem);

            temp_doc.appendChild(entity_elem);
        }

    QApplication::clipboard()->setText(temp_doc.toString());
}

void ECEditorWindow::PasteEntity()
{
    // Dont allow paste operation if we are placing previosly pasted object to a scene.
    if(findChild<QObject*>("EntityPlacer"))
        return;
    // First we need to check if component is holding EC_OgrePlacable component to tell where entity should be located at.
    /// \todo local only server wont save those objects.
    ScenePtr scene = framework->Scene()->GetDefaultScene();
    assert(scene);
    if(!scene)
        return;
    
    QDomDocument temp_doc;
    QClipboard *clipboard = QApplication::clipboard();
    if (temp_doc.setContent(clipboard->text()))
    {
        //Check if clipboard contain infomation about entity's id,
        //which is used to find a right type of entity from the scene.
        QDomElement ent_elem = temp_doc.firstChildElement("entity");
        if(ent_elem.isNull())
            return;
        QString id = ent_elem.attribute("id");
        EntityPtr originalEntity = scene->GetEntity(ParseString<entity_id_t>(id.toStdString()));
        if(!originalEntity)
        {
            LogWarning("ECEditorWindow cannot create a new copy of entity, cause scene manager couldn't find entity. (id " + id.toStdString() + ").");
            return;
        }

        EntityPtr entity = scene->CreateEntity();
        assert(entity);
        if(!entity)
            return;

        bool hasPlaceable = false;
        Entity::ComponentVector components = originalEntity->Components();
        for(uint i = 0; i < components.size(); i++)
        {
            // If the entity is holding placeable component we can place it into the scene.
            if(components[i]->TypeName() == "EC_Placeable")
            {
                hasPlaceable = true;
                ComponentPtr component = entity->GetOrCreateComponent(components[i]->TypeName(), components[i]->Name(), AttributeChange::Default);
            }

            ComponentPtr component = entity->GetOrCreateComponent(components[i]->TypeName(), components[i]->Name(), AttributeChange::Default);
            AttributeVector attributes = components[i]->GetAttributes();
            for(uint j = 0; j < attributes.size(); j++)
            {
                IAttribute *attribute = component->GetAttribute(attributes[j]->GetNameString().c_str());
                if(attribute)
                    attribute->FromString(attributes[j]->ToString(), AttributeChange::Default);
            }
        }
        if(hasPlaceable)
        {
            EntityPlacer *entityPlacer = new EntityPlacer(framework, entity->GetId(), this);
            entityPlacer->setObjectName("EntityPlacer");
        }
        AddEntity(entity->GetId());
        scene->EmitEntityCreated(entity);
    }
}

void ECEditorWindow::OpenEntityActionDialog()
{
    QList<EntityWeakPtr> entities;
    foreach(EntityPtr entity, GetSelectedEntities())
        entities.append(entity);

    if (entities.size())
    {
        EntityActionDialog *d = new EntityActionDialog(entities, this);
        connect(d, SIGNAL(finished(int)), this, SLOT(EntityActionDialogFinished(int)));
        d->show();
    }
}

void ECEditorWindow::OpenFunctionDialog()
{
    QObjectWeakPtrList objs;
    foreach(EntityPtr entity, GetSelectedEntities())
        objs << boost::dynamic_pointer_cast<QObject>(entity);

    if (objs.size())
    {
        FunctionDialog *d = new FunctionDialog(objs, this);
        connect(d, SIGNAL(finished(int)), this, SLOT(FunctionDialogFinished(int)));
        d->show();
    }
}

void ECEditorWindow::HighlightEntities(const QString &type, const QString &name)
{
    QSet<entity_id_t> entities;
    foreach(EntityPtr entity, GetSelectedEntities())
        if (entity->GetComponent(type, name))
            entities.insert(entity->GetId());
    BoldEntityListItems(entities);
}

void ECEditorWindow::RefreshPropertyBrowser()
{
    PROFILE(EC_refresh_browser);
    if (!ecBrowser)
        return;

    ScenePtr scene = framework->Scene()->GetDefaultScene();
    if (!scene)
    {
        ecBrowser->clear();
        return;
    }

    // Unbold all items for starters.
    BoldEntityListItems(QSet<entity_id_t>());

    QList<EntityPtr> entities = GetSelectedEntities();
    // If any of entities was not selected clear the browser window.
    if (!entities.size())
    {
        ecBrowser->clear();
        transformEditor->SetGizmoVisible(false);
        return;
    }

    QList<EntityPtr> old_entities = ecBrowser->GetEntities();
    qStableSort(entities.begin(), entities.end(), CmpEntityById);
    qStableSort(old_entities.begin(), old_entities.end(), CmpEntityById);

    // Check what entities need to get removed/added to browser.
    QList<EntityPtr>::iterator iter1 = old_entities.begin(), iter2 = entities.begin();
    while(iter1 != old_entities.end() || iter2 != entities.end())
    {
        // No point to continue the iteration if old_entities list is empty. We can just push all new entitites into the browser.
        if (iter1 == old_entities.end())
        {
            for(;iter2 != entities.end(); iter2++)
                ecBrowser->AddEntity(*iter2);
            break;
        }
        // Only old entities are left and they can just be removed from the browser.
        else if(iter2 == entities.end())
        {
            for(;iter1 != old_entities.end(); iter1++)
                ecBrowser->RemoveEntity(*iter1);
            break;
        }

        // Entity has already added to the browser.
        if((*iter1)->GetId() == (*iter2)->GetId())
        {
            iter2++;
            iter1++;
        }
        // Found new entity that that need to be added to the browser.
        else if((*iter1)->GetId() > (*iter2)->GetId())
        {
            ecBrowser->AddEntity(*iter2);
            iter2++;
        }
        // Couldn't find entity in new entities list, so it need to be removed from the browser.
        else
        {
            ecBrowser->RemoveEntity(*iter1);
            iter1++;
        }
    }

    ecBrowser->UpdateBrowser();

    if (!GetSelectedEntities().isEmpty())
    {
        transformEditor->SetSelection(entities);
        transformEditor->FocusGizmoPivotToAabbBottomCenter();
        // Shows gizmo only if we have focus.
        transformEditor->SetGizmoVisible(hasFocus);
    }
}

void ECEditorWindow::ShowEntityContextMenu(const QPoint &pos)
{
    assert(entityList);
    if (!entityList)
        return;

    QListWidgetItem *item = entityList->itemAt(pos);
    if (!item)
        return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *editXml = new QAction(tr("Edit XML..."), menu);
    QAction *deleteEntity = new QAction(tr("Delete"), menu);
    QAction *addComponent = new QAction(tr("Add new component..."), menu);
    QAction *copyEntity = new QAction(tr("Copy"), menu);
    QAction *pasteEntity = new QAction(tr("Paste"), menu);
    QAction *actions = new QAction(tr("Actions..."), menu);
    QAction *functions = new QAction(tr("Functions..."), menu);

    connect(editXml, SIGNAL(triggered()), this, SLOT(ShowXmlEditorForEntity()));
    connect(deleteEntity, SIGNAL(triggered()), this, SLOT(DeleteEntity()));
    connect(addComponent, SIGNAL(triggered()), this, SLOT(CreateComponent()));
    connect(copyEntity, SIGNAL(triggered()), this, SLOT(CopyEntity()));
    connect(pasteEntity, SIGNAL(triggered()), this, SLOT(PasteEntity()));
    connect(actions, SIGNAL(triggered()), this, SLOT(OpenEntityActionDialog()));
    connect(functions, SIGNAL(triggered()), this, SLOT(OpenFunctionDialog()));

    menu->addAction(editXml);
    menu->addAction(deleteEntity);
    menu->addAction(addComponent);
    menu->addAction(copyEntity);
    menu->addAction(pasteEntity);
    menu->addAction(actions);
    menu->addAction(functions);

    menu->popup(entityList->mapToGlobal(pos));
}

void ECEditorWindow::ShowXmlEditorForEntity()
{
    QList<EntityPtr> entities = GetSelectedEntities();
    std::vector<EntityComponentSelection> selection;
    for(uint i = 0; i < (uint)entities.size(); i++)
    {
        EntityComponentSelection entityComponent;
        entityComponent.entity = entities[i];
        entityComponent.components = entities[i]->Components();
        selection.push_back(entityComponent);
    }

    if (!selection.size())
        return;

    QList<EntityPtr> ents;
    foreach(EntityComponentSelection ecs, selection)
        ents << ecs.entity;

    emit EditEntityXml(ents);
}

void ECEditorWindow::ShowXmlEditorForComponent(const std::vector<ComponentPtr> &components)
{
    if(!components.size())
        return;

    QList<ComponentPtr> comps;
    foreach(ComponentPtr component, components)
        comps << component;

    emit EditComponentXml(comps);
}

void ECEditorWindow::ShowXmlEditorForComponent(const QString &componentType)
{
    if (componentType.isEmpty())
        return;

    foreach(const EntityPtr &e, GetSelectedEntities())
    {
        ComponentPtr component = e->GetComponent(componentType);
        if (component)
            emit EditComponentXml(component);
    }
}

void ECEditorWindow::ToggleEntityList()
{
    QWidget *entity_widget = findChild<QWidget*>("entity_widget");
    if(entity_widget)
    {
        if (entity_widget->isVisible())
        {
            entity_widget->hide();
            resize(size().width() - entity_widget->size().width(), size().height());
            if (toggleEntitiesButton)
                toggleEntitiesButton->setText(tr("Show Entities"));
        }
        else
        {
            entity_widget->show();
            resize(size().width() + entity_widget->sizeHint().width(), size().height());
            if (toggleEntitiesButton)
                toggleEntitiesButton->setText(tr("Hide Entities"));
        }
    }
}

void ECEditorWindow::RemoveEntity(Entity* entity)
{
    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item->GetEntity().get() == entity)
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void ECEditorWindow::SetFocus(bool focus)
{
    hasFocus = focus;
//    LogInfo("ECEditorWindow::SetFocus: " + ToString(focus));
//    bool showGizmo = !GetSelectedEntities().isEmpty() && hasFocus;
//    LogInfo("SetFocus: showGizmo: " + ToString(showGizmo));
    transformEditor->SetGizmoVisible(!GetSelectedEntities().isEmpty() && hasFocus);

    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->isSelected())
            HighlightEntity(item->GetEntity(), hasFocus);
    }
}

void ECEditorWindow::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible)
        emit FocusChanged(this);

    bool showGizmo = !GetSelectedEntities().isEmpty() && hasFocus;
//    LogInfo("ECEditorWindow::setVisible: showGizmo: " + ToString(showGizmo));
    transformEditor->SetGizmoVisible(showGizmo);

    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->isSelected())
            HighlightEntity(item->GetEntity(), hasFocus);
    }
}

void ECEditorWindow::DeselectAllEntities()
{
    for(uint i = 0; i < (uint)entityList->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item)
            SetEntitySelected(item, false);
    }
}

void ECEditorWindow::HighlightEntity(const EntityPtr &entity, bool highlight)
{
#ifdef EC_Highlight_ENABLED
    if (entity)
    {
        // If component already has an EC_Highlight, that is not ours, do nothing, as the highlights would conflict
        ComponentPtr c = entity->GetComponent(EC_Highlight::TypeNameStatic());
        if ((c) && (c->Name() != cEcEditorHighlight))
            return;
        
        if (highlight)
        {
            EC_Highlight *hl = dynamic_cast<EC_Highlight *>(entity->GetOrCreateComponent(
                EC_Highlight::TypeNameStatic(), cEcEditorHighlight).get());
            if (hl)
            {
                hl->SetTemporary(true);
                hl->visible.Set(true, AttributeChange::Default);
            }
        }
        else
        {
            ComponentPtr c = entity->GetComponent(EC_Highlight::TypeNameStatic(), cEcEditorHighlight);
            if (c)
                entity->RemoveComponent(c);
        }
    }
#else
    LogInfo("ECEditorWindow::HighlightEntity: EC_Highlight not included in the build.");
#endif
}

void ECEditorWindow::hideEvent(QHideEvent* e)
{
    ClearEntities();
    if (ecBrowser)
        ecBrowser->clear();
    transformEditor->SetGizmoVisible(false);

    QWidget::hideEvent(e);
}

void ECEditorWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange)
        setWindowTitle(tr("Entity-Component Editor"));
    else
       QWidget::changeEvent(e);
}

bool ECEditorWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::WindowActivate)
        emit FocusChanged(this);
    return QWidget::eventFilter(obj, e);
}

void ECEditorWindow::BoldEntityListItems(const QSet<entity_id_t> &bolded_entities)
{
    PROFILE(BoldEntityListItems);
    for(uint i = 0; i < (uint)entityList->count(); ++i)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item)
        {
            EntityPtr ent = item->GetEntity();
            QFont font = item->font();
            if (ent && bolded_entities.contains(ent->GetId()))
            {
                font.setBold(true);
                item->setFont(font);
            }
            else
            {
                font.setBold(false);
                item->setFont(font);
            }
        }
    }
}

void ECEditorWindow::OnDefaultSceneChanged(Scene *scene)
{
    if (!scene)
        return;

    /// todo disconnect previous scene connection.
    connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)),
        SLOT(RemoveEntity(Entity*)), Qt::UniqueConnection);
    connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionTypeField)),
        SLOT(OnActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);
}

void ECEditorWindow::AddComponentDialogFinished(int result)
{
    AddComponentDialog *dialog = qobject_cast<AddComponentDialog*>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    ScenePtr scene = framework->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogWarning("Fail to add new component to entity, since default world scene was null");
        return;
    }

    foreach(entity_id_t id, dialog->GetEntityIds())
    {
        EntityPtr entity = scene->GetEntity(id);
        if (!entity)
        {
            LogWarning("Fail to add new component to entity, since couldn't find a entity with ID:" + ::ToString<entity_id_t>(id));
            continue;
        }

        // Check if component has been already added to a entity.
        ComponentPtr comp = entity->GetComponent(dialog->GetTypeName(), dialog->GetName());
        if (comp)
        {
            LogWarning("Fail to add a new component, cause there was already a component with a same name and a type");
            continue;
        }

        comp = framework->Scene()->CreateComponentByName(scene.get(), dialog->GetTypeName(), dialog->GetName());
        assert(comp);
        if (comp)
        {
            comp->SetNetworkSyncEnabled(dialog->GetSynchronization());
            comp->SetTemporary(dialog->GetTemporary());
            entity->AddComponent(comp, AttributeChange::Default);
        }
    }
}

