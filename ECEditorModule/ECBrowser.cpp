// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECBrowser.h"
#include "ECComponentEditor.h"
#include "TreeWidgetItemExpandMemory.h"

#include "SceneAPI.h"
#include "Entity.h"
#include "IComponent.h"
#include "SceneManager.h"
#include "Framework.h"
#include "EC_DynamicComponent.h"
#include "CoreTypes.h"
#include "LoggingFunctions.h"

//DEFINE_POCO_LOGGING_FUNCTIONS("ECBrowser")
#include <QtBrowserItem>
#include <QLayout>
#include <QShortcut>
#include <QMenu>
#include <QDomDocument>
#include <QMimeData>

#include "MemoryLeakCheck.h"

ECBrowser::ECBrowser(Foundation::Framework *framework, QWidget *parent):
    QtTreePropertyBrowser(parent),
    menu_(0),
    treeWidget_(0),
    framework_(framework)
{ 
    setMouseTracking(true);
    setAcceptDrops(true);
    setResizeMode(QtTreePropertyBrowser::Interactive);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(ShowComponentContextMenu(const QPoint &)));
    treeWidget_ = findChild<QTreeWidget *>();
    if(treeWidget_)
    {
        treeWidget_->setSortingEnabled(true);
        treeWidget_->setFocusPolicy(Qt::StrongFocus);
        treeWidget_->setAcceptDrops(true);
        treeWidget_->setDragDropMode(QAbstractItemView::DropOnly);
        QHeaderView *header = treeWidget_->header();
        if (header)
            header->setSortIndicator(0, Qt::AscendingOrder); 
        connect(treeWidget_, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(SelectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)), Qt::UniqueConnection);
    }

    QShortcut *delete_shortcut = new QShortcut(QKeySequence::Delete, this);
    QShortcut *copy_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this);
    QShortcut *paste_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
    connect(delete_shortcut, SIGNAL(activated()), SLOT(OnDeleteAction()), Qt::UniqueConnection);
    connect(copy_shortcut, SIGNAL(activated()), SLOT(CopyComponent()), Qt::UniqueConnection);
    connect(paste_shortcut, SIGNAL(activated()), SLOT(PasteComponent()), Qt::UniqueConnection);
}

ECBrowser::~ECBrowser()
{
    // Clear should release all ComponentGroup objects from the memory.
    clear();
}

void ECBrowser::AddEntity(Scene::EntityPtr entity)
{
    PROFILE(ECBrowser_AddNewEntity);

    assert(entity);
    if(!entity)
        return;

    //If entity is already added to browser no point to continue.
    if(HasEntity(entity))
        return;
    entities_.push_back(Scene::EntityPtr(entity));

    connect(entity.get(), SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)),
        SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
    connect(entity.get(), SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)),
        SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
}

void ECBrowser::RemoveEntity(Scene::EntityPtr entity)
{
    if (!entity)
        return;

    for(EntityWeakPtrList::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
        if (iter->lock() == entity)
        {
            Scene::EntityPtr ent_ptr = iter->lock();

            disconnect(entity.get(), SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this,
                SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
            disconnect(entity.get(), SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this,
                SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));

            Scene::Entity::ComponentVector components = ent_ptr->Components();
            for(uint i = 0; i < components.size(); i++)
                RemoveComponentFromGroup(components[i]);

            entities_.erase(iter);
            break;
        }
}

QList<Scene::EntityPtr> ECBrowser::GetEntities() const
{
    QList<Scene::EntityPtr> ret;
    for(uint i = 0; i < entities_.size(); i++)
        if(!entities_[i].expired())
            ret.push_back(entities_[i].lock());
    return ret;
}

QObjectList ECBrowser::GetSelectedComponents() const
{
    QTreeWidgetItem *item = treeWidget_->currentItem();
    if(!item)
        return QObjectList();
    // Go back to the root node.
    while(item->parent())
        item = item->parent();

    if (treeWidget_ && item)
    {
        TreeItemToComponentGroup::const_iterator iter = itemToComponentGroups_.find(item);
        if (iter != itemToComponentGroups_.end())
        {
            QObjectList components;
            for(uint i = 0; i < (*iter)->components_.size(); ++i)
            {
                IComponent *comp = (*iter)->components_[i].lock().get();
                if (comp)
                    components.push_back(comp);
            }
            return components;
        }
    }
    return QObjectList();
}

void ECBrowser::clear()
{
    for(TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
        iter != itemToComponentGroups_.end(); ++iter)
        SAFE_DELETE(iter.value())
    itemToComponentGroups_.clear();

    entities_.clear();
    QtTreePropertyBrowser::clear();
}

void ECBrowser::UpdateBrowser()
{
    PROFILE(ECBrowser_UpdateBrowser);

    // Sorting tends to be a heavy operation so we disable it until we have made all changes to a tree structure.
    if(treeWidget_)
        treeWidget_->setSortingEnabled(false);

    for(EntityWeakPtrList::iterator iter = entities_.begin(); iter != entities_.end(); iter++)
    {
        if((*iter).expired())
            continue;

        const Scene::Entity::ComponentVector components = (*iter).lock()->Components();
        for(uint i = 0; i < components.size(); i++)
            AddNewComponentToGroup(components[i]);
    }
    if(treeWidget_)
        treeWidget_->setSortingEnabled(true);

    for(TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
        iter != itemToComponentGroups_.end();
        iter++)
    {
        (*iter)->editor_->UpdateUi();
    }

}

void ECBrowser::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
    if (event->mimeData()->hasFormat("application/vnd.inventory.item") && item && !item->childCount())
        event->acceptProposedAction();
}

void ECBrowser::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
    if(item)
        dropMimeData(item, 0, event->mimeData(), event->dropAction());
}

void ECBrowser::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
        if(item && !item->childCount())
        {
            event->accept();
            return;
        }
    }
    event->ignore();
}

void ECBrowser::focusInEvent(QFocusEvent *event)
{
    QtTreePropertyBrowser::focusInEvent(event);
}

bool ECBrowser::dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action)
{
    return false;

    ///\todo Regression. Need to reimplement this so that we can drop any kind of strings to ECEditor and not just uuids. -jj.
#if 0
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.inventory.item"))
        return false;

    QByteArray encodedData = data->data("application/vnd.inventory.item");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QString asset_id;
    while(!stream.atEnd())
    {
        QString mimedata, asset_type, item_id, name;
        stream >> mimedata;

        QStringList list = mimedata.split(";", QString::SkipEmptyParts);
        if (list.size() < 4)
            continue;

        item_id = list.at(1);
        if (!RexUUID::IsValid(item_id.toStdString()))
            continue;

        name = list.at(2);
        asset_id = list.at(3);
    }

    if (!RexUUID::IsValid(asset_id.toStdString()))
        return false;

    QTreeWidgetItem *rootItem = item;
    while(rootItem->parent())
        rootItem = rootItem->parent();

    TreeItemToComponentGroup::iterator iter = componentGroups_.find(rootItem);
    if(iter != componentGroups_.end())
    {
        for(uint i = 0; i < (*iter)->components_.size(); i++)
        {
            ComponentWeakPtr compWeak = (*iter)->components_[i];
            if(compWeak.expired())
                continue;

            QStringList names;
            names << item->text(0);
            if(item->parent())
                names << item->parent()->text(0);

            // Try to find the right attribute.
            IAttribute *attr = 0;
            for(uint i = 0; i < names.size(); i++)
            {
                attr = compWeak.lock()->GetAttribute(names[i]);
                if(attr)
                    break;
            }
            if(!attr)
                continue;
            if(attr->TypeName() == "string")
            {
                Attribute<QString> *attribute = dynamic_cast<Attribute<QString> *>(attr);
                if(attribute)
                    attribute->Set(QString::fromStdString(asset_id.toStdString()), AttributeChange::Default);
            }
            else if(attr->TypeName() == "qvariant")
            {
                Attribute<QVariant> *attribute = dynamic_cast<Attribute<QVariant> *>(attr);
                if(attribute)
                {
                    if(attribute->Get().type() == QVariant::String)
                    {
                        attribute->Set(asset_id, AttributeChange::Default);
                    }
                }
            }
            else if(attr->TypeName() == "qvariantarray")
            {
                Attribute<std::vector<QVariant> > *attribute = dynamic_cast<Attribute<std::vector<QVariant> > *>(attr);
                if(attribute)
                {
                    // We asume that item's name is form of "[0]","[1]" etc. We need to cut down those first and last characters
                    // to able to get real index number of that item that is cause sorting can make the item order a bit odd.
                    QString indexText = "";
                    QString itemText = item->text(0);
                    for(uint i = 1; i < itemText.size() - 1; i++)
                        indexText += itemText[i];
                    bool ok;
                    int index = indexText.toInt(&ok);
                    if(!ok)
                        return false;

                    std::vector<QVariant> variants = attribute->Get();
                    if ((int)variants.size() > index)
                        variants[index] = asset_id;
                    else if((int)variants.size() == index)
                        variants.push_back(asset_id);
                    else
                        return false;

                    attribute->Set(variants, AttributeChange::Default);
                }
            }
            else if(attr->TypeName() == "qvariantlist")
            {
                Attribute<QVariantList > *attribute = dynamic_cast<Attribute<QVariantList > *>(attr);
                if(attribute)
                {
                    // We asume that item's name is form of "[0]","[1]" etc. We need to cut down those first and last characters
                    // to able to get real index number of that item that is cause sorting can make the item order a bit odd.
                    QString indexText = "";
                    QString itemText = item->text(0);
                    for(uint i = 1; i < itemText.size() - 1; i++)
                        indexText += itemText[i];
                    bool ok;
                    int index = indexText.toInt(&ok);
                    if(!ok)
                        return false;

                    QVariantList variants = attribute->Get();
                    if(variants.size() > index)
                        variants[index] = asset_id;
                    else if(variants.size() == index)
                        variants.push_back(asset_id);
                    else
                        return false;

                    attribute->Set(variants, AttributeChange::Default);
                }
            }
        }
    }
    else
        return false;

    return true;
#endif
}

void ECBrowser::ShowComponentContextMenu(const QPoint &pos)
{
    /// @todo Click position should be converted to treeWidget's space, so that editor will select the right
    /// QTreeWidget item, when user right clicks on the browser. right now position is bit off and wrong item 
    /// may get selected.
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
        // Delete action functionality can vary based on what QTreeWidgetItem is selected on the browser.
        // If root item is selected we assume that we want to remove component and non root items are attributes
        // that need to removed (attribute delete is only enabled with EC_DynamicComponent).
        QAction *deleteAction= new QAction(tr("Delete"), menu_);

        //Add shortcuts for actions
        copyComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
        pasteComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
        deleteAction->setShortcut(QKeySequence::Delete);

        QTreeWidgetItem *parentItem = treeWidgetItem;
        while(parentItem->parent())
            parentItem = parentItem->parent();
        TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(parentItem);//FindSuitableGroup(*parentItem);

        if(parentItem == treeWidgetItem)
        {
            connect(copyComponent, SIGNAL(triggered()), this, SLOT(CopyComponent()), Qt::UniqueConnection);
            connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()), Qt::UniqueConnection);
            connect(editXml, SIGNAL(triggered()), this, SLOT(OpenComponentXmlEditor()), Qt::UniqueConnection);
            menu_->addAction(copyComponent);
            menu_->addAction(pasteComponent);
            menu_->addAction(editXml);
        }
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(OnDeleteAction()), Qt::UniqueConnection);
        menu_->addAction(deleteAction);

        if(iter != itemToComponentGroups_.end())
        {
            if((*iter)->isDynamic_)
            {
                QAction *addAttribute = new QAction(tr("Add new attribute..."), menu_);
                QObject::connect(addAttribute, SIGNAL(triggered()), this, SLOT(CreateAttribute()), Qt::UniqueConnection);
                menu_->addAction(addAttribute);
            }
        }
    }
    else
    {
        QAction *addComponent = new QAction(tr("Add new component ..."), menu_);
        QAction *pasteComponent = new QAction(tr("Paste"), menu_);
        menu_->addAction(addComponent);
        menu_->addAction(pasteComponent);
        connect(addComponent, SIGNAL(triggered()), this, SIGNAL(CreateNewComponent()), Qt::UniqueConnection);
        connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()), Qt::UniqueConnection);
    }
    menu_->popup(mapToGlobal(pos));
}

void ECBrowser::SelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    PROFILE(ECBrowser_SelectionChanged);

    QTreeWidgetItem *item = current;
    QTreeWidgetItem *attribute_item = current;
    if(!item)
        return;
    // Go back to the root node.
    while(item->parent())
    {
        attribute_item = item;
        item = item->parent();
    }

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(item);
    if(iter != itemToComponentGroups_.end())
    {
        QString attributeName;
        QString attributeType;
        if ((*iter)->editor_)
        {
            attributeType = (*iter)->editor_->GetAttributeType(attribute_item->text(0));
            if (!attributeType.isNull())
                attributeName = attribute_item->text(0);
        }

        // Could add a loop that will continue to find a component that hasn't expired. 
        IComponent *comp = (*iter)->components_[0].lock().get();
        if (comp)
            emit SelectionChanged(comp->TypeName(), comp->Name(), attributeType, attributeName);
    }
}

void ECBrowser::OnComponentAdded(IComponent* comp, AttributeChange::Type type) 
{
    Scene::EntityPtr entity_ptr = framework_->Scene()->GetDefaultScene()->GetEntity(comp->GetParentEntity()->GetId());
    if(!HasEntity(entity_ptr))
        return;
    ComponentPtr comp_ptr;
    try
    {
        comp_ptr = comp->shared_from_this();
    } catch(...)
    {
        LogError("Fail to add new component to ECBroser. Make sure that component's parent entity is set.");
        return;
    }

    TreeItemToComponentGroup::iterator iterComp = itemToComponentGroups_.begin();
    for(; iterComp != itemToComponentGroups_.end(); iterComp++)
        if((*iterComp)->ContainsComponent(comp_ptr))
        {
            LogWarning("Fail to add new component to a component group, because component was already added.");
            return;
        }
    AddNewComponentToGroup(comp_ptr);

    ComponentGroup *group = FindSuitableGroup(comp_ptr);
    if (group)
        group->editor_->UpdateUi();
}

void ECBrowser::OnComponentRemoved(IComponent* comp, AttributeChange::Type type)
{
    Scene::EntityPtr entity_ptr = framework_->Scene()->GetDefaultScene()->GetEntity(comp->GetParentEntity()->GetId());
    if(!HasEntity(entity_ptr))
        return;

    ComponentPtr comp_ptr;

    try
    {
        comp_ptr = comp->shared_from_this();
    } catch(...)
    {
        LogError("Fail to remove component from ECBroser. Make sure that component's parent entity is set.");
        return;
    } 

    TreeItemToComponentGroup::iterator iterComp = itemToComponentGroups_.begin();
    for(; iterComp != itemToComponentGroups_.end(); iterComp++)
    {
        if(!(*iterComp)->ContainsComponent(comp_ptr))
            continue;
        RemoveComponentFromGroup(comp_ptr);
        return;
    }

    ComponentGroup *group = FindSuitableGroup(comp_ptr);
    if (group)
        group->editor_->UpdateUi();
}

void ECBrowser::OpenComponentXmlEditor()
{
    QTreeWidgetItem *item = treeWidget_->currentItem();
    if (!item)
        return;

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(item);//begin();
    if (iter != itemToComponentGroups_.end())
    {
        if (!(*iter)->components_.size())
            return;
        ComponentPtr pointer = (*iter)->components_[0].lock();
        if (pointer)
            emit ShowXmlEditorForComponent(pointer->TypeName().toStdString());
    }
}

void ECBrowser::CopyComponent()
{
    QDomDocument temp_doc;
    QDomElement entity_elem;
    QClipboard *clipboard = QApplication::clipboard();

    QTreeWidgetItem *item = treeWidget_->currentItem();
    if (!item)
        return;

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(item);
    if (iter != itemToComponentGroups_.end())
    {
        if (!(*iter)->components_.size())
            return;
        // Just take a first component from the componentgroup and copy it's attribute values to clipboard. 
        // Note! wont take account that other components might have different values in their attributes
        ComponentWeakPtr pointer = (*iter)->components_[0];
        if (!pointer.expired())
        {
            pointer.lock()->SerializeTo(temp_doc, entity_elem);
            QString xmlText = temp_doc.toString();
            clipboard->setText(xmlText);
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
        /// @todo add suport to multi component copy/paste feature.
        QDomElement comp_elem = temp_doc.firstChildElement("component");
        if (comp_elem.isNull())
            return;

        for(EntityWeakPtrList::iterator iter = entities_.begin();
            iter != entities_.end();
            iter++)
        {
            if((*iter).expired())
                continue;
            Scene::EntityPtr entity_ptr = (*iter).lock();

            ComponentPtr component;
            QString type = comp_elem.attribute("type");
            QString name = comp_elem.attribute("name");
            if (!entity_ptr->HasComponent(type, name))
            {
                component = framework_->GetComponentManager()->CreateComponent(type, name);
                entity_ptr->AddComponent(component, AttributeChange::Default);
            }
            else
                component = entity_ptr->GetComponent(type, name);
            if (component)
                component->DeserializeFrom(comp_elem, AttributeChange::Default);
        }
    }
}

void ECBrowser::DynamicComponentChanged()
{
    EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(sender()); 
    if (!component) 
    {
        LogError("Fail to dynamic cast sender object to EC_DynamicComponent in DynamicComponentChanged mehtod.");
        return;
    }
    ComponentPtr comp_ptr;
    try
    {
        comp_ptr = component->shared_from_this();
    } catch(...)
    {
        LogError("IComponent::shared_from_this failed! Component must have been deleted!");
        return;
    }

    Scene::Entity *entity = component->GetParentEntity();
    ComponentPtr compPtr = entity->GetComponent(component);
    RemoveComponentFromGroup(comp_ptr);
    AddNewComponentToGroup(comp_ptr);

    ComponentGroup *group = FindSuitableGroup(comp_ptr);
    if (group)
        group->editor_->UpdateUi();
}

void ECBrowser::OnComponentNameChanged(const QString &newName)
{
    IComponent *component = dynamic_cast<IComponent*>(sender());
    if (!component)
        return;

    ComponentPtr comp_ptr;
    try
    {
        comp_ptr = component->shared_from_this();
    } catch(...)
    {
        LogError("Couldn't update component name, cause parent entity was null.");
        return;
    }

    RemoveComponentFromGroup(comp_ptr);
    AddNewComponentToGroup(comp_ptr);
}

void ECBrowser::CreateAttribute()
{
    QTreeWidgetItem *item = treeWidget_->currentItem();
    if (!item)
        return;

    QTreeWidgetItem *rootItem = item;
    while(rootItem->parent())
        rootItem = rootItem->parent();

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(rootItem);
    if (iter == itemToComponentGroups_.end())
        return;
    if (!(*iter)->IsDynamic())
        return;

    /// @todo Should this dialog be converted to modless?
    bool ok = false;
    QString typeName = QInputDialog::getItem(this, tr("Give attribute type"), tr("Typename:"),
        framework_->GetComponentManager()->GetAttributeTypes(), 0, false, &ok);
    if (!ok)
        return;
    QString name = QInputDialog::getText(this, tr("Give attribute name"), tr("Name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return;

    std::vector<ComponentWeakPtr> components = (*iter)->components_;
    for(uint i = 0; i < components.size(); i++)
    {
        ComponentPtr component = components[i].lock();
        if (!component)
            continue;
        EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent*>(component.get());
        if (dc)
        {
            if(dc->CreateAttribute(typeName, name))
                dc->ComponentChanged(AttributeChange::Default);
        }
    }
}

void ECBrowser::OnDeleteAction()
{
    QTreeWidgetItem *item = treeWidget_->currentItem();
    if (!item)
        return;

    if (item->parent())
        DeleteAttribute(item);
    else
        DeleteComponent(item);
}

ComponentGroup *ECBrowser::FindSuitableGroup(ComponentPtr comp)
{
    PROFILE(ECBrowser_FindSuitableGroup);

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
    for(; iter != itemToComponentGroups_.end(); iter++)
        if((*iter)->IsSameComponent(comp))
            return (*iter);
    return 0;
}

void ECBrowser::AddNewComponentToGroup(ComponentPtr comp)
{
    PROFILE(ECBroweser_AddNewComponentToGroup);
    assert(comp);
    if (!comp.get() && !treeWidget_)
        return;

    ComponentGroup *comp_group = FindSuitableGroup(comp);
    if (comp_group)
    {
        // No point to add same component multiple times.
        if (comp_group->ContainsComponent(comp))
            return;

        comp_group->editor_->AddNewComponent(comp);
        comp_group->components_.push_back(ComponentWeakPtr(comp));
        if (comp_group->IsDynamic())
        {
            EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent*>(comp.get());
            connect(dc, SIGNAL(AttributeAdded(IAttribute *)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
            connect(dc, SIGNAL(AttributeRemoved(const QString &)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
            connect(dc, SIGNAL(ComponentNameChanged(const QString&, const QString&)),
                    SLOT(ComponentNameChanged(const QString&)), Qt::UniqueConnection);
        }
        return;
    }

    QSet<QTreeWidgetItem*> oldList;
    QSet<QTreeWidgetItem*> newList;

    // Find a new QTreeWidgetItem from the browser and save the information to ComponentGroup object.
    for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
        oldList.insert(treeWidget_->topLevelItem(i));

    // Disconnect itemExpanded() and itemCollapsed() signal before we create new items to the tree widget
    // so that we don't spam TreeWidgetItemExpandMemory's data as QtPropertyBrowser expands all items automatically.
    if (expandMemory_.lock())
    {
        disconnect(treeWidget_, SIGNAL(itemExpanded(QTreeWidgetItem *)), expandMemory_.lock().get(), SLOT(HandleItemExpanded(QTreeWidgetItem *)));
        disconnect(treeWidget_, SIGNAL(itemCollapsed(QTreeWidgetItem *)), expandMemory_.lock().get(), SLOT(HandleItemCollapsed(QTreeWidgetItem *)));
    }

    ECComponentEditor *componentEditor = new ECComponentEditor(comp, this);
    for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
        newList.insert(treeWidget_->topLevelItem(i));

    QSet<QTreeWidgetItem*> changeList = newList - oldList;
    QTreeWidgetItem *newItem = 0;
    if(changeList.size() == 1)
    {
        newItem = (*changeList.begin());
        // Apply possible item expansions for the new item.
        if (expandMemory_.lock())
            expandMemory_.lock()->ExpandItem(treeWidget_, newItem);
    }
    else
    {
        LogError("Failed to add a new component to ECEditor, for some reason the QTreeWidgetItem was not created."
            " Make sure that ECComponentEditor was intialized properly.");
        return;
    }

    bool dynamic = comp->TypeName() == EC_DynamicComponent::TypeNameStatic();
    if(dynamic)
    {
        EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent*>(comp.get());
        connect(dc, SIGNAL(AttributeAdded(IAttribute *)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
        connect(dc, SIGNAL(AttributeRemoved(const QString &)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
        connect(dc, SIGNAL(ComponentNameChanged(const QString &, const QString &)), SLOT(OnComponentNameChanged(const QString&)), Qt::UniqueConnection);
    }

    ComponentGroup *compGroup = new ComponentGroup(comp, componentEditor, dynamic);
    itemToComponentGroups_[newItem] = compGroup;

    // Connect itemExpanded() and itemCollapsed() signals back so that TreeWidgetItemExpandMemory
    // is kept up to date when user expands and collapses items.
    if (expandMemory_.lock())
    {
        connect(treeWidget_, SIGNAL(itemExpanded(QTreeWidgetItem *)), expandMemory_.lock().get(),
            SLOT(HandleItemExpanded(QTreeWidgetItem *)), Qt::UniqueConnection);
        connect(treeWidget_, SIGNAL(itemCollapsed(QTreeWidgetItem *)), expandMemory_.lock().get(),
            SLOT(HandleItemCollapsed(QTreeWidgetItem *)), Qt::UniqueConnection);
    }
}

void ECBrowser::RemoveComponentFromGroup(ComponentPtr comp)
{
    PROFILE(ECBrowser_RemoveComponentFromGroup);
    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
    for(; iter != itemToComponentGroups_.end(); iter++)
    {
        ComponentGroup *comp_group = *iter;
        if (comp_group->ContainsComponent(comp))
        {
            if (comp_group->IsDynamic())
            {
                EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent *>(comp.get());
                assert(dc);
                disconnect(dc, SIGNAL(AttributeAdded(IAttribute *)), this, SLOT(DynamicComponentChanged()));
                disconnect(dc, SIGNAL(AttributeRemoved(const QString &)), this, SLOT(DynamicComponentChanged()));
                disconnect(dc, SIGNAL(ComponentNameChanged(const QString&, const QString &)), this, SLOT(OnComponentNameChanged(const QString&)));
            }
            comp_group->RemoveComponent(comp);
            // Check if the component group still contains any components in it and if not, remove it from the browser list.
            if (!comp_group->IsValid())
            {
                SAFE_DELETE(comp_group);
                itemToComponentGroups_.erase(iter);
            }

            // The component can only be a member of one group, so can break here (we have invalidated 'iter' now).
            break;
        }
    }
}

void ECBrowser::RemoveComponentGroup(ComponentGroup *componentGroup)
{
    PROFILE(ECBrowser_RemoveComponentGroup);
    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
    for(; iter != itemToComponentGroups_.end(); iter++)
        if (componentGroup == *iter)
        {
            SAFE_DELETE(*iter);
            itemToComponentGroups_.erase(iter);
            break;
        }
}

bool ECBrowser::HasEntity(Scene::EntityPtr entity) const
{
    PROFILE(ECBrowser_HasEntity);
    for(uint i = 0; i < entities_.size(); i++)
    {
        if(!entities_[i].expired() && entities_[i].lock().get() == entity.get())
            return true;
    }
    return false;
}

void ECBrowser::DeleteAttribute(QTreeWidgetItem *item)
{
    assert(item);
    QTreeWidgetItem *rootItem = item;
    while(rootItem->parent())
        rootItem = rootItem->parent();

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(rootItem);
    if(iter == itemToComponentGroups_.end() || !(*iter)->IsDynamic())
        return;

    std::vector<ComponentWeakPtr> components = (*iter)->components_;
    for(uint i = 0; i < components.size(); i++)
    {
        ComponentPtr comp_ptr = components[i].lock();
        if(!comp_ptr)
            continue;
        EC_DynamicComponent *comp = dynamic_cast<EC_DynamicComponent*>(comp_ptr.get());
        if(comp)
        {
            // We assume that item's text contains attribute name.
            comp->RemoveAttribute(item->text(0));
            comp->ComponentChanged(AttributeChange::Default);
        }
    }
}

void ECBrowser::DeleteComponent(QTreeWidgetItem *item)
{
    assert(item);

    std::vector<ComponentWeakPtr> componentsToDelete;

    // The deletion logic below is done in two steps to avoid depending on the internal componentGroups_ member
    // while we are actually doing the deletion, since the Entity::RemoveComponent will call back to the 
    // ECBrowser::ComponentRemoved to update the UI.

    // Find the list of components we want to delete.
    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(item);
    if (iter != itemToComponentGroups_.end())
        componentsToDelete = (*iter)->components_;

    // Perform the actual deletion.
    for(std::vector<ComponentWeakPtr>::iterator iter = componentsToDelete.begin(); iter != componentsToDelete.end(); ++iter)
    {
        ComponentPtr comp = iter->lock();
        if (comp)
        {
            Scene::Entity *entity = comp->GetParentEntity();
            if (entity)
                entity->RemoveComponent(comp, AttributeChange::Default);
        }
    }
}
