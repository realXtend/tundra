/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ECEditorWindow.cpp
    @brief  Entity-component editor window. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorWindow.h"
#include "ECBrowser.h"
#include "AddComponentDialog.h"
#include "EntityActionDialog.h"
#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "FunctionInvoker.h"
#include "ECEditorModule.h"
#include "TransformEditor.h"
#include "UndoCommands.h"
#include "UndoManager.h"

#include "Scene/Scene.h"
#include "Entity.h"
#include "Application.h"
#include "Profiler.h"
#include "SceneAPI.h"
#include "EC_Name.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "LoggingFunctions.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/Quat.h"
#ifdef EC_Highlight_ENABLED
#include "EC_Highlight.h"
#endif

#include <QDomDocument>

#include "MemoryLeakCheck.h"

namespace
{

const char *cEcEditorHighlight = "EcEditorHighlight";

int AddUniqueListItem(const EntityPtr &entity, QListWidget* list, const QString& name)
{
    for(int i = 0; i < list->count(); ++i)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(list->item(i));
        if (item && item->Entity() && item->Entity() == entity)
            return i;
    }

    new EntityListWidgetItem(name, list, entity);
    return list->count() - 1;
}

} // ~unnamed namespace

ECEditorWindow::ECEditorWindow(Framework* fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    ecBrowser(0),
    hasFocus(true)
{
    /// @todo Create UI fully in code (very simple UI file).
    setupUi(this);
    installEventFilter(this);

    Scene *scene = fw->Scene()->MainCameraScene();
    assert(scene);
    undoManager_ = new UndoManager(scene->shared_from_this(), this);
    transformEditor = new TransformEditor(scene->shared_from_this(), undoManager_);

    undoButton->setDisabled(true);
    redoButton->setDisabled(true);

    undoButton->setIcon(QIcon(Application::InstallationDirectory() + "data/ui/images/icon/undo-icon.png"));
    redoButton->setIcon(QIcon(Application::InstallationDirectory() + "data/ui/images/icon/redo-icon.png"));

    undoButton->setMenu(undoManager_->UndoMenu());
    redoButton->setMenu(undoManager_->RedoMenu());

    entityWidget->hide();

    ecBrowser = new ECBrowser(framework, this, browserWidget);
    ecBrowser->setMinimumWidth(100);
    browserWidget->layout()->addWidget(ecBrowser);

    // signals from attribute browser to editor window.
    connect(ecBrowser, SIGNAL(ShowXmlEditorForComponent(const QString &)), SLOT(ShowXmlEditorForComponent(const QString &)));
    connect(ecBrowser, SIGNAL(CreateNewComponent()), SLOT(CreateComponent()));
    connect(ecBrowser, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)),
        SLOT(HighlightEntities(const QString&, const QString&)));
    connect(ecBrowser, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)),
        SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)), Qt::UniqueConnection);

    ECEditorModule *ecEditorModule = framework->Module<ECEditorModule>();
    ecBrowser->SetItemExpandMemory(ecEditorModule->ExpandMemory());

    entityList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(entityList, SIGNAL(itemSelectionChanged()), this, SLOT(Refresh()));
    connect(entityList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowEntityContextMenu(const QPoint &)));

    connect(toggleEntitiesButton, SIGNAL(pressed()), this, SLOT(ToggleEntityList()));
    connect(expandOrCollapseButton, SIGNAL(clicked()), ecBrowser, SLOT(ExpandOrCollapseAll()));

    connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), SLOT(RemoveEntity(Entity*)), Qt::UniqueConnection);
    connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecTypeField)),
        SLOT(OnActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);

    connect(this, SIGNAL(FocusChanged(ECEditorWindow *)), ecEditorModule, SLOT(ECEditorFocusChanged(ECEditorWindow*)));
    connect(this, SIGNAL(EditEntityXml(const QList<EntityPtr> &)), ecEditorModule, SLOT(CreateXmlEditor(const QList<EntityPtr> &)));
    connect(this, SIGNAL(EditComponentXml(const QList<ComponentPtr> &)), ecEditorModule, SLOT(CreateXmlEditor(const QList<ComponentPtr> &)));
    //connect(this, SIGNAL(AttributeAboutToBeEdited(IAttribute *)), this, SLOT(OnAboutToEditAttribute(IAttribute* )));
    connect(undoManager_, SIGNAL(CanUndoChanged(bool)), this, SLOT(OnUndoChanged(bool)));
    connect(undoManager_, SIGNAL(CanRedoChanged(bool)), this, SLOT(OnRedoChanged(bool)));
    connect(undoButton, SIGNAL(clicked()), undoManager_, SLOT(Undo()));
    connect(redoButton, SIGNAL(clicked()), undoManager_, SLOT(Redo()));

    connect(framework->Input()->TopLevelInputContext(), SIGNAL(KeyPressed(KeyEvent*)), SLOT(OnKeyEvent(KeyEvent*)));

    // Make sure the editor is cleared if the scene is cleared or removed.
    connect(scene, SIGNAL(SceneCleared(Scene *)), SLOT(OnSceneRemoved(Scene *)));
    connect(framework->Scene(), SIGNAL(SceneAboutToBeRemoved(Scene *, AttributeChange::Type)), SLOT(OnSceneRemoved(Scene *)));
}

ECEditorWindow::~ECEditorWindow()
{
    SAFE_DELETE(undoManager_);
    SAFE_DELETE(transformEditor);
    //DeselectAllEntities(); the list is already cleared here
}

EntityListWidgetItem *ECEditorWindow::AddEntity(const EntityPtr &entity, bool updateUi)
{
    PROFILE(ECEditorWindow_AddEntity);

    EntityListWidgetItem *item = 0;
    entityList->blockSignals(true);

    QString text = QString("%1 %2").arg(entity->Id()).arg(entity->Name().isEmpty() ? "(no name)" : entity->Name());
    int row = AddUniqueListItem(entity, entityList, text);
    item = checked_static_cast<EntityListWidgetItem *>(entityList->item(row));

    entityList->blockSignals(false);

    if (updateUi)
        Refresh();

    return item;
}

EntityListWidgetItem *ECEditorWindow::AddEntity(entity_id_t id, bool updateUi)
{
    return AddEntity(framework->Scene()->MainCameraScene()->EntityById(id), updateUi);
}

void ECEditorWindow::AddEntities(const EntityList &entities, bool selectAll)
{
    // SetEntitySelected() will block entity list's signals, no need to do it here.
    ClearEntities();

    foreach(const EntityPtr &entity, entities)
    {
        EntityListWidgetItem *item = AddEntity(entity, false);
        if (selectAll)
            SetEntitySelected(item, true, false);
    }

    if (selectAll)
        emit EntitiesSelected(entities, selectAll);

    Refresh();
}

void ECEditorWindow::AddEntities(const QList<entity_id_t> &entities, bool selectAll)
{
    EntityList entityPtrs;
    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    foreach(entity_id_t id, entities)
    {
        EntityPtr entity = scene->EntityById(id);
        entityPtrs.push_back(entity);
    }

    AddEntities(entityPtrs, selectAll);
}

void ECEditorWindow::RemoveEntity(entity_id_t entity_id, bool udpate_ui)
{
    PROFILE(ECEditorWindow_RemoveEntity);

    entityList->blockSignals(true);
    EntityPtr entity = framework->Scene()->MainCameraScene()->EntityById(entity_id);
    if (!entity)
    {
        LogError("Failed to remove entity, since scene doesn't contain entity with ID: " + QString::number(entity_id));
        return;
    }

    for(int i = 0; i < entityList->count(); i++)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->Entity() && item->Entity().get() == entity.get())
        {
            entityList->removeItemWidget(item);
            SAFE_DELETE(item)
            break;
        }
    }

    entityList->blockSignals(false);
    if (udpate_ui)
        Refresh();
}

void ECEditorWindow::SetSelectedEntities(const QList<entity_id_t> &ids)
{
    PROFILE(ECEditorWindow_SetSelectedEntities);

    entityList->blockSignals(true);
    foreach(entity_id_t id, ids)
        for(int i = 0; i < entityList->count(); ++i)
        {
            QListWidgetItem *item = entityList->item(i);
            if (id == item->text().toUInt())
            {
                item->setSelected(true);
                break;
            }
        }

    entityList->blockSignals(false);
    Refresh();
}

void ECEditorWindow::ClearEntities()
{
    DeselectAllEntities();
    entityList->clear();
    Refresh();
}

std::vector<ComponentPtr> ECEditorWindow::SelectedComponents() const
{
    return ecBrowser ? ecBrowser->SelectedComponents() : std::vector<ComponentPtr>();
}

EntityList ECEditorWindow::SelectedEntities() const
{
    EntityList ret;

    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    if (!scene)
        return ret;

    for(int i = 0; i < entityList->count(); ++i)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->Entity() && item->isSelected())
        {
            EntityPtr entity = item->Entity();
            if (entity)
                ret.push_back(entity);
            /// @todo Remove item if Entity expired?
        }
    }
    return ret;
}

void ECEditorWindow::SetEntitySelected(EntityListWidgetItem *item, bool select, bool signal)
{
    entityList->blockSignals(true);
    item->setSelected(select);
    entityList->blockSignals(false);

    if (framework->Module<ECEditorModule>()->IsHighlightingEnabled())
        HighlightEntity(item->Entity(), select);

    if (signal)
        emit EntitySelected(item->Entity(), select);
}

EntityListWidgetItem *ECEditorWindow::FindItem(entity_id_t id) const
{
    for(int i = 0; i < entityList->count(); i++)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->Entity() && item->Entity()->Id() == id)
            return item;
    }

    return 0;
}

void ECEditorWindow::SetHighlightingEnabled(bool show)
{
    for(int i = 0; i < entityList->count(); i++)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && item->isSelected())
            HighlightEntity(item->Entity(), show);
    }
}

void ECEditorWindow::SetGizmoVisible(bool show)
{
    transformEditor->SetGizmoVisible(show);
}

void ECEditorWindow::CreateComponent()
{
    QList<entity_id_t> ids;
    foreach(const EntityPtr &e, SelectedEntities())
        ids.push_back(e->Id());

    if (ids.size())
    {
        AddComponentDialog *dialog = new AddComponentDialog(framework, ids, framework->Ui()->MainWindow(), Qt::Tool);
        dialog->SetComponentList(framework->Scene()->ComponentTypes());
        connect(dialog, SIGNAL(finished(int)), this, SLOT(AddComponentDialogFinished(int)));
        dialog->show();
        dialog->activateWindow();
    }
}

void ECEditorWindow::OnKeyEvent(KeyEvent *keyEvent)
{
    if (!isVisible() || !undoManager_)
        return;

    const QKeySequence undo = framework->Input()->KeyBinding("TundraEditors.Undo", QKeySequence(Qt::ControlModifier + Qt::Key_Z));
    const QKeySequence redo = framework->Input()->KeyBinding("TundraEditors.Redo", QKeySequence(Qt::ControlModifier + Qt::Key_Y));

    if (keyEvent->sequence == undo)
    {
        undoManager_->Undo();
        keyEvent->Suppress();
    }
    else if (keyEvent->sequence == redo)
    {
        undoManager_->Redo();
        keyEvent->Suppress();
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
        if (!e.expired())
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
                    objNameWithId.append('(' + QString::number((uint)e->Id()) + ')');
                else if (c && !c->Name().trimmed().isEmpty())
                    objNameWithId.append('(' + c->Name() + ')');
            }

            QString errorMsg;
            QVariant ret;
            FunctionInvoker::Invoke(obj, dialog->Function(), params, &ret, &errorMsg);

            QString retValStr;
            ///\todo For some reason QVariant::toString() cannot convert QStringList to QString properly.
            /// Convert it manually here.
            if (ret.type() == QVariant::StringList)
                foreach(QString s, ret.toStringList())
                    retValStr.append("\n" + s);
            else
                retValStr = ret.toString();

            if (errorMsg.isEmpty())
                dialog->AppendReturnValueText(objNameWithId + ' ' + retValStr);
            else
                dialog->AppendReturnValueText(objNameWithId + ' ' + errorMsg);
        }
}

void ECEditorWindow::OnActionTriggered(Entity *entity, const QString &action, const QStringList &params)
{
    if (action.compare("MousePress", Qt::CaseInsensitive) == 0 && params.size())
    {
        MouseEvent::MouseButton button = static_cast<MouseEvent::MouseButton>(params[0].toUInt());
        if (hasFocus && isVisible() && button == MouseEvent::LeftButton)
        {
            /// @todo This will now work if we lose windows focus and previous key state stays, replace this with InputContext.
            // If Ctrl is down, we're appending selection. If not, clear the old selection completely.
            if (!framework->Input()->IsKeyDown(Qt::Key_Control))
                DeselectAllEntities();

            EntityListWidgetItem *item = FindItem(entity->Id());
            if (item)
                SetEntitySelected(item, !item->isSelected());
            else
                SetEntitySelected(AddEntity(entity->shared_from_this()), true);

            Refresh();
        }
    }
}

void ECEditorWindow::DeleteEntity()
{
    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    if (!scene)
        return;

    foreach(const EntityPtr &e, SelectedEntities())
        scene->RemoveEntity(e->Id(), AttributeChange::Default);
}

void ECEditorWindow::CopyEntity()
{
    /// @todo multiple entities copy-paste support.
    QDomDocument temp_doc;

    // Create root Scene element always for consistency, even if we only have one entity
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = temp_doc.createElement("scene");
    foreach(const EntityPtr &entity, SelectedEntities())
        if (entity)
        {
            QDomElement entity_elem = temp_doc.createElement("entity");
            entity_elem.setAttribute("id", QString::number((int)entity->Id()));

            const Entity::ComponentMap &components = entity->Components();
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                i->second->SerializeTo(temp_doc, entity_elem);

            scene_elem.appendChild(entity_elem);
        }

    temp_doc.appendChild(scene_elem);

    QApplication::clipboard()->setText(temp_doc.toString());
}

void ECEditorWindow::PasteEntity()
{
    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    assert(scene);
    if (!scene)
        return;

    QString errorMsg;
    QDomDocument temp_doc("Scene");
    if (!temp_doc.setContent(QApplication::clipboard()->text(), false, &errorMsg))
    {
        LogError("Parsing scene XML from clipboard failed: " + errorMsg);
        return;
    }

    // Check if clipboard contain infomation about entity's id,
    // which is used to find a right type of entity from the scene.
    QDomElement sceneElem = temp_doc.firstChildElement("scene");
    if (sceneElem.isNull())
        return;
    QDomElement ent_elem = sceneElem.firstChildElement("entity");
    if (ent_elem.isNull())
        return;
    QString id = ent_elem.attribute("id");
    EntityPtr originalEntity = scene->EntityById(id.toUInt());
    if (!originalEntity)
    {
        LogWarning("ECEditorWindow::PasteEntity: cannot create a new copy of entity, because scene manager couldn't find the original entity. (id " + id + ").");
        return;
    }

    EntityPtr entity = scene->CreateEntity(originalEntity->IsLocal() ? scene->NextFreeIdLocal() : scene->NextFreeId());
    assert(entity);
    if (!entity)
        return;

    const Entity::ComponentMap &components =  originalEntity->Components();
    for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
    {
        ComponentPtr component = entity->GetOrCreateComponent(i->second->TypeId(), i->second->Name(), AttributeChange::Default);
        const AttributeVector &attributes = i->second->Attributes();
        for(size_t j = 0; j < attributes.size(); j++)
            if (attributes[j])
            {
                IAttribute *attribute = component->AttributeByName(attributes[j]->Name());
                if(attribute)
                    attribute->FromString(attributes[j]->ToString(), AttributeChange::Default);
            }
    }

    AddEntity(entity);
}

void ECEditorWindow::OpenEntityActionDialog()
{
    QList<EntityWeakPtr> entities;
    foreach(const EntityPtr &entity, SelectedEntities())
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
    foreach(const EntityPtr &entity, SelectedEntities())
        objs << entity;

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
    foreach(const EntityPtr &entity, SelectedEntities())
        if (entity->Component(type, name))
            entities.insert(entity->Id());
    BoldEntityListItems(entities);
}

void ECEditorWindow::Refresh()
{
    PROFILE(ECEditorWindow_Refresh);
    if (!ecBrowser)
        return;

    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    if (!scene)
    {
        ecBrowser->clear();
        return;
    }

    // Unbold all items for starters.
    BoldEntityListItems(QSet<entity_id_t>());

    EntityList entities = SelectedEntities();
    if (entities.empty()) // If any of entities was not selected clear the browser window.
    {
        ecBrowser->clear();
        transformEditor->SetSelection(entities);
        transformEditor->SetGizmoVisible(false);
        return;
    }

    EntityList old_entities = ecBrowser->Entities();
    entities.sort();
    old_entities.sort();

    // Check what entities need to get removed/added to browser.
    EntityList::iterator iter1 = old_entities.begin(), iter2 = entities.begin();
    while(iter1 != old_entities.end() || iter2 != entities.end())
    {
        // No point to continue the iteration if old_entities list is empty. We can just push all new entities into the browser.
        if (iter1 == old_entities.end())
        {
            for(;iter2 != entities.end(); ++iter2)
                ecBrowser->AddEntity(*iter2);
            break;
        }
        // Only old entities are left and they can just be removed from the browser.
        else if(iter2 == entities.end())
        {
            for(;iter1 != old_entities.end(); ++iter1)
                ecBrowser->RemoveEntity(*iter1);
            break;
        }

        // Entity has already added to the browser.
        if((*iter1)->Id() == (*iter2)->Id())
        {
            ++iter2;
            ++iter1;
        }
        // Found new entity that that need to be added to the browser.
        else if((*iter1)->Id() > (*iter2)->Id())
        {
            ecBrowser->AddEntity(*iter2);
            ++iter2;
        }
        // Couldn't find entity in new entities list, so it need to be removed from the browser.
        else
        {
            ecBrowser->RemoveEntity(*iter1);
            ++iter1;
        }
    }

    ecBrowser->UpdateBrowser();

    // Show/set only entities with placeable to transform editor
    EntityList entitiesWithPlaceable = SelectedEntities();
    for(EntityList::iterator it = entitiesWithPlaceable.begin(); it != entitiesWithPlaceable.end(); )
        if ((*it)->Component<EC_Placeable>())
            ++it;
        else
            it = entitiesWithPlaceable.erase(it);

    if (!entitiesWithPlaceable.empty())
    {
        transformEditor->SetSelection(entitiesWithPlaceable);
        transformEditor->FocusGizmoPivotToAabbCenter();
        // Shows gizmo only if we have focus.
        const bool enabled = framework->Module<ECEditorModule>()->IsGizmoEnabled();
        transformEditor->SetGizmoVisible(hasFocus && enabled);
    }
}

void ECEditorWindow::ShowEntityContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = entityList->itemAt(pos);
    // Do not necessarily return if we have no item
    // We can use paste entity without selection if we seem to have valid scene XML in the contents.
    QString clipboardContents = QApplication::clipboard()->text();
    bool clipboardHasEntityXml = clipboardContents.contains("scene") && clipboardContents.contains("entity");
    if (!item && !clipboardHasEntityXml)
        return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose); /**< @todo has no effect if we don't call close() explicitly, and hence leaks memory (until ECEditorWindow is destroyed). */
    QAction *editXml = 0, *deleteEntity = 0, *addComponent = 0, *copyEntity = 0, *pasteEntity = 0, *actions = 0, *functions = 0;
    if (item)
    {
        editXml = new QAction(tr("Edit XML..."), menu);
        deleteEntity = new QAction(tr("Delete"), menu);
        addComponent = new QAction(tr("Add new component..."), menu);
        copyEntity = new QAction(tr("Copy"), menu);
        actions = new QAction(tr("Actions..."), menu);
        functions = new QAction(tr("Functions..."), menu);

        connect(editXml, SIGNAL(triggered()), this, SLOT(ShowXmlEditorForEntity()));
        connect(deleteEntity, SIGNAL(triggered()), this, SLOT(DeleteEntity()));
        connect(addComponent, SIGNAL(triggered()), this, SLOT(CreateComponent()));
        connect(copyEntity, SIGNAL(triggered()), this, SLOT(CopyEntity()));
        connect(actions, SIGNAL(triggered()), this, SLOT(OpenEntityActionDialog()));
        connect(functions, SIGNAL(triggered()), this, SLOT(OpenFunctionDialog()));
    }

    if (clipboardHasEntityXml)
    {
        pasteEntity = new QAction(tr("Paste"), menu);
        connect(pasteEntity, SIGNAL(triggered()), this, SLOT(PasteEntity()));
    }

    if (item)
    {
        menu->addAction(editXml);
        menu->addAction(deleteEntity);
        menu->addAction(addComponent);
        menu->addAction(copyEntity);
    }
    if (clipboardHasEntityXml)
        menu->addAction(pasteEntity);
    if (item)
    {
        menu->addAction(actions);
        menu->addAction(functions);
    }

    if (item)
    {
        QList<QObject*> targets;
        EntityListWidgetItem* entityItem = checked_static_cast<EntityListWidgetItem*>(item);
        if (entityItem && entityItem->Entity())
            targets.push_back(entityItem->Entity().get());
        framework->Ui()->EmitContextMenuAboutToOpen(menu, targets);
    }

    menu->popup(entityList->mapToGlobal(pos));
}

void ECEditorWindow::ShowXmlEditorForEntity()
{
    std::vector<EntityComponentSelection> selection;
    foreach(const EntityPtr &e, SelectedEntities())
    {
        EntityComponentSelection entityComponent;
        entityComponent.entity = e;
        const Entity::ComponentMap &components = e->Components();
        for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            entityComponent.components.push_back(i->second);
        selection.push_back(entityComponent);
    }

    if (!selection.size())
        return;

    QList<EntityPtr> ents;
    foreach(const EntityComponentSelection &ecs, selection)
        ents << ecs.entity;

    emit EditEntityXml(ents);
}

void ECEditorWindow::ShowXmlEditorForComponent(const QList<ComponentPtr> &components)
{
    emit EditComponentXml(components);
}

void ECEditorWindow::ShowXmlEditorForComponent(const QString &componentType)
{
    QList<ComponentPtr> components;
    foreach(const EntityPtr &e, SelectedEntities())
    {
        ComponentPtr component = e->Component(componentType);
        if (component)
            components << component;
    }

    emit EditComponentXml(components);
}

void ECEditorWindow::ToggleEntityList()
{
    if (entityWidget->isVisible())
    {
        entityWidget->hide();
        resize(size().width() - entityWidget->size().width(), size().height());
        toggleEntitiesButton->setText(tr("Show Entities"));
    }
    else
    {
        entityWidget->show();
        resize(size().width() + entityWidget->sizeHint().width(), size().height());
        toggleEntitiesButton->setText(tr("Hide Entities"));
    }
}

void ECEditorWindow::RemoveEntity(Entity* entity)
{
    for(int i = 0; i < entityList->count(); i++)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item->Entity().get() == entity)
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void ECEditorWindow::SetFocus(bool focus)
{
    hasFocus = focus;
    SetGizmoVisible(!SelectedEntities().empty() && hasFocus);
    if (framework->Module<ECEditorModule>()->IsHighlightingEnabled())
        for(int i = 0; i < entityList->count(); i++)
        {
            EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
            if (item && item->isSelected())
                HighlightEntity(item->Entity(), hasFocus);
        }
}

void ECEditorWindow::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible)
        emit FocusChanged(this);

    transformEditor->SetGizmoVisible(!SelectedEntities().empty() && hasFocus);

    if (framework->Module<ECEditorModule>()->IsHighlightingEnabled())
        for(int i = 0; i < entityList->count(); i++)
        {
            EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
            if (item && item->isSelected())
                HighlightEntity(item->Entity(), hasFocus);
        }
}

void ECEditorWindow::DeselectAllEntities()
{
    emit EntitiesSelected(SelectedEntities(), false);

    for(int i = 0; i < entityList->count(); i++)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item)
            SetEntitySelected(item, false, false);
    }
}

void ECEditorWindow::HighlightEntity(const EntityPtr &entity, bool highlight)
{
#ifdef EC_Highlight_ENABLED
    if (entity)
    {
        // Optimization when selecting large group of entities, and sensible logic otherwise too:
        // Don't create EC_Highlight if there is no EC_Mesh in the entity. EC_Highlight does
        // absolutely nothing if there is no mesh. Granted it listens when EC_Mesh is added, but if you
        // are going to add meshes you might as well reselect your entities to get a highlight.
        // Creating the EC_Highlight to the entity is a major time spender if we are talking of large amount of entities.
        if (!entity->Component<EC_Mesh>())
            return;

        // If component already has an EC_Highlight, that is not ours, do nothing, as the highlights would conflict
        ComponentPtr c = entity->Component(EC_Highlight::ComponentTypeId);
        if (c && c->Name() != cEcEditorHighlight)
            return;
        
        if (highlight)
        {
            // Create Highlights as disconnected: gives tremendous performance gain when dealing with hundreds or even thousands of entities
            // as Scene Structure window doesn't create potentially thousands of items that exists only for a brief moment.
            shared_ptr<EC_Highlight> hl = entity->GetOrCreateComponent<EC_Highlight>(cEcEditorHighlight, AttributeChange::Disconnected, false);
            if (hl)
            {
                hl->SetTemporary(true);
                hl->visible.Set(true, AttributeChange::Default);
            }
        }
        else
        {
            ComponentPtr c = entity->Component(EC_Highlight::ComponentTypeId, cEcEditorHighlight);
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
    PROFILE(ECEditorWindow_BoldEntityListItems);
    for(int i = 0; i < entityList->count(); ++i)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item)
        {
            EntityPtr ent = item->Entity();
            QFont font = item->font();
            if (ent && bolded_entities.contains(ent->Id()))
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

/// @todo Make this some sort of SetScene function.
/*
void ECEditorWindow::OnDefaultSceneChanged(Scene *scene)
{
    if (!scene)
        return;

    /// @todo disconnect previous scene connection.
    connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)),
        SLOT(RemoveEntity(Entity*)), Qt::UniqueConnection);
    connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecTypeField)),
        SLOT(OnActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);
}
*/

void ECEditorWindow::AddComponentDialogFinished(int result)
{
    AddComponentDialog *dialog = qobject_cast<AddComponentDialog*>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    Scene *scene = framework->Scene()->MainCameraScene(); /**< @todo Use "scene of this editor", or use Entity's Scene. */
    if (!scene)
    {
        LogWarning("Failed to add new component to entity, since main camera scene was null");
        return;
    }

    QList<entity_id_t> targetEntities;

    foreach(entity_id_t id, dialog->EntityIds())
    {
        EntityPtr entity = scene->EntityById(id);
        if (!entity)
        {
            LogWarning("Failed to add a new component to an entity, since couldn't find a entity with ID: " + QString::number(id));
            continue;
        }

        // Check if component has been already added to a entity.
        ComponentPtr comp = entity->Component(dialog->TypeId(), dialog->Name());
        if (comp)
        {
            LogWarning("Failed to add a new component, because there was already a component with the same type and the same name.");
            continue;
        }

        targetEntities << id;
    }

    undoManager_->Push(new AddComponentCommand(scene->shared_from_this(), undoManager_->Tracker(),
        targetEntities, dialog->TypeId(), dialog->Name(), dialog->IsReplicated(), dialog->IsTemporary()));
}

void ECEditorWindow::OnAboutToEditAttribute(IAttribute *attr)
{
    switch(attr->TypeId())
    {
    case cAttributeReal:
        undoManager_->Push(new EditAttributeCommand<float>(attr));
        break;
    case cAttributeInt:
        undoManager_->Push(new EditAttributeCommand<int>(attr));
        break;
    case cAttributeUInt:
        undoManager_->Push(new EditAttributeCommand<unsigned int>(attr));
        break;
    case cAttributeFloat2:
        undoManager_->Push(new EditAttributeCommand<float2>(attr));
        break;
    case cAttributeFloat3:
        undoManager_->Push(new EditAttributeCommand<float3>(attr));
        break;
    case cAttributeFloat4:
        undoManager_->Push(new EditAttributeCommand<float4>(attr));
        break;
    case cAttributeQuat:
        undoManager_->Push(new EditAttributeCommand<Quat>(attr));
        break;
    case cAttributeColor:
        undoManager_->Push(new EditAttributeCommand<Color>(attr));
        break;
    case cAttributeString:
        undoManager_->Push(new EditAttributeCommand<QString>(attr));
        break;
    case cAttributeBool:
        undoManager_->Push(new EditAttributeCommand<bool>(attr));
        break;
    case cAttributeQVariant:
        undoManager_->Push(new EditAttributeCommand<QVariant>(attr));
        break;
    case cAttributeQVariantList:
        undoManager_->Push(new EditAttributeCommand<QVariantList>(attr));
        break;
    case cAttributeEntityReference:
        undoManager_->Push(new EditAttributeCommand<EntityReference>(attr));
        break;
    case cAttributeAssetReference:
        undoManager_->Push(new EditAttributeCommand<AssetReference>(attr));
        break;
    case cAttributeAssetReferenceList:
        undoManager_->Push(new EditAttributeCommand<AssetReferenceList>(attr));
        break;
    case cAttributeTransform:
        undoManager_->Push(new EditAttributeCommand<Transform>(attr));
        break;
    case cAttributeQPoint:
        undoManager_->Push(new EditAttributeCommand<QPoint>(attr));
        break;
    default:
        LogWarning("Unknown attribute type " + attr->TypeName() + " for pushing into the undo stack.");
        break;
    }
}

void ECEditorWindow::OnUndoChanged(bool canUndo)
{
    undoButton->setEnabled(canUndo);
}

void ECEditorWindow::OnRedoChanged(bool canRedo)
{
    redoButton->setEnabled(canRedo);
}

void ECEditorWindow::OnSceneRemoved(Scene *removedScene)
{
    for(int i = 0; i < entityList->count(); ++i)
    {
        EntityListWidgetItem *item = checked_static_cast<EntityListWidgetItem*>(entityList->item(i));
        if (item && (!item->Entity() || !item->Entity()->ParentScene() || item->Entity()->ParentScene() == removedScene))
        {
            ClearEntities(); /**< @todo Assuming all the edited entities are from the same scene. */
            break;
        }
    }
}
