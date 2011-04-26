/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.cpp
 *  @brief  Entity-component editor window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "ECEditorWindow.h"
#include "ECBrowser.h"
#include "EntityPlacer.h"
#include "AddComponentDialog.h"
#include "EntityActionDialog.h"
#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "FunctionInvoker.h"
#include "ECEditorModule.h"

#include "UiProxyWidget.h"
#include "ModuleManager.h"
#include "SceneAPI.h"
#include "SceneManager.h"
#include "EC_Name.h"
#include "ComponentManager.h"
#include "EC_Placeable.h"
#include "InputAPI.h"
#include "LoggingFunctions.h"

#include <QUiLoader>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

uint AddUniqueListItem(Entity *entity, QListWidget* list, const QString& name)
{
    for(int i = 0; i < list->count(); ++i)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(list->item(i));
        if (item && !item->GetEntity().isNull() && item->GetEntity().data() == entity)
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

ECEditorWindow::ECEditorWindow(Framework* framework) :
    framework_(framework),
    toggle_entities_button_(0),
    entity_list_(0),
    browser_(0),
    component_dialog_(0),
    has_focus_(true)
{
    Initialize();
}

ECEditorWindow::~ECEditorWindow()
{
}

void ECEditorWindow::AddEntity(entity_id_t entity_id, bool udpate_ui)
{
    PROFILE(AddEntity);
    if (entity_list_)
    {
        entity_list_->blockSignals(true);
        //If entity don't have EC_Name then entity_name is same as it's id.
        QString entity_name = QString::number(entity_id);
        EntityPtr entity = framework_->Scene()->GetDefaultScene()->GetEntity(entity_id);
        if(entity && entity->HasComponent("EC_Name"))
            entity_name = dynamic_cast<EC_Name*>(entity->GetComponent("EC_Name").get())->name.Get();

        /// @todo This will now work if we loose windows focus and previos key state stays, replace this with InputContext.
        if(!framework_->Input()->IsKeyDown(Qt::Key_Control))
            entity_list_->clearSelection();

        int row = AddUniqueListItem(entity.get(), entity_list_, entity_name);
        QListWidgetItem *item = entity_list_->item(row);
        // Toggle selection.
        item->setSelected(!item->isSelected());
        entity_list_->blockSignals(false);
    }
    if (udpate_ui)
        RefreshPropertyBrowser();
}

void ECEditorWindow::AddEntities(const QList<entity_id_t> &entities, bool select_all)
{
    entity_list_->blockSignals(true);
    ClearEntities();
    foreach(entity_id_t id, entities)
    {
        QString entity_name = QString::number(id);
        EntityPtr entity = framework_->Scene()->GetDefaultScene()->GetEntity(id);
        if(entity && entity->HasComponent("EC_Name"))
            entity_name = dynamic_cast<EC_Name*>(entity->GetComponent("EC_Name").get())->name.Get();

        int row = AddUniqueListItem(entity.get(), entity_list_, entity_name);
        QListWidgetItem *item = entity_list_->item(row);
        if (select_all)
            item->setSelected(true);
    }
    entity_list_->blockSignals(false);
    RefreshPropertyBrowser();
}

void ECEditorWindow::RemoveEntity(entity_id_t entity_id, bool udpate_ui)
{
    PROFILE(RemoveEntity);
    if (!entity_list_)
        return;

    entity_list_->blockSignals(true);
    EntityPtr entity = framework_->Scene()->GetDefaultScene()->GetEntity(entity_id);
    if (!entity)
    {
        LogError("Fail to remove entity, since scene don't contain entity by ID:" + ToString<entity_id_t>(entity_id));
        return;
    }

    for(uint i = 0; i < (uint)entity_list_->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
        if (item && item->GetEntity() && item->GetEntity().data() == entity.get())
        {
            entity_list_->removeItemWidget(item);
            SAFE_DELETE(item)
            break;
        }
    }
    entity_list_->blockSignals(false);
    if (udpate_ui)
        RefreshPropertyBrowser();
}

void ECEditorWindow::SetSelectedEntities(const QList<entity_id_t> &ids)
{
    PROFILE(SetSelectedEntities);
    if (!entity_list_)
        return;

    entity_list_->blockSignals(true);
    foreach(entity_id_t id, ids)
        for(uint i = 0; i < (uint)entity_list_->count(); ++i)
        {
            QListWidgetItem *item = entity_list_->item(i);
            if (id == (entity_id_t)item->text().toInt())
            {
                item->setSelected(true);
                break;
            }
        }
    entity_list_->blockSignals(false);
    RefreshPropertyBrowser();
}

void ECEditorWindow::ClearEntities()
{
    if (entity_list_)
        entity_list_->clear();
    RefreshPropertyBrowser();
}

QObjectList ECEditorWindow::GetSelectedComponents() const
{
    if (browser_)
        return browser_->GetSelectedComponents();
    return QObjectList();
}

void ECEditorWindow::DeleteEntitiesFromList()
{
    if ((entity_list_) && (entity_list_->hasFocus()))
        for(int i = entity_list_->count() - 1; i >= 0; --i)
            if (entity_list_->item(i)->isSelected())
            {
                QListWidgetItem* item = entity_list_->takeItem(i);
                delete item;
            }
}

void ECEditorWindow::DeleteComponent(const QString &componentType, const QString &name)
{
    if(componentType.isEmpty())
        return;

    foreach(EntityPtr entity, GetSelectedEntities())
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
        component_dialog_ = new AddComponentDialog(framework_, ids, NULL);
        component_dialog_->SetComponentList(framework_->GetComponentManager()->GetAvailableComponentTypeNames());
        connect(component_dialog_, SIGNAL(finished(int)), this, SLOT(ComponentDialogFinished(int)));
        component_dialog_->show();
    }
}

void ECEditorWindow::ActionTriggered(Entity *entity, const QString &action, const QStringList &params)
{
    if (params.size() && action == "MousePress")
    {
        MouseEvent::MouseButton mouse_event = static_cast<MouseEvent::MouseButton>(params[0].toUInt());
        if (has_focus_ && isVisible() && mouse_event == MouseEvent::LeftButton)
        {
            AddEntity(entity->GetId());
        }
    }
}

void ECEditorWindow::DeleteEntity()
{
    ScenePtr scene = framework_->Scene()->GetDefaultScene();
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

    foreach(EntityPtr entity, GetSelectedEntities())
    {
        Entity *e = entity.get();
        if (e)
        {
            QDomElement entity_elem = temp_doc.createElement("entity");
            entity_elem.setAttribute("id", QString::number((int)entity->GetId()));

            foreach(ComponentPtr component, entity->Components())
                component->SerializeTo(temp_doc, entity_elem);

            temp_doc.appendChild(entity_elem);
        }
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
    ScenePtr scene = framework_->Scene()->GetDefaultScene();
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
            EntityPlacer *entityPlacer = new EntityPlacer(framework_, entity->GetId(), this);
            entityPlacer->setObjectName("EntityPlacer");
        }
        AddEntity(entity->GetId());
        scene->EmitEntityCreated(entity);
    }
}

void ECEditorWindow::OpenEntityActionDialog()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    if (!action)
        return;

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

void ECEditorWindow::EntityActionDialogFinished(int result)
{
    EntityActionDialog *dialog = qobject_cast<EntityActionDialog *>(sender());
    if (!dialog)
        return;

    if (result == QDialog::Rejected)
        return;

    foreach(EntityWeakPtr e, dialog->Entities())
        if (e.lock())
            e.lock()->Exec(dialog->ExecutionType(), dialog->Action(), dialog->Parameters());
}

void ECEditorWindow::OpenFunctionDialog()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    assert(action);
    if (!action)
        return;

    QObjectWeakPtrList objs;
    foreach(EntityPtr entity, GetSelectedEntities())
        objs << boost::dynamic_pointer_cast<QObject>(entity);

    if (objs.empty())
        return;

    FunctionDialog *d = new FunctionDialog(objs, this);
    connect(d, SIGNAL(finished(int)), this, SLOT(FunctionDialogFinished(int)));
    d->show();
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

    foreach(QObjectWeakPtr o, dialog->Objects())
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
            invoker.Invoke(obj, dialog->Function(), &ret, params, &errorMsg);

            if (errorMsg.isEmpty())
                dialog->AppendReturnValueText(objNameWithId + ' ' + ret.toString());
            else
                dialog->AppendReturnValueText(objNameWithId + ' ' + errorMsg);
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
    if (!browser_)
        return;

    ScenePtr scene = framework_->Scene()->GetDefaultScene();
    if (!scene)
    {
        browser_->clear();
        return;
    }

    QSet<entity_id_t> emtpty_list; 
    BoldEntityListItems(emtpty_list);

    QList<EntityPtr> entities = GetSelectedEntities();
    // If any of enities was not selected clear the browser window.
    if (!entities.size())
    {
        browser_->clear();
        return;
    }

    QList<EntityPtr> old_entities = browser_->GetEntities();
    qStableSort(entities.begin(), entities.end(), CmpEntityById);
    qStableSort(old_entities.begin(), old_entities.end(), CmpEntityById);
    // Check what entities need to get removed/added to browser.
    QList<EntityPtr>::iterator iter1 = old_entities.begin();
    QList<EntityPtr>::iterator iter2 = entities.begin();
    while(iter1 != old_entities.end() || iter2 != entities.end())
    {
        // No point to continue the iteration if old_entities list is empty. We can just push all new entitites into the browser.
        if (iter1 == old_entities.end())
        {
            for(;iter2 != entities.end(); iter2++)
                browser_->AddEntity(*iter2);
            break;
        }
        // Only old entities are left and they can just be removed from the browser.
        else if(iter2 == entities.end())
        {
            for(;iter1 != old_entities.end(); iter1++)
                browser_->RemoveEntity(*iter1);
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
            browser_->AddEntity(*iter2);
            iter2++;
        }
        // Couldn't find entity in new entities list, so it need to be removed from the browser.
        else
        {
            browser_->RemoveEntity(*iter1);
            iter1++;
        }
    }
    browser_->UpdateBrowser();
}

void ECEditorWindow::ShowEntityContextMenu(const QPoint &pos)
{
    assert(entity_list_);
    if (!entity_list_)
        return;

    QListWidgetItem *item = entity_list_->itemAt(pos);
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

    menu->popup(entity_list_->mapToGlobal(pos));
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

void ECEditorWindow::ShowXmlEditorForComponent(std::vector<ComponentPtr> components)
{
    if(!components.size())
        return;

    QList<ComponentPtr> comps;
    foreach(ComponentPtr component, components)
        comps << component;

    emit EditComponentXml(comps);
}

void ECEditorWindow::ShowXmlEditorForComponent(const std::string &componentType)
{
    if(componentType.empty())
        return;

    foreach(EntityPtr entity, GetSelectedEntities())
    {
        ComponentPtr component = entity->GetComponent(QString::fromStdString(componentType));
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
            if (toggle_entities_button_)
                toggle_entities_button_->setText(tr("Show Entities"));
        }
        else
        {
            entity_widget->show();
            resize(size().width() + entity_widget->sizeHint().width(), size().height());
            if (toggle_entities_button_)
                toggle_entities_button_->setText(tr("Hide Entities"));
        }
    }
}

void ECEditorWindow::EntityRemoved(Entity* entity)
{
    for(uint i = 0; i < (uint)entity_list_->count(); i++)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
        if (item->GetEntity().data() == entity)
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void ECEditorWindow::SetFocus(bool focus)
{
    has_focus_ = focus;
}

void ECEditorWindow::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible)
        emit OnFocusChanged(this);
}

void ECEditorWindow::hideEvent(QHideEvent* hide_event)
{
    ClearEntities();
    if(browser_)
        browser_->clear();
    QWidget::hideEvent(hide_event);
} 

void ECEditorWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange)
        setWindowTitle(QApplication::translate("ECEditor", "Entity-Component Editor"));
    else
       QWidget::changeEvent(e);
}

bool ECEditorWindow::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    if (type == QEvent::WindowActivate)
        emit OnFocusChanged(this);
    return QWidget::eventFilter(obj, event);
}

void ECEditorWindow::BoldEntityListItems(const QSet<entity_id_t> &bolded_entities)
{
    PROFILE(BoldEntityListItems);
    QSet<entity_id_t> old_enitities;
    EntityListWidgetItem *item = 0;
    for(uint i = 0; i < (uint)entity_list_->count(); ++i)
    {
        item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
        if (item)
        {
            QPointer<Entity> ent = item->GetEntity();
            QFont font = item->font();
            if (!ent.isNull() && bolded_entities.contains(ent.data()->GetId()))
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

void ECEditorWindow::Initialize()
{
    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile file(Application::InstallationDirectory() + "data/ui/eceditor.ui");
    file.open(QFile::ReadOnly);
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

    toggle_entities_button_ = findChild<QPushButton *>("but_show_entities");
    entity_list_ = findChild<QListWidget*>("list_entities");
    QWidget *entity_widget = findChild<QWidget*>("entity_widget");
    if(entity_widget)
        entity_widget->hide();

    QWidget *browserWidget = findChild<QWidget*>("browser_widget");
    if(browserWidget)
    {
        browser_ = new ECBrowser(framework_, browserWidget);
        browser_->setMinimumWidth(100);
        QVBoxLayout *property_layout = dynamic_cast<QVBoxLayout *>(browserWidget->layout());
        if (property_layout)
            property_layout->addWidget(browser_);
    }

    if(browser_)
    {
        // signals from attribute browser to editor window.
        connect(browser_, SIGNAL(ShowXmlEditorForComponent(const std::string &)), SLOT(ShowXmlEditorForComponent(const std::string &)));
        connect(browser_, SIGNAL(CreateNewComponent()), SLOT(CreateComponent()));
        connect(browser_, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)), SLOT(HighlightEntities(const QString&, const QString&)));
        connect(browser_, SIGNAL(SelectionChanged(const QString&, const QString &, const QString&, const QString&)),
                SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)), Qt::UniqueConnection);
        browser_->SetItemExpandMemory(framework_->GetModule<ECEditorModule>()->ExpandMemory());
    }

    if (entity_list_)
    {
        entity_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
        connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshPropertyBrowser()));
        connect(entity_list_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowEntityContextMenu(const QPoint &)));
    }

    if (toggle_entities_button_)
        connect(toggle_entities_button_, SIGNAL(pressed()), this, SLOT(ToggleEntityList()));

    // Default world scene is not added yet, so we need to listen when framework will send a DefaultWorldSceneChanged signal.
    connect(framework_->Scene(), SIGNAL(DefaultWorldSceneChanged(SceneManager *)), SLOT(DefaultSceneChanged(SceneManager *)));

    ECEditorModule *module = framework_->GetModule<ECEditorModule>();
    if (module)
        connect(this, SIGNAL(OnFocusChanged(ECEditorWindow *)), module, SLOT(ECEditorFocusChanged(ECEditorWindow*)));

    SceneManager *scene = framework_->Scene()->GetDefaultScene().get();
    if (scene)
    {
        connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), 
            SLOT(EntityRemoved(Entity*)), Qt::UniqueConnection);
        connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionType)),
                SLOT(ActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);
    }
}

void ECEditorWindow::DefaultSceneChanged(SceneManager *scene)
{
    assert(scene);
    /// todo disconnect previous scene connection.
    connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), 
            SLOT(EntityRemoved(Entity*)), Qt::UniqueConnection);
    connect(scene, SIGNAL(ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionType)),
            SLOT(ActionTriggered(Entity *, const QString &, const QStringList &)), Qt::UniqueConnection);
}

void ECEditorWindow::ComponentDialogFinished(int result)
{
    AddComponentDialog *dialog = qobject_cast<AddComponentDialog*>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    ScenePtr scene = framework_->Scene()->GetDefaultScene();
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

        comp = framework_->GetComponentManager()->CreateComponent(dialog->GetTypeName(), dialog->GetName());
        assert(comp);
        if (comp)
        {
            comp->SetNetworkSyncEnabled(dialog->GetSynchronization());
            comp->SetTemporary(dialog->GetTemporary());
            entity->AddComponent(comp, AttributeChange::Default);
        }
    }
}

QList<EntityPtr> ECEditorWindow::GetSelectedEntities() const
{
    QList<EntityPtr> ret;

    if (!entity_list_)
        return ret;

    ScenePtr scene = framework_->Scene()->GetDefaultScene();
    if (!scene)
        return ret;

    for(uint i = 0; i < (uint)entity_list_->count(); ++i)
    {
        EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
        if (item && item->GetEntity() && item->isSelected())
        {
            EntityPtr entity = scene->GetEntity(item->GetEntity()->GetId());
            if (entity)
                ret.push_back(entity);
        }
    }
    return ret;
}
