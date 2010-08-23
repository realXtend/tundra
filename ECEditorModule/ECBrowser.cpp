// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ECBrowser.h"
#include "ComponentInterface.h"
#include "ECComponentEditor.h"
#include "SceneManager.h"
#include "ECEditorModule.h"
#include "Framework.h"
#include "EC_DynamicComponent.h"
#include "ECEditorModule.h"

#include <QtBrowserItem>
#include <QLayout>
#include <QShortcut>
#include <QMenu>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    ECBrowser::ECBrowser(Foundation::Framework *framework, QWidget *parent): 
        QtTreePropertyBrowser(parent),
        menu_(0),
        treeWidget_(0),
        framework_(framework)
    {
        InitBrowser();
    }

    ECBrowser::~ECBrowser()
    {
        clear();
    }

    void ECBrowser::AddNewEntity(Scene::Entity *entity)
    {
        assert(entity);
        if(!entity)
            return;
        if(selectedEntities_.find(entity) != selectedEntities_.end())
            return;

        selectedEntities_.insert(entity);

        QObject::connect(entity->GetScene(),
                         SIGNAL(ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)),
                         this,
                         SLOT(NewComponentAdded(Scene::Entity*, Foundation::ComponentInterface*)));

        QObject::connect(entity->GetScene(),
                         SIGNAL(ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)),
                         this,
                         SLOT(ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*)));
    }

    void ECBrowser::RemoveEntity(Scene::Entity *entity)
    {
        if(!entity)
            return;
        if(selectedEntities_.find(entity) == selectedEntities_.end())
            return;

        Scene::Entity::ComponentVector components = entity->GetComponentVector();
        for(uint i = 0; i < components.size(); i++)
            RemoveComponentFromGroup(components[i].get());
        selectedEntities_.erase(entity);
    }

    void ECBrowser::clear()
    {
        while(!componentGroups_.empty())
        {
            SAFE_DELETE(componentGroups_.back())
            componentGroups_.pop_back();
        }
        QtTreePropertyBrowser::clear();
    }

    void ECBrowser::UpdateBrowser()
    {
        EntitySet::iterator iter = selectedEntities_.begin();
        for(;iter != selectedEntities_.end(); iter++)
        {
            const Scene::Entity::ComponentVector components = (*iter)->GetComponentVector();
            for(uint i = 0; i < components.size(); i++)
            {
                AddNewComponentToGroup(components[i]);
            }
        }

        // Update component editors ui.
        ComponentGroupList::iterator compIter = componentGroups_.begin();
        for(; compIter != componentGroups_.end(); compIter++)
        {
            (*compIter)->editor_->UpdateEditorUI();
        }
    }

    void ECBrowser::InitBrowser()
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowComponentContextMenu(const QPoint &)));
        treeWidget_ = findChild<QTreeWidget *>();
        treeWidget_->setSortingEnabled(true);
        if(treeWidget_)
        {
            //treeWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            treeWidget_->setFocusPolicy(Qt::StrongFocus);
            connect(treeWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
            QShortcut* delete_shortcut = new QShortcut(Qt::Key_Delete, treeWidget_);
            QShortcut* copy_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), treeWidget_);
            QShortcut* paste_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), treeWidget_);
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteComponent()));
            connect(copy_shortcut, SIGNAL(activated()), this, SLOT(CopyComponent()));
            connect(paste_shortcut, SIGNAL(activated()), this, SLOT(PasteComponent()));
        }
    }

    void ECBrowser::ShowComponentContextMenu(const QPoint &pos)
    {
        //! @todo position should be converted to treeWidget's space so that editor will select the right component when user right clicks on the browser.
        //! right now position is bit off and wrong item is selected. Included fast fix that wont make it work perfectly.
        if(!treeWidget_)
            return;

        /*QPoint globalPos = mapToGlobal(pos);
        QPoint point = treeWidget_->mapFromGlobal(globalPos);*/
        QTreeWidgetItem *treeWidgetItem = treeWidget_->itemAt(pos.x(), pos.y() - 20);
        if(treeWidgetItem)
            treeWidget_->setCurrentItem(treeWidgetItem);
        else
            treeWidget_->setCurrentItem(0);

        SAFE_DELETE(menu_);
        menu_ = new QMenu(this);
        menu_->setAttribute(Qt::WA_DeleteOnClose);
        if(treeWidgetItem)
        {
            QAction *copyComponent = new QAction(tr("Copy"), menu_);
            QAction *pasteComponent = new QAction(tr("Paste"), menu_);
            QAction *editXml= new QAction(tr("Edit XML..."), menu_);
            //Delete action functionality can vary based on what QTreeWidgetItem is selected on the browser.
            //If root item is selected we assume that we want to remove component and if attributes root node is selected we want to remove that attribute instead.
            QAction *deleteAction= new QAction(tr("Delete"), menu_);

            //Add shortcuts for actions
            copyComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
            pasteComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
            deleteAction->setShortcut(QKeySequence::Delete);

            QTreeWidgetItem *parentItem = treeWidgetItem;
            while(parentItem->parent())
                parentItem = parentItem->parent();
            ComponentGroupList::iterator iter = FindSuitableGroup(*parentItem);

            if(parentItem == treeWidgetItem)
            {
                QObject::connect(copyComponent, SIGNAL(triggered()), this, SLOT(CopyComponent()));
                QObject::connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()));
                QObject::connect(editXml, SIGNAL(triggered()), this, SLOT(OpenComponentXmlEditor()));
                menu_->addAction(copyComponent);
                menu_->addAction(pasteComponent);
                menu_->addAction(editXml);
            }
            QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(DeleteComponent()));
            menu_->addAction(deleteAction);

            if(iter != componentGroups_.end())
            {
                if((*iter)->isDynamic_)
                {
                    // Check if the selected tree widget name is same as some of the dynamic component's attribute name.
                    if((*iter)->ContainAttribute(treeWidgetItem->text(0).toStdString()))
                    {
                        QObject::disconnect(deleteAction, SIGNAL(triggered()), this, SLOT(DeleteComponent()));
                        QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(RemoveAttribute()));
                    }
                    QAction *addAttribute = new QAction(tr("Add new attribute"), menu_);
                    QObject::connect(addAttribute, SIGNAL(triggered()), this, SLOT(CreateAttribute()));
                    menu_->addAction(addAttribute);
                }
            }
        }
        else
        {
            QAction *addComponent = new QAction(tr("Add new component"), menu_);
            QAction *pasteComponent = new QAction(tr("Paste"), menu_);
            menu_->addAction(addComponent);
            menu_->addAction(pasteComponent);
            QObject::connect(addComponent, SIGNAL(triggered()), this, SIGNAL(CreateNewComponent()));
            QObject::connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()));
        }
        menu_->popup(mapToGlobal(pos));
    }
    
    void ECBrowser::SelectionChanged()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;
        // Go back to the root node.
        while(item->parent())
            item = item->parent();
        
        ComponentGroupList::iterator iter = FindSuitableGroup(*item);
        if(iter != componentGroups_.end())
        {
            for(uint i = 0; i < (*iter)->components_.size(); i++)
            {
                if((*iter)->components_[i].expired())
                    continue;
                emit ComponentSelected((*iter)->components_[i].lock().get());
            }
        }
    }
    
    void ECBrowser::NewComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp) 
    {
        EntitySet::iterator iter = selectedEntities_.find(entity);
        // We aren't interested in entities that aren't selected.
        if(iter == selectedEntities_.end())
            return;

        ComponentGroupList::iterator iterComp = componentGroups_.begin();
        for(; iterComp != componentGroups_.end(); iterComp++)
        {
            if((*iterComp)->ContainComponent(comp))
                return;
        }
        Foundation::ComponentInterfacePtr componentPtr = entity->GetComponent(comp->TypeName(), comp->Name());
        assert(componentPtr.get());
        AddNewComponentToGroup(componentPtr);
    }

    void ECBrowser::ComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp)
    {
        EntitySet::iterator iter = selectedEntities_.find(entity);
        // We aren't interested in entities that aren't selected.
        if(iter == selectedEntities_.end())
            return;

        ComponentGroupList::iterator iterComp = componentGroups_.begin();
        for(; iterComp != componentGroups_.end(); iterComp++)
        {
            if(!(*iterComp)->ContainComponent(comp))
                continue;
            
            Foundation::ComponentInterfacePtr componentPtr = entity->GetComponent(comp->TypeName(), comp->Name());
            assert(componentPtr.get());
            RemoveComponentFromGroup(comp);
            return;
        }
    }

    
    void ECBrowser::OpenComponentXmlEditor()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(item == (*iter)->browserListItem_)
            {
                if(!(*iter)->components_.size())
                    return;
                Foundation::ComponentWeakPtr pointer = (*iter)->components_[0];
                if(!pointer.expired())
                {
                    emit ShowXmlEditorForComponent(pointer.lock()->TypeName().toStdString());
                    break;
                }
            }
        }
    }

    void ECBrowser::CopyComponent()
    {
        QDomDocument temp_doc;
        QDomElement entity_elem;
        QClipboard *clipboard = QApplication::clipboard();

        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(item == (*iter)->browserListItem_)
            {
                if(!(*iter)->components_.size())
                    return;
                // Just take a first component from the componentgroup and copy its attributes to xml. 
                // Note! wont take account that other components might have different values in their attributes
                Foundation::ComponentWeakPtr pointer = (*iter)->components_[0];
                if(!pointer.expired())
                {
                    pointer.lock()->SerializeTo(temp_doc, entity_elem);
                    QString xmlText = temp_doc.toString();
                    clipboard->setText(xmlText);
                    break;
                }
            }
        }
    }

    void ECBrowser::PasteComponent()
    {
        QDomDocument temp_doc;
        QClipboard *clipboard = QApplication::clipboard();
        if (temp_doc.setContent(clipboard->text()))
        {
            // Only single component can be pasted.
            //! @todo add suport to multi component copy/paste feature.
            QDomElement comp_elem = temp_doc.firstChildElement("component");
            EntitySet::iterator iter = selectedEntities_.begin();
            while(iter != selectedEntities_.end())
            {
                Foundation::ComponentInterfacePtr component;
                QString type = comp_elem.attribute("type");
                QString name = comp_elem.attribute("name");
                if(!(*iter)->HasComponent(type, name))
                {
                    component = framework_->GetComponentManager()->CreateComponent(type, name);
                    (*iter)->AddComponent(component, AttributeChange::Local);
                }
                else
                    component = (*iter)->GetComponent(type, name);
                if(!component.get())
                {
                    iter++;
                    continue;
                }
                component->DeserializeFrom(comp_elem, AttributeChange::Local);
                component->ComponentChanged(AttributeChange::Local);
                iter++;
            }
        }
    }

    void ECBrowser::DeleteComponent()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            ComponentGroup *componentGroup = (*iter);
            if(item != componentGroup->browserListItem_)
                continue;

            while(!componentGroup->components_.empty())
            {
                Foundation::ComponentWeakPtr pointer = componentGroup->components_.back();
                if(!pointer.expired())
                {
                    Foundation::ComponentInterfacePtr comp = pointer.lock();
                    Scene::Entity *entity = comp->GetParentEntity();
                    entity->RemoveComponent(comp, AttributeChange::Local);
                }
                else
                {
                    // If component has been expired no point to keep in the component group
                    componentGroup->components_.pop_back();
                }
            }
            break;
        }
    }
    
    void ECBrowser::DynamicComponentChanged(const QString &name)
    {
        EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(sender());
        if(!component)
            return;

        Scene::Entity *entity = component->GetParentEntity();
        Foundation::ComponentInterfacePtr compPtr = entity->GetComponent(component);
        RemoveComponentFromGroup(component);
        AddNewComponentToGroup(compPtr);
    }

    void ECBrowser::ComponentNameChanged(const std::string &newName)
    {
        Foundation::ComponentInterface *component = dynamic_cast<Foundation::ComponentInterface*>(sender());
        if(component)
        {
            RemoveComponentFromGroup(component);
            Scene::Entity *entity = component->GetParentEntity();
            if(!entity)
                return;
            Foundation::ComponentInterfacePtr compPtr = entity->GetComponent(component);
            AddNewComponentToGroup(compPtr);
        }
    }

    void ECBrowser::CreateAttribute()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        QTreeWidgetItem *rootItem = item;
        while(rootItem->parent())
            rootItem = rootItem->parent();

        ComponentGroupList::iterator iter = FindSuitableGroup(*rootItem);
        if(iter == componentGroups_.end())
            return;
        if(!(*iter)->IsDynamic())
            return;

        bool ok = false;
        QStringList attributeList;
        StringVector attributeTypes = framework_->GetComponentManager()->GetAttributeTypes();
        for(uint i = 0; i < attributeTypes.size(); i++)
            attributeList.push_back(QString::fromStdString(attributeTypes[i]));

        //! @todo replace this code with a one that will use Dialog::open method and listens a signal that will tell us when dialog is closed. Should be more safe way to get this done.
        QString typeName = QInputDialog::getItem(this, tr("Give attribute type"), tr("Typename:"), attributeList, 0, false, &ok);
        if (!ok)
            return;
        QString name = QInputDialog::getText(this, tr("Give attribute name"), tr("Name:"), QLineEdit::Normal, QString(), &ok);
        if (!ok)
            return;

        std::vector<Foundation::ComponentWeakPtr> components = (*iter)->components_;
        for(uint i = 0; i < components.size(); i++)
        {
            if(components[i].expired())
                continue;
            EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(components[i].lock().get());
            if(component)
            {
                if(component->CreateAttribute(typeName, name))
                    component->ComponentChanged("Local");
            }
        }
    }

    void ECBrowser::RemoveAttribute()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        QTreeWidgetItem *rootItem = item;
        while(rootItem->parent())
            rootItem = rootItem->parent();

        ComponentGroupList::iterator iter = FindSuitableGroup(*rootItem);
        if(iter == componentGroups_.end())
            return;
        if(!(*iter)->IsDynamic())
            return;

        std::vector<Foundation::ComponentWeakPtr> components = (*iter)->components_;
        for(uint i = 0; i < components.size(); i++)
        {
            if(components[i].expired())
                continue;
            EC_DynamicComponent *comp = dynamic_cast<EC_DynamicComponent*>(components[i].lock().get());
            if(comp)
            {
                comp->RemoveAttribute(item->text(0));
                comp->ComponentChanged("Local");
            }
        }
        //RemoveComponentGroup(*iter);
    }

    ComponentGroupList::iterator ECBrowser::FindSuitableGroup(const Foundation::ComponentInterface &comp)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if((*iter)->IsSameComponent(comp))
                return iter;
        }
        return iter;
    }

    ComponentGroupList::iterator ECBrowser::FindSuitableGroup(const QTreeWidgetItem &item)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if((*iter)->browserListItem_ == &item)
                return iter;
        }
        return iter;
    }

    void ECBrowser::AddNewComponentToGroup(Foundation::ComponentInterfacePtr comp)
    {
        assert(comp.get());
        if(!comp.get() && !treeWidget_)
            return;

        ComponentGroupList::iterator iter = FindSuitableGroup(*comp.get());
        if(iter != componentGroups_.end())
        {
            // No point to add same component multiple times.
            if((*iter)->ContainComponent(comp.get()))
                return;

            (*iter)->editor_->AddNewComponent(comp, false);
            (*iter)->components_.push_back(Foundation::ComponentWeakPtr(comp));
            if((*iter)->IsDynamic())
            {
                EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent*>(comp.get());
                connect(dynComp, SIGNAL(AttributeAdded(const QString &)), 
                        this, SLOT(DynamicComponentChanged(const QString &)));
                connect(dynComp, SIGNAL(AttributeRemoved(const QString &)), 
                        this, SLOT(DynamicComponentChanged(const QString &)));
                connect(dynComp, SIGNAL(OnComponentNameChanged(const std::string&)), 
                        this, SLOT(ComponentNameChanged(const std::string&)));
            }
            return;
        }

        QSet<QTreeWidgetItem*> oldList;
        QSet<QTreeWidgetItem*> newList;

        // Find a new QTreeWidgetItem from the browser and save the information to ComponentGroup object.
        for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
            oldList.insert(treeWidget_->topLevelItem(i));
        ECComponentEditor *componentEditor = new ECComponentEditor(comp, this);
        for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
            newList.insert(treeWidget_->topLevelItem(i));
        QSet<QTreeWidgetItem*> changeList = newList - oldList;
        QTreeWidgetItem *newItem = 0;
        if(changeList.size() == 1)
            newItem = (*changeList.begin());
        else
        {
            ECEditorModule::LogError("Added a new component editor, but for some reason the editor's widget was not added to browser widget."
                                     " Make sure that ECComponentEditor was intialized properly.");
            return;
        }

        bool dynamic = comp->TypeName() == "EC_DynamicComponent";
        if(dynamic)
        {
            EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent*>(comp.get());
            connect(dynComp, SIGNAL(AttributeAdded(const QString &)), 
                    this, SLOT(DynamicComponentChanged(const QString &)));
            connect(dynComp, SIGNAL(AttributeRemoved(const QString &)), 
                    this, SLOT(DynamicComponentChanged(const QString &)));
            connect(dynComp, SIGNAL(OnComponentNameChanged(const std::string&)), 
                    this, SLOT(ComponentNameChanged(const std::string&)));
        }
        ComponentGroup *compGroup = new ComponentGroup(comp, componentEditor, newItem, dynamic);
        if(compGroup)
            componentGroups_.push_back(compGroup);
    }

    void ECBrowser::RemoveComponentFromGroup(Foundation::ComponentInterface *comp)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(!(*iter)->ContainComponent(comp))
                continue;
            if((*iter)->IsDynamic())
            {
                EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp);
                disconnect(dynComp, SIGNAL(AttributeAdded(const QString &)), 
                           this, SLOT(DynamicComponentChanged(const QString &)));
                disconnect(dynComp, SIGNAL(AttributeRemoved(const QString &)), 
                        this, SLOT(DynamicComponentChanged(const QString &)));
                disconnect(dynComp, SIGNAL(OnComponentNameChanged(const std::string&)), 
                           this, SLOT(ComponentNameChanged(const std::string&)));
            }
            (*iter)->RemoveComponent(comp);
            //Ensure that coponent group is valid and if it's not, remove it from the browser list.
            if(!(*iter)->IsValid())
            {
                SAFE_DELETE(*iter)
                componentGroups_.erase(iter);
            }
            break;
        }
    }

    void ECBrowser::RemoveComponentGroup(ComponentGroup *componentGroup)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(componentGroup != (*iter))
                continue;

            SAFE_DELETE(*iter)
            componentGroups_.erase(iter);
            break;
        }
    }
}