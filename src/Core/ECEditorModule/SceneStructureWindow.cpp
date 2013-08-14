/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneStructureWindow.cpp
    @brief  Window with tree view of contents of scene. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"
#include "SceneTreeWidgetItems.h"
#include "TreeWidgetUtils.h"
#include "UndoManager.h"

#include "Framework.h"
#include "Application.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "EC_Name.h"
#include "AssetReference.h"
#include "LoggingFunctions.h"

#include <QTreeWidgetItemIterator>
#include <QToolButton>

#include "MemoryLeakCheck.h"

SceneStructureWindow::SceneStructureWindow(Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    showComponents(true),
    showAttributesMode(ShowAssetReferences),
    treeWidget(0),
    expandAndCollapseButton(0),
    searchField(0)
{
    // Init main widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(325, 400);

    // Create child widgets
    treeWidget = new SceneTreeWidget(fw, this);
    expandAndCollapseButton = new QPushButton(tr("Expand All"), this);
    expandAndCollapseButton->setFixedHeight(22);

    searchField = new QLineEdit(this);
    /// @todo use searchField->setPlaceholderText(); instead of manual custom implementation for "Search..." text.
    searchField->setText(tr("Search..."));
    searchField->setStyleSheet("color:grey;");
    searchField->installEventFilter(this);
    searchField->setFixedHeight(20);

    QLabel *sortLabel = new QLabel(tr("Sort by"), this);
    QComboBox *sortComboBox = new QComboBox(this);
    sortComboBox->addItem(tr("ID"));
    sortComboBox->addItem(tr("Name"));
    sortComboBox->setFixedHeight(20);

    QCheckBox *compCheckBox = new QCheckBox(tr("Components"), this);
    compCheckBox->setChecked(showComponents);

    QLabel *attributeLabel = new QLabel(tr("Attributes"), this);
    QComboBox *attributeComboBox = new QComboBox(this);
    attributeComboBox->addItem(tr("None"), DoNotShowAttributes);
    attributeComboBox->addItem(tr("Assets"), ShowAssetReferences);
    attributeComboBox->addItem(tr("Dynamic"), ShowDynamicAttributes);
    attributeComboBox->addItem(tr("All"), ShowAllAttributes);
    attributeComboBox->setCurrentIndex(showAttributesMode);

    undoButton_ = new QToolButton();
    undoButton_->setPopupMode(QToolButton::MenuButtonPopup);
    undoButton_->setDisabled(true);

    redoButton_ = new QToolButton();
    redoButton_->setPopupMode(QToolButton::MenuButtonPopup);
    redoButton_->setDisabled(true);

    undoButton_->setIcon(QIcon(Application::InstallationDirectory() + "data/ui/images/icon/undo-icon.png"));
    redoButton_->setIcon(QIcon(Application::InstallationDirectory() + "data/ui/images/icon/redo-icon.png"));

    // Fill layouts
    QHBoxLayout *layoutFilterAndSort = new QHBoxLayout();
    layoutFilterAndSort->setContentsMargins(0,0,0,0);
    layoutFilterAndSort->setSpacing(5);
    layoutFilterAndSort->addWidget(searchField);
    layoutFilterAndSort->addWidget(sortLabel);
    layoutFilterAndSort->addWidget(sortComboBox);
    layoutFilterAndSort->addWidget(expandAndCollapseButton);

    QHBoxLayout *layoutSettingsVisibility = new QHBoxLayout();
    layoutSettingsVisibility->setContentsMargins(0,0,0,0);
    layoutSettingsVisibility->setSpacing(5);
    layoutSettingsVisibility->addWidget(undoButton_);
    layoutSettingsVisibility->addWidget(redoButton_);
    layoutSettingsVisibility->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layoutSettingsVisibility->addWidget(compCheckBox);
    layoutSettingsVisibility->addWidget(attributeLabel);
    layoutSettingsVisibility->addWidget(attributeComboBox);

    layout->addLayout(layoutFilterAndSort);
    layout->addWidget(treeWidget);
    layout->addLayout(layoutSettingsVisibility);
    
    // Connect to widget signals
    connect(attributeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(ShowAttributesInternal(int)));
    connect(compCheckBox, SIGNAL(toggled(bool)), SLOT(ShowComponents(bool)));
    connect(sortComboBox, SIGNAL(currentIndexChanged(int)), SLOT(Sort(int)));
    connect(searchField, SIGNAL(textEdited(const QString &)), SLOT(Search(const QString &)));
    connect(expandAndCollapseButton, SIGNAL(clicked()), SLOT(ExpandOrCollapseAll()));
    connect(treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(CheckTreeExpandStatus(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(CheckTreeExpandStatus(QTreeWidgetItem*)));
}

SceneStructureWindow::~SceneStructureWindow()
{
    SetScene(ScenePtr());
}

void SceneStructureWindow::SetScene(const ScenePtr &newScene)
{
    if (!scene.expired() && (newScene == scene.lock()))
        return;

    ScenePtr previous = scene.lock();
    if (previous)
    {
        disconnect(previous.get());
        Clear();
    }

    scene = newScene;
    treeWidget->SetScene(newScene);

    if (newScene)
    {
        // Now that treeWidget has scene, it also has UndoManager.
        UndoManager *undoMgr = treeWidget->GetUndoManager();
        undoButton_->setMenu(undoMgr->UndoMenu());
        redoButton_->setMenu(undoMgr->RedoMenu());
        connect(undoMgr, SIGNAL(CanUndoChanged(bool)), this, SLOT(OnUndoChanged(bool)), Qt::UniqueConnection);
        connect(undoMgr, SIGNAL(CanRedoChanged(bool)), this, SLOT(OnRedoChanged(bool)), Qt::UniqueConnection);
        connect(undoButton_, SIGNAL(clicked()), undoMgr, SLOT(Undo()), Qt::UniqueConnection);
        connect(redoButton_, SIGNAL(clicked()), undoMgr, SLOT(Redo()), Qt::UniqueConnection);

        Scene *scenePtr = scene.lock().get();
        connect(scenePtr, SIGNAL(EntityAcked(Entity *, entity_id_t)), SLOT(AckEntity(Entity *, entity_id_t)));
        connect(scenePtr, SIGNAL(EntityCreated(Entity *, AttributeChange::Type)), SLOT(AddEntity(Entity *)));
        connect(scenePtr, SIGNAL(EntityTemporaryStateToggled(Entity *, AttributeChange::Type)), SLOT(UpdateEntityTemporaryState(Entity *)));
        connect(scenePtr, SIGNAL(EntityRemoved(Entity *, AttributeChange::Type)), SLOT(RemoveEntity(Entity *)));
        connect(scenePtr, SIGNAL(ComponentAdded(Entity *, IComponent *, AttributeChange::Type)),
            SLOT(AddComponent(Entity *, IComponent *)));
        connect(scenePtr, SIGNAL(ComponentRemoved(Entity *, IComponent *, AttributeChange::Type)),
            SLOT(RemoveComponent(Entity *, IComponent *)));

        Populate();
    }
}

void SceneStructureWindow::ShowComponents(bool show)
{
    showComponents = show;
    treeWidget->showComponents =show;

    treeWidget->setSortingEnabled(false);

    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityGroupItem *gItem = dynamic_cast<EntityGroupItem*>(treeWidget->topLevelItem(i));
        if (gItem)
        {
            for (int j = 0; j < gItem->childCount(); ++j)
            {
                QTreeWidgetItem *item = gItem->child(j);
                for(int k = 0; k < item->childCount(); ++k)
                    item->child(k)->setHidden(!showComponents);
            }
        }
        else
        {
            QTreeWidgetItem *item = treeWidget->topLevelItem(i);
            for (int j = 0; j < item->childCount(); ++j)
                item->child(j)->setHidden(!showComponents);
        }
    }

    if (showAttributesMode != DoNotShowAttributes)
    {
        ClearAttributes();
        CreateAttributes();
    }

    treeWidget->setSortingEnabled(true);

    expandAndCollapseButton->setEnabled(showComponents || showAttributesMode != DoNotShowAttributes);
}

void SceneStructureWindow::ShowAttributes(AttributeMode show)
{
    showAttributesMode = show;

    treeWidget->setSortingEnabled(false);

    if (scene.expired())
    {
        Clear();
        return;
    }

    ClearAttributes();

    if (showAttributesMode != DoNotShowAttributes)
        CreateAttributes();

    treeWidget->setSortingEnabled(true);

    expandAndCollapseButton->setEnabled(showComponents || showAttributesMode != DoNotShowAttributes);
}

void SceneStructureWindow::SetEntitySelected(const EntityPtr &entity, bool selected)
{
    if (entity)
    {
        EntityItem *eItem = EntityItemOfEntity(entity.get());
        if (eItem)
        {
            QFont font = eItem->font(0);
            font.setBold(selected);
            eItem->setFont(0, font);
        }
    }
}

void SceneStructureWindow::ClearSelectedEntites()
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (!eItem)
            continue;
        QFont font = eItem->font(0);
        if (!font.bold())
            continue;
        font.setBold(false);
        eItem->setFont(0, font);
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
        LogWarning("Scene pointer expired. Cannot populate tree widget.");
        return;
    }

    treeWidget->setSortingEnabled(false);

    for(Scene::iterator it = s->begin(); it != s->end(); ++it)
        AddEntity((*it).second.get());

    treeWidget->setSortingEnabled(true);
}

void SceneStructureWindow::Clear()
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        SAFE_DELETE(item);
    }
}

EntityItem* SceneStructureWindow::EntityItemOfEntity(Entity *ent) const
{
    return ent ? EntityItemById(ent->Id()) : 0;
}

EntityItem* SceneStructureWindow::EntityItemById(entity_id_t id) const
{
    EntityItem *eItem = 0;
    
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *temp = dynamic_cast<EntityItem*>(treeWidget->topLevelItem(i));
        if (temp)
        {
            if (temp->Id() == id)
            {
                eItem = temp;
                break;
            }
        }
        else
        {
            EntityGroupItem *gItem = dynamic_cast<EntityGroupItem*>(treeWidget->topLevelItem(i));
            if (gItem)
            {
                for (int j = 0; j < gItem->childCount(); ++j)
                {
                    EntityItem *childEnt = dynamic_cast<EntityItem*>(gItem->child(j));
                    if (childEnt && childEnt->Id() == id)
                    {
                        eItem = childEnt;
                        break;
                    }
                }
            }
        }
    }

    return eItem;
}

void SceneStructureWindow::ClearAttributes()
{
    QList<AttributeItem *> attributeItems;
    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AttributeItem *aItem = dynamic_cast<AttributeItem *>(*it);
        if (aItem)
            attributeItems << aItem;
        ++it;
    }

    foreach(AttributeItem *aItem, attributeItems)
    {
        QTreeWidgetItem *p = aItem->parent();
        p->removeChild(aItem);
        SAFE_DELETE(aItem);
    }
}

void SceneStructureWindow::AddEntity(Entity* entity)
{
    if (!entity->Group().isEmpty())
    {
        EntityGroupItem *groupItem = entityGroupItems_[entity->Group()];
        if (!groupItem)
        {
            groupItem = new EntityGroupItem(entity->Group());
            entityGroupItems_[entity->Group()] = groupItem;
        }

        const Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        groupItem->setFlags(flags);
        treeWidget->addTopLevelItem(groupItem);

        EntityItem *entityItem = new EntityItem(entity->shared_from_this(), groupItem);
        entityItem->setFlags(flags);

        groupItem->addChild(entityItem);
    }
    else
    {
        EntityItem *entityItem = new EntityItem(entity->shared_from_this());
        entityItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        treeWidget->addTopLevelItem(entityItem);
    }

    const Entity::ComponentMap &components = entity->Components();
    for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
        AddComponent(entity, i->second.get());

    // If we have an ongoing search, make sure that the new item is compared too.
    QString searchFilter = searchField->text().trimmed();
    if (!searchFilter.isEmpty() && searchFilter != tr("Search..."))
        TreeWidgetSearch(treeWidget, 0, searchFilter);
}

void SceneStructureWindow::AckEntity(Entity* entity, entity_id_t oldId)
{
    RemoveEntityById(oldId);
    AddEntity(entity);
}

void SceneStructureWindow::UpdateEntityTemporaryState(Entity *entity)
{
    EntityItem *entItem = EntityItemOfEntity(entity);
    if (!entItem)
        return;

    entItem->SetText(entity);

    for(int j = 0; j < entItem->childCount(); ++j)
    {
        ComponentItem *compItem = dynamic_cast<ComponentItem *>(entItem->child(j));
        if (compItem && compItem->Component().get() && compItem->Parent() == entItem)
            compItem->SetText(compItem->Component().get());
    }
}

void SceneStructureWindow::RemoveEntity(Entity* entity)
{
    EntityItem *item = EntityItemOfEntity(entity);
    if (item)
        RemoveEntityItem(item);
}

void SceneStructureWindow::RemoveEntityById(entity_id_t id)
{
    EntityItem *item = EntityItemById(id);
    if (item)
        RemoveEntityItem(item);
}

void SceneStructureWindow::RemoveEntityItem(EntityItem* item)
{
    EntityGroupItem *gItem = item->Parent();
    SAFE_DELETE(item);

    // Delete entity group item if this entity being deleted is the last child
    if (!gItem)
        return;

    QString groupName = gItem->GroupName();
    if (!gItem->childCount())
    {
        SAFE_DELETE(gItem);
        entityGroupItems_[groupName] = 0;
    }
}

void SceneStructureWindow::AddComponent(Entity* entity, IComponent* comp)
{
    EntityItem *eItem = EntityItemOfEntity(entity);
    if (!eItem)
        return;

    ComponentItem *cItem = new ComponentItem(comp->shared_from_this(), eItem);
    cItem->setHidden(!showComponents);

    eItem->addChild(cItem);

    connect(comp, SIGNAL(ComponentNameChanged(const QString &, const QString &)),
        SLOT(UpdateComponentName(const QString &, const QString &)));

    if (comp->TypeId() == EC_Name::ComponentTypeId)
    {
        // Retrieve entity's name from Name component. Also hook up change signal so that UI keeps synch with the name.
        eItem->SetText(entity);

        connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
            SLOT(UpdateEntityName(IAttribute *)), Qt::UniqueConnection);
    }

    if (comp->SupportsDynamicAttributes())
    {
        // Hook to changes of dynamic attributes in order to keep the UI in sync (currently only DynamicComponent has these).
        connect(comp, SIGNAL(AttributeAdded(IAttribute *)),
            SLOT(AddDynamicAttribute(IAttribute *)), Qt::UniqueConnection);
        connect(comp, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)),
            SLOT(RemoveDynamicAttribute(IAttribute *)), Qt::UniqueConnection);
        connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
            SLOT(UpdateDynamicAttribute(IAttribute *)), Qt::UniqueConnection);
    }

    // Add possible attributes.
    CreateAttributesForComponent(cItem);

    // If we have an ongoing search, make sure that the new item is compared too.
    QString searchFilter = searchField->text().trimmed();
    if (!searchFilter.isEmpty() && searchFilter != tr("Search..."))
        TreeWidgetSearch(treeWidget, 0, searchFilter);
}

void SceneStructureWindow::RemoveComponent(Entity* entity, IComponent* comp)
{
    EntityItem *eItem = EntityItemOfEntity(entity);
    if (!eItem)
        return;

    for(int j = 0; j < eItem->childCount(); ++j)
    {
        ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
        if (cItem && cItem->Component().get() == comp)
        {
            eItem->removeChild(cItem);
            SAFE_DELETE(cItem);
            break;
        }
    }

    if (comp->TypeId() == EC_Name::ComponentTypeId)
        eItem->setText(0, QString("%1").arg(entity->Id()));
}

void SceneStructureWindow::CreateAttributesForEntity(EntityItem *eItem)
{
    if (!eItem)
        return;

    EntityPtr entity = eItem->Entity();
    if (!entity)
        return;

    if (showComponents) // Parent to component items.
    {
        for(int i = 0; i < eItem->childCount(); ++i)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(i));
            CreateAttributesForComponent(cItem);
        }
    }
    else // Parent to entity items.
    {
        const Entity::ComponentMap &components = entity->Components();
        for(Entity::ComponentMap::const_iterator it = components.begin(); it != components.end(); ++it)
            foreach(IAttribute *attr, it->second->Attributes())
                if (attr && (showAttributesMode == ShowAllAttributes ||
                    (showAttributesMode == ShowDynamicAttributes && attr->IsDynamic()) ||
                    (showAttributesMode == ShowAssetReferences && (attr->TypeId() == cAttributeAssetReference ||
                    attr->TypeId() == cAttributeAssetReferenceList))))
                {
                    CreateAttributeItem(eItem, attr);
                }
    }
}

void SceneStructureWindow::CreateAttributesForComponent(ComponentItem *cItem)
{
    if (cItem && cItem->Component())
        foreach(IAttribute *attr, cItem->Component()->Attributes())
            if (attr && (showAttributesMode == ShowAllAttributes ||
                (showAttributesMode == ShowDynamicAttributes && attr->IsDynamic()) ||
                (showAttributesMode == ShowAssetReferences && (attr->TypeId() == cAttributeAssetReference ||
                attr->TypeId() == cAttributeAssetReferenceList))))
            {
                CreateAttributeItem(cItem, attr);
            }
}

void SceneStructureWindow::CreateAttributes()
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityGroupItem *gItem = dynamic_cast<EntityGroupItem*>(treeWidget->topLevelItem(i));
        if (gItem)
            for(int j = 0; j < gItem->childCount(); ++j)
                CreateAttributesForEntity(dynamic_cast<EntityItem *>(gItem->child(j)));
        else
            CreateAttributesForEntity(dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i)));
    }
}

void SceneStructureWindow::CreateAttributeItem(QTreeWidgetItem *parentItem, IAttribute *attr)
{
    // Do not create duplicate items. Recreate already existing items in order to guarantee up-to-date information.
    QList<AttributeItem *> alreadyExisting;
    for(int i = 0; i < parentItem->childCount(); ++i)
    {
        AttributeItem *item = dynamic_cast<AttributeItem *>(parentItem->child(i));
        if (item && item->ptr.Get() == attr)
            alreadyExisting << item;
    }

    foreach(AttributeItem *item, alreadyExisting)
    {
        parentItem->removeChild(item);
        SAFE_DELETE(item);
    }

    // Create special items for asset references.>
    if (attr->TypeId() == cAttributeAssetReference || attr->TypeId() == cAttributeAssetReferenceList)
    {
        const bool visible = (showAttributesMode == ShowAllAttributes || showAttributesMode == ShowAssetReferences ||
            (showAttributesMode == ShowDynamicAttributes && attr->IsDynamic()));

        // If this attribute is an empty AssetReferenceList it will have a dummy item so
        // that it appears in the tree.
        AttributeItem *aItem = new AssetRefItem(attr, parentItem);
        aItem->setHidden(!visible);
        parentItem->addChild(aItem);

        if (attr->TypeId() == cAttributeAssetReferenceList)
        {
            // Fill ref for possible already existing first AssetReferenceList item.
            const AssetReferenceList &refs = static_cast<Attribute<AssetReferenceList> *>(attr)->Get();
            if (refs.Size() >= 1)
                aItem->setText(0, QString("%1: %2").arg(attr->Id()).arg(refs[0].ref)); /**< @todo Make this some kind of Update() call instead? */

            // Create new items for rest of the refs.
            for(int i = 1; i < refs.Size(); ++i)
            {
                aItem = new AssetRefItem(attr, refs[i].ref, parentItem);
                aItem->setHidden(!visible);
                parentItem->addChild(aItem);
            }
        }
    }
    else
    {
        AttributeItem *aItem = new AttributeItem(attr, parentItem);
        const bool visible = (showAttributesMode == ShowAllAttributes || (showAttributesMode == ShowDynamicAttributes && attr->IsDynamic()));
        aItem->setHidden(!visible);
        parentItem->addChild(aItem);
    }
}

void SceneStructureWindow::AddDynamicAttribute(IAttribute *attr)
{
    if (showAttributesMode != ShowAllAttributes && showAttributesMode != ShowDynamicAttributes)
        return;

    QTreeWidgetItem *parentItem = 0;
    if (showComponents)
    {
        // Find parent component item
        QTreeWidgetItemIterator it(treeWidget);
        while(*it)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(*it);
            if (cItem && cItem->Component().get() == attr->Owner())
            {
                parentItem = cItem;
                break;
            }

            ++it;
        }
    }
    else
    {
        // Find parent entity item.
        Entity *entity = attr->Owner()->ParentEntity();
        assert(entity);
        if (!entity)
            return;

        parentItem = EntityItemOfEntity(entity);
    }

    assert(parentItem);
    if (!parentItem)
    {
        //LogError("");
        return;
    }

    CreateAttributeItem(parentItem, attr);
}

void SceneStructureWindow::RemoveDynamicAttribute(IAttribute *attr)
{
    const bool multipleItems = (attr->TypeId() == cAttributeAssetReferenceList && static_cast<Attribute<AssetReferenceList> *>(attr)->Get().Size() > 1);

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AttributeItem *item = dynamic_cast<AttributeItem *>(*it);
        if (item)
        {
            if (item->ptr.Expired()) // Clean up expired items while at it.
            {
                SAFE_DELETE(item);
            }
            else if (item->ptr.Get() == attr)
            {
                SAFE_DELETE(item);
                if (!multipleItems)
                    break;
            }
        }

        ++it;
    }
}

void SceneStructureWindow::UpdateDynamicAttribute(IAttribute *attr)
{
    const bool multipleItems = (attr->TypeId() == cAttributeAssetReferenceList && static_cast<Attribute<AssetReferenceList> *>(attr)->Get().Size() > 1);

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AttributeItem *item = dynamic_cast<AttributeItem *>(*it);
        if (item && item->ptr.Get() == attr)
        {
            // In case of attribute consisting of multiple items, recreate all items in CreateAttributeItem.
            if (multipleItems)
                CreateAttributeItem(item->parent(), attr);
            else
                item->Update(attr);
            break;
        }

        ++it;
    }
}

void SceneStructureWindow::UpdateEntityName(IAttribute *attr)
{
    EC_Name *nameComp = qobject_cast<EC_Name *>(sender());
    if (!nameComp || (nameComp->ParentEntity() == 0))
        return;

    Entity *entity = nameComp->ParentEntity();
    EntityItem *item = EntityItemOfEntity(entity);
    if (item)
    {
        if (attr == &nameComp->group)
        {
            RemoveEntity(entity);
            AddEntity(entity);
        }
        else if (attr == &nameComp->name)
            item->SetText(entity);
    }
}

void SceneStructureWindow::UpdateComponentName(const QString & /*oldName*/, const QString & /*newName*/)
{
    IComponent *comp = qobject_cast<IComponent *>(sender());
    if (!comp)
        return;

    EntityItem *eItem = EntityItemOfEntity(comp->ParentEntity());
    if (eItem)
    {
        for(int j = 0; j < eItem->childCount(); ++j)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
            if (cItem && cItem->Component().get() == comp /* && (cItem->name == oldName)*/)
                cItem->SetText(comp);
        }
    }
}

void SceneStructureWindow::Sort(int column)
{
    treeWidget->sortItems(column, treeWidget->header()->sortIndicatorOrder());
}

bool SceneStructureWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (searchField && searchField == obj)
    {
        switch (e->type())
        {
            case QEvent::FocusIn:
            {
                QString currentText = searchField->text();
                if (currentText == "Search...")
                {
                    searchField->setText("");
                    searchField->setStyleSheet("color:black;");
                }
                else if (!currentText.isEmpty())
                {
                    // Calling selectAll() directly here won't do anything
                    // as the ongoing QFocusEvent will overwrite what it does.
                    QTimer::singleShot(1, searchField, SLOT(selectAll()));
                }
                break;
            }
            case QEvent::FocusOut:
                if (searchField->text().simplified().isEmpty())
                {
                    searchField->setText(tr("Search..."));
                    searchField->setStyleSheet("color:grey;");
                }
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(obj, e);
}

void SceneStructureWindow::Search(const QString &filter)
{
    TreeWidgetSearch(treeWidget, 0, filter);
}

void SceneStructureWindow::ExpandOrCollapseAll()
{
    treeWidget->blockSignals(true);
    bool treeExpanded = TreeWidgetExpandOrCollapseAll(treeWidget);
    treeWidget->blockSignals(false);
    expandAndCollapseButton->setText(treeExpanded ? tr("Collapse All") : tr("Expand All"));
}

void SceneStructureWindow::CheckTreeExpandStatus(QTreeWidgetItem * /*item*/)
{
    bool anyExpanded = false;
    QTreeWidgetItemIterator iter(treeWidget, QTreeWidgetItemIterator::HasChildren);
    while(*iter)
    {
        QTreeWidgetItem *iterItem = (*iter);
        if (iterItem->isExpanded())
        {
            if (iterItem->parent() && !iterItem->parent()->isExpanded())
                anyExpanded = false;
            else
            {
                anyExpanded = true;
                break;
            }
        }
        ++iter;
    }

    expandAndCollapseButton->setText(anyExpanded ? tr("Collapse All") : tr("Expand All"));
}

void SceneStructureWindow::OnUndoChanged(bool canUndo)
{
    undoButton_->setEnabled(canUndo);
}

void SceneStructureWindow::OnRedoChanged(bool canRedo)
{
    redoButton_->setEnabled(canRedo);
}
