// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECBrowser.h"
#include "ECEditorWindow.h"
#include "ComponentGroup.h"
#include "ECComponentEditor.h"
#include "TreeWidgetItemExpandMemory.h"
#include "TreeWidgetUtils.h"

#include "Profiler.h"
#include "SceneAPI.h"
#include "UiAPI.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetReference.h"
#include "UiMainWindow.h"
#include "Entity.h"
#include "IComponent.h"
#include "Scene/Scene.h"
#include "Framework.h"
#include "EC_DynamicComponent.h"
#include "LoggingFunctions.h"
#include "UndoCommands.h"
#include "UndoManager.h"

#include <QtBrowserItem>
#include <QLayout>
#include <QShortcut>
#include <QMenu>
#include <QDomDocument>
#include <QMimeData>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "MemoryLeakCheck.h"

#ifdef Q_WS_MAC
#define KEY_DELETE_SHORTCUT QKeySequence(Qt::CTRL + Qt::Key_Backspace)
#else
#define KEY_DELETE_SHORTCUT QKeySequence::Delete
#endif

ECBrowser::ECBrowser(Framework *framework, ECEditorWindow *editorWindow, QWidget *parent):
    QtTreePropertyBrowser(parent),
    menu_(0),
    treeWidget_(0),
    editorWindow_(editorWindow),
    framework_(framework)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setResizeMode(QtTreePropertyBrowser::Interactive);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(ShowComponentContextMenu(const QPoint &)));
    treeWidget_ = findChild<QTreeWidget *>();
    assert(treeWidget_);
    treeWidget_->setSortingEnabled(true);
    treeWidget_->setFocusPolicy(Qt::StrongFocus);
    treeWidget_->setAcceptDrops(true);
    treeWidget_->setDragDropMode(QAbstractItemView::DropOnly);
    treeWidget_->header()->setSortIndicator(0, Qt::AscendingOrder);

    connect(treeWidget_, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
        SLOT(SelectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)), Qt::UniqueConnection);

    connect(treeWidget_, SIGNAL(itemExpanded(QTreeWidgetItem *)), SLOT(ResizeHeaderToContents()));
    connect(treeWidget_, SIGNAL(itemCollapsed(QTreeWidgetItem *)), SLOT(ResizeHeaderToContents()));

    QShortcut *delete_shortcut = new QShortcut(KEY_DELETE_SHORTCUT, this);
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

void ECBrowser::AddEntity(const EntityPtr &entity)
{
    PROFILE(ECBrowser_AddNewEntity);

    assert(entity);
    if(!entity)
        return;

    //If entity is already added to browser no point to continue. 
    if(HasEntity(entity))
        return;
    entities_[entity->Id()] = entity;

    connect(entity.get(), SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)),
        SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
    connect(entity.get(), SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)),
        SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
}

void ECBrowser::RemoveEntity(const EntityPtr &entity)
{
    if (!entity)
        return;

    EntityMap::iterator iter = entities_.find(entity->Id());
    if (iter != entities_.end())
    {
        Entity *entity = iter->second.lock().get();
        if (entity)
        {
            disconnect(entity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this,
                SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
            disconnect(entity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this,
                SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));

            const Entity::ComponentMap components = entity->Components();
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                RemoveComponentFromGroup(i->second);
        }

        entities_.erase(iter);
    }
}

QList<EntityPtr> ECBrowser::Entities() const
{
    QList<EntityPtr> ret;
    for(EntityMap::const_iterator it = entities_.begin(); it != entities_.end(); ++it)
        if(!it->second.expired())
            ret.push_back(it->second.lock());
    return ret;
}

QObjectList ECBrowser::SelectedComponents() const
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
    assert(treeWidget_ != 0);

    // Sorting tends to be a heavy operation so we disable it until we have made all changes to a tree structure.
    treeWidget_->setSortingEnabled(false);

    for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
    {
        if(iter->second.expired())
            continue;

        const Entity::ComponentMap &components = iter->second.lock()->Components();
        for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            AddNewComponentToGroup(i->second);
    }

    for(TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
        iter != itemToComponentGroups_.end();
        ++iter)
    {
        (*iter)->editor_->UpdateUi();
    }

    treeWidget_->setSortingEnabled(true);
}

void ECBrowser::ExpandOrCollapseAll()
{
    if (treeWidget_)
    {
        treeWidget_->blockSignals(true);
        TreeWidgetExpandOrCollapseAll(treeWidget_);
        treeWidget_->blockSignals(false);
    }
}

void ECBrowser::dragEnterEvent(QDragEnterEvent *event)
{
    ///\todo Regression, "application/vnd.inventory.item" deprecated, reimplement
    QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
    if (event->mimeData()->hasFormat("application/vnd.inventory.item") && item && !item->childCount())
        event->acceptProposedAction();
}

void ECBrowser::dropEvent(QDropEvent *event)
{
    ///\todo Regression, "application/vnd.inventory.item" deprecated, reimplement
    QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
    if(item)
        dropMimeData(item, 0, event->mimeData(), event->dropAction());
}

void ECBrowser::dragMoveEvent(QDragMoveEvent *event)
{
    ///\todo Regression, "application/vnd.inventory.item" deprecated, reimplement
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

bool ECBrowser::dropMimeData(QTreeWidgetItem * /*item*/, int /*index*/, const QMimeData * /*data*/, Qt::DropAction /*action*/)
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
            if(attr->TypeName().compare("string", Qt::CaseInsensitive) == 0)
            {
                Attribute<QString> *attribute = dynamic_cast<Attribute<QString> *>(attr);
                if(attribute)
                    attribute->Set(QString::fromStdString(asset_id.toStdString()), AttributeChange::Default);
            }
            else if(attr->TypeName().compare("qvariant", Qt::CaseInsensitive) == 0)
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
            else if(attr->TypeName().compare("qvariantlist", Qt::CaseInsensitive) == 0)
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
    if (treeWidgetItem)
        treeWidget_->setCurrentItem(treeWidgetItem);
    else
        treeWidget_->setCurrentItem(0);

    SAFE_DELETE(menu_);
    menu_ = new QMenu(this);
    if(treeWidgetItem)
    {
        QAction *copyComponent = new QAction(tr("Copy"), menu_);
        QAction *pasteComponent = new QAction(tr("Paste"), menu_);
        QAction *editXml = new QAction(tr("Edit XML..."), menu_);
        // Delete action functionality can vary based on what QTreeWidgetItem is selected on the browser.
        // If root item is selected we assume that we want to remove component and non root items are attributes
        // that need to removed (attribute delete is only enabled with EC_DynamicComponent).
        QAction *deleteAction = new QAction(tr("Delete"), menu_);

        //Add shortcuts for actions
        copyComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
        pasteComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
        deleteAction->setShortcut(KEY_DELETE_SHORTCUT);

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

        if (iter != itemToComponentGroups_.end())
        {
            if ((*iter)->isDynamic_)
            {
                QAction *addAttribute = new QAction(tr("Add new attribute..."), menu_);
                connect(addAttribute, SIGNAL(triggered()), this, SLOT(CreateAttribute()), Qt::UniqueConnection);
                menu_->addAction(addAttribute);
            }
            else
            {
                // Disable delete action for static attributes
                if (treeWidget_->currentItem() && treeWidget_->currentItem()->parent())
                    deleteAction->setDisabled(true);
            }

            QList<QObject*> targets;
            for (unsigned i = 0; i < (*iter)->components_.size(); ++i)
            {
                IComponent* comp = (*iter)->components_[i].lock().get();
                if (comp)
                {
                    IAttribute *attr = comp->AttributeByName(treeWidget_->currentItem()->text(0));
                    if (attr && attr->TypeId() == cAttributeAssetReference)
                    {
                        Attribute<AssetReference> *attribute = static_cast<Attribute<AssetReference> *>(attr);
                        AssetPtr asset = framework_->Asset()->GetAsset(attribute->Get().ref);
                        if (asset)
                            targets.append(asset.get());
                    }

                    targets.push_back(comp);
                }
            }

            framework_->Ui()->EmitContextMenuAboutToOpen(menu_, targets);
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

void ECBrowser::SelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
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

void ECBrowser::OnComponentAdded(IComponent* comp, AttributeChange::Type /*type*/)
{
    EntityPtr entity_ptr = framework_->Scene()->MainCameraScene()->GetEntity(comp->ParentEntity()->Id());
    if(!HasEntity(entity_ptr))
        return;
    ComponentPtr comp_ptr;
    try
    {
        comp_ptr = comp->shared_from_this();
    } catch(...)
    {
        LogError("Fail to add new component to ECBrowser. Make sure that component's parent entity is set.");
        return;
    }

    TreeItemToComponentGroup::iterator iterComp = itemToComponentGroups_.begin();
    for(; iterComp != itemToComponentGroups_.end(); ++iterComp)
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

void ECBrowser::OnComponentRemoved(IComponent* comp, AttributeChange::Type /*type*/)
{
    EntityPtr entity_ptr = framework_->Scene()->MainCameraScene()->GetEntity(comp->ParentEntity()->Id());
    if(!HasEntity(entity_ptr))
        return;

    ComponentPtr comp_ptr;

    try
    {
        comp_ptr = comp->shared_from_this();
    } catch(...)
    {
        LogError("Fail to remove component from ECBrowser. Make sure that component's parent entity is set.");
        return;
    } 

    TreeItemToComponentGroup::iterator iterComp = itemToComponentGroups_.begin();
    for(; iterComp != itemToComponentGroups_.end(); ++iterComp)
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
            emit ShowXmlEditorForComponent(pointer->TypeName());
    }
}

void ECBrowser::CopyComponent()
{
    QTreeWidgetItem *item = treeWidget_->currentItem();
    if (!item)
        return;

    QDomDocument temp_doc("Scene");
    QDomElement sceneElem = temp_doc.createElement("scene");
    QClipboard *clipboard = QApplication::clipboard();

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.find(item);
    if (iter != itemToComponentGroups_.end())
    {
        if (!(*iter)->components_.size())
            return;
        ///\todo multiple component selection support
        // Just take a first component from the componentgroup and copy its attribute values to clipboard.
        // Note! wont take account that other components might have different values in their attributes
        ComponentWeakPtr pointer = (*iter)->components_[0];
        if (!pointer.expired())
        {
            pointer.lock()->SerializeTo(temp_doc, sceneElem, true);
            temp_doc.appendChild(sceneElem);
            clipboard->setText(temp_doc.toString());
        }
    }
}

void ECBrowser::PasteComponent()
{
    QDomDocument temp_doc("Scene");
    QClipboard *clipboard = QApplication::clipboard();
    if (temp_doc.setContent(clipboard->text()))
    {
        QDomElement sceneElem = temp_doc.firstChildElement("scene");
        if (sceneElem.isNull())
            return;

        // Only single component can be pasted.
        /// @todo add suport to multi component copy/paste feature.
        QDomElement comp_elem = sceneElem.firstChildElement("component");
        if (comp_elem.isNull())
            return;

        for(EntityMap::iterator iter = entities_.begin(); iter != entities_.end(); ++iter)
        {
            if (iter->second.expired())
                continue;
            EntityPtr entity_ptr = iter->second.lock();

            QString type = comp_elem.attribute("type");
            QString name = comp_elem.attribute("name");
            QString sync = comp_elem.attribute("sync");
            QString temp = comp_elem.attribute("temporary");

            int copy = 2;
            QString newName = name;
            while(entity_ptr->Component(type, newName))
                newName = QString(name + " (%1)").arg(copy++);

            comp_elem.setAttribute("name", newName);
            ComponentPtr component = framework_->Scene()->CreateComponentByName(entity_ptr->ParentScene(), type, newName);

            if (!sync.isEmpty())
                component->SetReplicated(ParseBool(sync));
            if (!temp.isEmpty())
                component->SetTemporary(ParseBool(temp));

            entity_ptr->AddComponent(component, AttributeChange::Default);
            component->DeserializeFrom(comp_elem, AttributeChange::Default);
        }
    }
}

void ECBrowser::DynamicComponentChanged()
{
    EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(sender());
    if (!component)
    {
        LogError("EC_Browser::DynamicComponentChanged: Failed to dynamic cast sender object to EC_DynamicComponent.");
        return;
    }
    ComponentPtr comp_ptr;
    try
    {
        comp_ptr = component->shared_from_this();
    } catch(...)
    {
        LogError("EC_Browser::DynamicComponentChanged: IComponent::shared_from_this failed! Component must have been deleted!");
        return;
    }

    RemoveComponentFromGroup(comp_ptr);
    AddNewComponentToGroup(comp_ptr);

    ComponentGroup *group = FindSuitableGroup(comp_ptr);
    if (group)
        group->editor_->UpdateUi();
}

void ECBrowser::RemoveAttributeFromDynamicComponent(IAttribute *attr)
{
    if (!attr)
        return;
    ComponentPtr component = attr->Owner()->shared_from_this();
    ComponentGroup *group = FindSuitableGroup(component);
    if (group)
        group->editor_->RemoveAttribute(component, attr);
}

void ECBrowser::OnComponentNameChanged(const QString & /*newName*/)
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
        LogError("EC_Browser::OnComponentNameChanged: IComponent::shared_from_this failed! Component must have been deleted!");
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

    // Find the dynamic component
    EC_DynamicComponent *dynComp = 0;
    std::vector<ComponentWeakPtr> components = (*iter)->components_;
    for(uint i = 0; i < components.size(); i++)
    {
        dynComp = dynamic_cast<EC_DynamicComponent*>(components[i].lock().get());
        if (dynComp)
            break;
    }

    if (!dynComp)
    {
        LogError("ECBrowser:CreateAttribute() Could not find EC_DynamicCompoent in selection.");
        return;
    }

    /// @todo make this code to its own NewAttributeDialog class.
    // Create the dialog
    QDialog newAttrDialog(framework_->Ui()->MainWindow());
    newAttrDialog.setModal(true);
    newAttrDialog.setWindowFlags(Qt::Tool);
    newAttrDialog.setWindowTitle(tr("Create New Attribute"));

    QPushButton *buttonCreate = new QPushButton(tr("Create"));
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"));
    buttonCreate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCreate->setDefault(true);
    buttonCancel->setAutoDefault(false);

    QComboBox *comboTypes = new QComboBox();
    comboTypes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    comboTypes->addItems(SceneAPI::AttributeTypes());

    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameEdit->setFocus(Qt::ActiveWindowFocusReason);

    QLabel *lName = new QLabel(tr("Name"));
    QLabel *lType = new QLabel(tr("Type"));
    lName->setMinimumWidth(50);

    QLabel *errorLabel = new QLabel();
    errorLabel->setStyleSheet("QLabel { background-color: rgba(255,0,0,150); padding: 4px; border: 1px solid grey; }");
    errorLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->hide();

    QGridLayout *grid = new QGridLayout();
    grid->setVerticalSpacing(8);
    grid->addWidget(lName, 0, 0);
    grid->addWidget(nameEdit, 0, 1, Qt::AlignLeft, 1);
    grid->addWidget(lType, 1, 0);
    grid->addWidget(comboTypes, 1, 1, Qt::AlignLeft, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonCreate);
    buttonLayout->addWidget(buttonCancel);

    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addLayout(grid);
    vertLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vertLayout->addWidget(errorLabel);
    vertLayout->addLayout(buttonLayout);

    newAttrDialog.setLayout(vertLayout);

    connect(nameEdit, SIGNAL(returnPressed()), &newAttrDialog, SLOT(accept()));
    connect(buttonCreate, SIGNAL(clicked()), &newAttrDialog, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), &newAttrDialog, SLOT(reject()));

    // Execute dialog
    newAttrDialog.resize(300, 120);
    newAttrDialog.activateWindow();
    bool dialogDone = false;
    while (!dialogDone)
    {
        int ret = newAttrDialog.exec();
        if (ret == QDialog::Rejected)
            break;

        QString typeName = comboTypes->currentText();
        QString name = nameEdit->text().trimmed();

        if (name.isEmpty())
        {
            errorLabel->setText(tr("Attribute name cannot be empty."));
            errorLabel->show();
            continue;
        }

        if (!dynComp->ContainsAttribute(name))
        {
            dialogDone = true;
            editorWindow_->GetUndoManager()->Push(new AddAttributeCommand(dynComp, typeName, name));
        }
        else
        {
            errorLabel->setText(tr("Attribute \"%1\" already exists in this component. Pick a unique name.").arg(name));
            errorLabel->show();
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

void ECBrowser::ResizeHeaderToContents()
{
    treeWidget_->resizeColumnToContents(0);
    treeWidget_->resizeColumnToContents(1);
}

ComponentGroup *ECBrowser::FindSuitableGroup(const ComponentPtr &comp)
{
    PROFILE(ECBrowser_FindSuitableGroup);

    TreeItemToComponentGroup::iterator iter = itemToComponentGroups_.begin();
    for(; iter != itemToComponentGroups_.end(); ++iter)
        if((*iter)->IsSameComponent(comp))
            return (*iter);
    return 0;
}

void ECBrowser::AddNewComponentToGroup(const ComponentPtr &comp)
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
            //connect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
            connect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(RemoveAttributeFromDynamicComponent(IAttribute *)), Qt::UniqueConnection);
            connect(dc, SIGNAL(ComponentNameChanged(const QString&, const QString&)), SLOT(OnComponentNameChanged(const QString&)), Qt::UniqueConnection);
        }
        return;
    }

    QSet<QTreeWidgetItem*> oldList, newList;

    // Find a new QTreeWidgetItem from the browser and save the information to ComponentGroup object.
    for(uint i = 0; i < (uint)treeWidget_->topLevelItemCount(); i++)
        oldList.insert(treeWidget_->topLevelItem(i));

    // Disconnect itemExpanded() and itemCollapsed() signal before we create new items to the tree widget
    // so that we don't spam TreeWidgetItemExpandMemory's data as QtPropertyBrowser expands all items automatically.
    if (expandMemory_.lock())
    {
        disconnect(treeWidget_, SIGNAL(itemExpanded(QTreeWidgetItem *)), expandMemory_.lock().get(), SLOT(HandleItemExpanded(QTreeWidgetItem *)));
        disconnect(treeWidget_, SIGNAL(itemCollapsed(QTreeWidgetItem *)), expandMemory_.lock().get(), SLOT(HandleItemCollapsed(QTreeWidgetItem *)));
    }

    ECComponentEditor *componentEditor = new ECComponentEditor(comp, this);
    connect(componentEditor, SIGNAL(AttributeAboutToBeEdited(IAttribute *)), editorWindow_, SLOT(OnAboutToEditAttribute(IAttribute *)));

    for(uint i = 0; i < (uint)treeWidget_->topLevelItemCount(); i++)
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
    if (dynamic)
    {
        EC_DynamicComponent *dc = static_cast<EC_DynamicComponent *>(comp.get());
        connect(dc, SIGNAL(AttributeAdded(IAttribute *)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
        //connect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(DynamicComponentChanged()), Qt::UniqueConnection);
        connect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(RemoveAttributeFromDynamicComponent(IAttribute *)), Qt::UniqueConnection);
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
    for(; iter != itemToComponentGroups_.end(); ++iter)
    {
        ComponentGroup *comp_group = *iter;
        if (comp_group->ContainsComponent(comp))
        {
            if (comp_group->IsDynamic())
            {
                EC_DynamicComponent *dc = checked_static_cast<EC_DynamicComponent *>(comp.get());
                disconnect(dc, SIGNAL(AttributeAdded(IAttribute *)), this, SLOT(DynamicComponentChanged()));
                //disconnect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), this, SLOT(DynamicComponentChanged()));
                disconnect(dc, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), this, SLOT(RemoveAttributeFromDynamicComponent(IAttribute *)));
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
    for(; iter != itemToComponentGroups_.end(); ++iter)
        if (componentGroup == *iter)
        {
            SAFE_DELETE(*iter);
            itemToComponentGroups_.erase(iter);
            break;
        }
}

bool ECBrowser::HasEntity(const EntityPtr &entity) const
{
    return entities_.find(entity->Id()) != entities_.end();
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
            editorWindow_->GetUndoManager()->Push(new RemoveAttributeCommand(comp_ptr.get()->AttributeByName(item->text(0))));
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
    ComponentPtr comp = componentsToDelete.at(0).lock();
    editorWindow_->GetUndoManager()->Push(new RemoveCommand(comp->ParentScene()->shared_from_this(), editorWindow_->GetUndoManager()->Tracker(),
            QList<EntityWeakPtr>(), QList<ComponentWeakPtr>::fromVector(QVector<ComponentWeakPtr>::fromStdVector(componentsToDelete))));
}
