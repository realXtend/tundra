/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.cpp
 *  @brief  Entity-component editor window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ECBrowser.h"
#include "EntityPlacer.h"
#include "AddComponentDialog.h"

#include "ModuleManager.h"
#include "SceneManager.h"
#include "EC_Name.h"
#include "ComponentManager.h"
#include "XMLUtilities.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "EC_Placeable.h"
#include "Input.h"

#include <QUiLoader>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

using namespace RexTypes;

namespace ECEditor
{
    //! @bug code below causes a crash if user relog into the server and try to use ECEditorWindow.
    uint AddUniqueListItem(Scene::Entity *entity, QListWidget* list, const QString& name)
    {
        for (int i = 0; i < list->count(); ++i)
        {
            EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(list->item(i));
            if (item && !item->GetEntity().isNull() && item->GetEntity().data() == entity)
                return i;
        }

        EntityListWidgetItem *new_item = new EntityListWidgetItem(name, list, entity);
        return list->count() - 1;
    }

    uint AddTreeItem(QTreeWidget *list, const QString &type_name, const QString &name, int entity_id)
    {
        for(int i = 0; i < list->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *existing = list->topLevelItem(i);
            if (existing && existing->text(0) == type_name)
            {
                // We have already item for this EC. Create a dummy parent for the existing EC item and
                // the new one we're adding if it's not already.
                ///\todo Check/test if the code block below is required for real.
                if (existing->text(2) == "(Multiple)")
                {
                    // It's already dummy parent. Add new item to its child.
                    QTreeWidgetItem *item = new QTreeWidgetItem(existing);
                    item->setText(0, type_name);
                    item->setText(1, name);
                    item->setText(2, QString::number(entity_id));
                    existing->addChild(item);
                    return i;
                }

                // The existing item is not dummy parent yet, make it now.
                QTreeWidgetItem *dummyParent = new QTreeWidgetItem(list);
                dummyParent->setText(0, type_name);
                dummyParent->setText(1, "");
                dummyParent->setText(2, "(Multiple)");

                // Relocate the existing item from the top level to a child of the dummy parent.
                existing = list->takeTopLevelItem(i);
                dummyParent->addChild(existing);
                list->addTopLevelItem(dummyParent);

                // Finally, create new item for this EC.
                QTreeWidgetItem *item = new QTreeWidgetItem(dummyParent);
                item->setText(0, type_name);
                item->setText(1, name);
                item->setText(2, QString::number(entity_id));
                dummyParent->addChild(item);
                return i;
            }
        }

        // No existing top level item, create one now.
        QTreeWidgetItem *item = new QTreeWidgetItem(list);
        item->setText(0, type_name);
        item->setText(1, name);
        item->setText(2, QString::number(entity_id));
        list->addTopLevelItem(item);
        return list->topLevelItemCount() - 1;
    }

    //! Function that is used by std::sort algorithm to sort entities by their ids.
    bool CmpEntityById(Scene::EntityPtr a, Scene::EntityPtr b)
    {
        return a->GetId() < b->GetId();
    }

    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        QWidget(),
        framework_(framework),
        toggle_entities_button_(0),
        entity_list_(0),
        browser_(0),
        component_dialog_(0)
    {
        Initialize();
    }

    ECEditorWindow::~ECEditorWindow()
    {
    }

    void ECEditorWindow::AddEntity(entity_id_t entity_id)
    {
        if (entity_list_)
        {
            //If entity don't have EC_Name then entity_name is same as it's id.
            QString entity_name = QString::number(entity_id);
            Scene::EntityPtr entity = framework_->GetDefaultWorldScene()->GetEntity(entity_id);
            if(entity && entity->HasComponent("EC_Name"))
                entity_name = dynamic_cast<EC_Name*>(entity->GetComponent("EC_Name").get())->name.Get();

            //! @todo This will now work if we loose windows focus and previos key state stays, replace this with InputContext.
            if(!framework_->GetInput()->IsKeyDown(Qt::Key_Control))
                entity_list_->clearSelection();

            int row = AddUniqueListItem(entity.get(), entity_list_, entity_name);
            QListWidgetItem *item = entity_list_->item(row);
            // Toggle selection.
            item->setSelected(!item->isSelected());
        }
    }

    void ECEditorWindow::RemoveEntity(entity_id_t entity_id)
    {
        if (!entity_list_)
            return;

        Scene::EntityPtr entity = framework_->GetDefaultWorldScene()->GetEntity(entity_id);
        if (!entity)
        {
            ECEditorModule::LogError("Fail to remove entity, since scene don't contain entity by ID:" + ToString<entity_id_t>(entity_id));
            return;
        }

        for(uint i = 0; i < entity_list_->count(); i++)
        {
            EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
            if (item && item->GetEntity() && item->GetEntity().data() == entity.get())
            {
                entity_list_->removeItemWidget(item);
                SAFE_DELETE(item)
                break;
            }
        }
    }

    void ECEditorWindow::ClearEntities()
    {
        if (entity_list_)
            entity_list_->clear();
        RefreshPropertyBrowser();
    }

    void ECEditorWindow::DeleteEntitiesFromList()
    {
        if ((entity_list_) && (entity_list_->hasFocus()))
        {
            for (uint i = entity_list_->count(); i--;)
            {
                if (entity_list_->item(i)->isSelected())
                {
                    QListWidgetItem* item = entity_list_->takeItem(i);
                    delete item;
                }
            }
        }
    }

    void ECEditorWindow::DeleteComponent(const QString &componentType, const QString &name)
    {
        if(componentType.isEmpty())
            return;

        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        for(uint i = 0; i < entities.size(); i++)
        {
            ComponentPtr component = entities[i]->GetComponent(componentType, name);
            if(component)
            {
                entities[i]->RemoveComponent(component, AttributeChange::Default);
                BoldEntityListItem(entities[i]->GetId(), false);
            }
        }
    }
    
    void ECEditorWindow::CreateComponent()
    {
        if(component_dialog_)
            component_dialog_->deleteLater();

        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        if(entities.size())//selectedEntities_.size())
        {
            QList<entity_id_t> entity_ids;
            for(uint i = 0; i < entities.size(); i++)
                entity_ids.push_back(entities[i]->GetId());

            component_dialog_ = new AddComponentDialog(framework_, entity_ids, this);
            component_dialog_->SetComponentList(GetAvailableComponents());
            connect(component_dialog_, SIGNAL(finished(int)), this, SLOT(ComponentDialogFinished(int)));
            component_dialog_->show();
        }
    }

    void ECEditorWindow::DeleteEntity()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        for(uint i = 0; i < entities.size(); ++i)
            scene->RemoveEntity(entities[i]->GetId(), AttributeChange::Default);
    }

    void ECEditorWindow::CopyEntity()
    {
        //! @todo will only take a copy of first entity of the selection. 
        //! should we support multi entity copy and paste functionality.
        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        QClipboard *clipboard = QApplication::clipboard();
        QDomDocument temp_doc;
        for(uint i = 0; i < entities.size(); i++)
        {
            Scene::Entity *entity = entities[i].get();
            if(entity)
            {
                QDomElement entity_elem = temp_doc.createElement("entity");
                
                QString id_str;
                id_str.setNum((int)entity->GetId());
                entity_elem.setAttribute("id", id_str);

                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); ++i)
                    if (components[i]->IsSerializable())
                        components[i]->SerializeTo(temp_doc, entity_elem);

                temp_doc.appendChild(entity_elem);
            }
        }
        clipboard->setText(temp_doc.toString());
    }

    void ECEditorWindow::PasteEntity()
    {
        // Dont allow paste operation if we are placing previosly pasted object to a scene.
        if(findChild<QObject*>("EntityPlacer"))
            return;
        // First we need to check if component is holding EC_OgrePlacable component to tell where entity should be located at.
        //! \todo local only server wont save those objects.
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        assert(scene.get());
        if(!scene.get())
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
            Scene::EntityPtr originalEntity = scene->GetEntity(ParseString<entity_id_t>(id.toStdString()));
            if(!originalEntity.get())
            {
                ECEditorModule::LogWarning("ECEditorWindow cannot create a new copy of entity, cause scene manager couldn't find entity. (id " + id.toStdString() + ").");
                return;
            }
            Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
            Scene::EntityPtr entity = scene->CreateEntity(framework_->GetDefaultWorldScene()->GetNextFreeId());
            assert(entity.get());
            if(!entity.get())
                return;

            bool hasPlaceable = false;
            Scene::Entity::ComponentVector components = originalEntity->GetComponentVector();
            for(uint i = 0; i < components.size(); i++)
            {
                // If the entity is holding placeable component we can place it into the scene.
                if(components[i]->TypeName() == "EC_Placeable")
                {
                    hasPlaceable = true;
                    ComponentPtr component = entity->GetOrCreateComponent(components[i]->TypeName(), components[i]->Name(), AttributeChange::Default);
                }
                // Ignore all nonserializable components.
                if(components[i]->IsSerializable())
                {
                    ComponentPtr component = entity->GetOrCreateComponent(components[i]->TypeName(), components[i]->Name(), AttributeChange::Default);
                    AttributeVector attributes = components[i]->GetAttributes();
                    for(uint j = 0; j < attributes.size(); j++)
                    {
                        IAttribute *attribute = component->GetAttribute(attributes[j]->GetNameString().c_str());
                        if(attribute)
                            attribute->FromString(attributes[j]->ToString(), AttributeChange::Default);
                    }
                    //component->ComponentChanged(AttributeChange::Default);
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

    void ECEditorWindow::HighlightEntities(IComponent *component)
    {
        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        for(uint i = 0; i < entities.size(); i++)
        {
            if(entities[i]->GetComponent(component->TypeName(), component->Name()))
                BoldEntityListItem(entities[i]->GetId(), true);
            else
                BoldEntityListItem(entities[i]->GetId(), false);
        }
    }

    void ECEditorWindow::RefreshPropertyBrowser() 
    {
        PROFILE(EC_refresh_browser);
        if(!browser_)
            return;

        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
        {
            browser_->clear();
            return;
        }

        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        // If any of enities was not selected clear the browser window.
        if(!entities.size())
        {
            browser_->clear();
            // Unbold all list elements from the enity list.
            for(uint i = 0; i < entity_list_->count(); i++)
            {
                EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
                if(item && item->GetEntity())
                    BoldEntityListItem(item->GetEntity()->GetId(), false);
            }
            return;
        }

        //! \todo hackish way to improve the browser's performance, by hiding the widget until all changes are made,
        //! so that unnecessary widget paints are avoided. This need be fixed in a way that browser's load is minimal.
        //! To ensure that the editor can handle thousands of induvicual elements in the same time.
        browser_->hide();
        QList<Scene::EntityPtr> old_entities = browser_->GetEntities();
        qStableSort(entities.begin(), entities.end(), CmpEntityById);
        qStableSort(old_entities.begin(), old_entities.end(), CmpEntityById);

        // Check what entities need to get removed and what need to get added to borwser.
        QList<Scene::EntityPtr>::iterator iter1 = old_entities.begin();
        QList<Scene::EntityPtr>::iterator iter2 = entities.begin();
        std::cout << "New List:" << std::endl;
        foreach(Scene::EntityPtr ent, entities)
            std::cout << ToString<entity_id_t>(ent->GetId()) << std::endl;
        std::cout << "Old List:" << std::endl;
        foreach(Scene::EntityPtr ent, old_entities)
            std::cout << ToString<entity_id_t>(ent->GetId()) << std::endl;

        while(iter1 != old_entities.end() || iter2 != entities.end())
        {
            // No point to continue the iteration if old_entities list is empty. We can just push all new entitites into the browser.
            if(iter1 == old_entities.end())
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

            // Entity has already added to browser.
            if((*iter1)->GetId() == (*iter2)->GetId())
            {
                iter2++;
                iter1++;
            }
            // Found new entity that that need to be added to browser.
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

        browser_->show();
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
        QAction *deleteEntity= new QAction(tr("Delete"), menu);
        QAction *addComponent = new QAction(tr("Add new component..."), menu);
        QAction *copyEntity = new QAction(tr("Copy"), menu);
        QAction *pasteEntity = new QAction(tr("Paste"), menu);

        connect(editXml, SIGNAL(triggered()), this, SLOT(ShowXmlEditorForEntity()));
        connect(deleteEntity, SIGNAL(triggered()), this, SLOT(DeleteEntity()));
        connect(addComponent, SIGNAL(triggered()), this, SLOT(CreateComponent()));
        connect(copyEntity, SIGNAL(triggered()), this, SLOT(CopyEntity()));
        connect(pasteEntity, SIGNAL(triggered()), this, SLOT(PasteEntity()));

        menu->addAction(editXml);
        menu->addAction(deleteEntity);
        menu->addAction(addComponent);
        menu->addAction(copyEntity);
        menu->addAction(pasteEntity);

        menu->popup(entity_list_->mapToGlobal(pos));
    }

    void ECEditorWindow::ShowXmlEditorForEntity()
    {
        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        std::vector<EntityComponentSelection> selection;// = GetSelectedComponents();
        for(uint i = 0; i < entities.size(); i++)
        {
            EntityComponentSelection entityComponent;
            entityComponent.entity = entities[i];
            entityComponent.components = entities[i]->GetComponentVector();
            selection.push_back(entityComponent);
        }

        if (!selection.size())
            return;

        QList<Scene::EntityPtr> ents;
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

        QList<Scene::EntityPtr> entities = GetSelectedEntities();
        for(uint i = 0; i < entities.size(); i++)
        {
            ComponentPtr component = entities[i]->GetComponent(QString::fromStdString(componentType));
            if(component)
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
                    toggle_entities_button_->setText(tr("Show entities"));
            }
            else
            {
                entity_widget->show();
                resize(size().width() + entity_widget->sizeHint().width(), size().height());
                if (toggle_entities_button_)
                    toggle_entities_button_->setText(tr("Hide entities"));
            }
        }
    }

    void ECEditorWindow::EntityRemoved(Scene::Entity* entity)
    {
        /*EntityIdSet::iterator iter = selectedEntities_.find(entity->GetId());
        if(iter != selectedEntities_.end())
            selectedEntities_.erase(iter);*/

        QList<QListWidgetItem*> items = entity_list_->findItems(QString::number(entity->GetId()), Qt::MatchExactly);
        for(uint i = 0; i < items.size(); i++)
            SAFE_DELETE(items[i]);
        //RefreshPropertyBrowser();
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
        {
            QString title = QApplication::translate("ECEditor", "Entity-component Editor");
            setWindowTitle(title);
        }
        else
           QWidget::changeEvent(e);
    }

    void ECEditorWindow::BoldEntityListItem(entity_id_t entity_id, bool bold)
    {
        if(!entity_list_)
        {
            ECEditorModule::LogError("Fail to bold QListWidgetItem, since entity list pointer is null.");
            return;
        }
        Scene::EntityPtr entity = framework_->GetDefaultWorldScene()->GetEntity(entity_id);
        if(!entity)
        {
            ECEditorModule::LogError("Fail to bold QListWidgetItem, since scene don't contain entity by ID:" + ToString<entity_id_t>(entity_id));
            return;
        }

        for(uint i = 0; i < entity_list_->count(); i++)
        {
            EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
            if(item && item->GetEntity() && item->GetEntity().data() == entity.get())
            {
                QFont font = item->font();
                font.setBold(bold);
                item->setFont(font);
                break;
            }
        }
    }

    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        QWidget *contents = loader.load(&file, this);
        if (!contents)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
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
            QObject::connect(browser_, SIGNAL(ShowXmlEditorForComponent(const std::string &)), this, SLOT(ShowXmlEditorForComponent(const std::string &)));
            QObject::connect(browser_, SIGNAL(CreateNewComponent()), this, SLOT(CreateComponent()));
            QObject::connect(browser_, SIGNAL(ComponentSelected(IComponent *)), 
                             this, SLOT(HighlightEntities(IComponent *)));
        }
/*
        if (component_list_ && browser_)
            connect(browser_, SIGNAL(AttributesChanged()), this, SLOT(RefreshComponentXmlData()));
*/
        if (entity_list_)
        {
            entity_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            QShortcut* delete_shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), entity_list_);
            QShortcut* copy_shortcut = new QShortcut(QKeySequence(Qt::Key_Control + Qt::Key_C), entity_list_);
            QShortcut* paste_shortcut = new QShortcut(QKeySequence(Qt::Key_Control + Qt::Key_V), entity_list_);
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteEntitiesFromList()));
            connect(copy_shortcut, SIGNAL(activated()), this, SLOT(CopyEntity()));
            connect(paste_shortcut, SIGNAL(activated()), this, SLOT(PasteEntity()));
            connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshPropertyBrowser()));
            connect(entity_list_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowEntityContextMenu(const QPoint &)));
        }

        if (toggle_entities_button_)
            connect(toggle_entities_button_, SIGNAL(pressed()), this, SLOT(ToggleEntityList()));

        // Scene is not added yet so we need to listen when it's added and we can connect scenemanager's EntityRemoved singal.
        connect(framework_, SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));
    }

    void ECEditorWindow::SceneAdded(const QString &name)
    {
        Scene::ScenePtr scenePtr = framework_->GetScene(name);
        if(scenePtr)
        {
            // If scene has already added no need to do multiple connection.
            disconnect(scenePtr.get(), SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)),
                       this, SLOT(EntityRemoved(Scene::Entity*)));
            connect(scenePtr.get(), SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), 
                    this, SLOT(EntityRemoved(Scene::Entity*)));
        }
    }

    void ECEditorWindow::ComponentDialogFinished(int result)
    {
        AddComponentDialog *dialog = qobject_cast<AddComponentDialog*>(sender());
        if(dialog)
        {
            if(result == QDialog::Accepted)
            {
                Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
                if(!scene)
                {
                    ECEditorModule::LogWarning("Fail to add new component to entity, since default world scene was null");
                    return;
                }
                QList<entity_id_t> entities = dialog->GetEntityIds();
                for(uint i = 0; i < entities.size(); i++)
                {
                    Scene::EntityPtr entity = scene->GetEntity(entities[i]);
                    if(!entity)
                    {
                        ECEditorModule::LogWarning("Fail to add new component to entity, since couldn't find a entity with ID:" + ::ToString<entity_id_t>(entities[i]));
                        continue;
                    }
                    ComponentPtr comp = entity->GetComponent(dialog->GetTypename(), dialog->GetName());
                    // Check if component has been already added to a entity.
                    if(comp)
                    {
                        ECEditorModule::LogWarning("Fail to add a new component, cause there was already a component with a same name and a type");
                        continue;
                    }
                    comp = framework_->GetComponentManager()->CreateComponent(dialog->GetTypename(), dialog->GetName());
                    comp->SetNetworkSyncEnabled(dialog->GetSynchronization());
                    if (comp)
                        entity->AddComponent(comp, AttributeChange::Default);
                }
            }
        }
    }

    QStringList ECEditorWindow::GetAvailableComponents() const
    {
        using namespace Foundation;
        QStringList components;
        ComponentManagerPtr comp_mgr = framework_->GetComponentManager();
        const ComponentManager::ComponentFactoryMap& factories = comp_mgr->GetComponentFactoryMap();
        ComponentManager::ComponentFactoryMap::const_iterator i = factories.begin();
        while (i != factories.end())
        {
            components.append(i->first); //<< i->first.c_str();
            ++i;
        }

        return components;
    }

    QList<Scene::EntityPtr> ECEditorWindow::GetSelectedEntities() const
    {
        QList<Scene::EntityPtr> ret;

        if (!entity_list_)
            return ret;

        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return ret;

        for (uint i = 0; i < entity_list_->count(); ++i)
        {
            EntityListWidgetItem *item = dynamic_cast<EntityListWidgetItem*>(entity_list_->item(i));
            if (item && item->GetEntity() && item->isSelected())
            {
                Scene::EntityPtr entity = scene->GetEntity(item->GetEntity()->GetId());
                if(entity)
                    ret.push_back(entity);
            }
        }
        return ret;
    }
}

