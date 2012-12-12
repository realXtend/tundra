/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   SceneStructureWindow.cpp
 *  @brief  Window with tree view of contents of scene.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"
#include "SceneTreeWidgetItems.h"
#include "TreeWidgetUtils.h"

#include "Framework.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "EC_Name.h"
#include "AssetReference.h"
#include "EC_DynamicComponent.h"

#include <QTreeWidgetItemIterator>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

SceneStructureWindow::SceneStructureWindow(Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    showComponents(true),
    showAssets(true),
    treeWidget(0),
    expandAndCollapseButton(0),
    searchField(0)
{
    // Init main widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(325, 400);

    // Create child widgets
    treeWidget = new SceneTreeWidget(fw, this);
    expandAndCollapseButton = new QPushButton(tr("Expand All"), this);

    searchField = new QLineEdit(this);
    searchField->setText(tr("Search..."));
    searchField->setStyleSheet("color:grey;");
    searchField->installEventFilter(this);

    QLabel *sortLabel = new QLabel(tr("Sort by"));
    QComboBox *sortComboBox = new QComboBox;
    sortComboBox->addItem(tr("ID"));
    sortComboBox->addItem(tr("Name"));

    QCheckBox *compCheckBox = new QCheckBox(tr("Components"), this);
    compCheckBox->setChecked(showComponents);
    QCheckBox *assetCheckBox = new QCheckBox(tr("Asset References"), this);
    assetCheckBox->setChecked(showAssets);

    // Fill layouts
    QHBoxLayout *layoutFilterAndSort = new QHBoxLayout();
    layoutFilterAndSort->addWidget(searchField);
    layoutFilterAndSort->addWidget(sortLabel);
    layoutFilterAndSort->addWidget(sortComboBox);

    QHBoxLayout *layoutSettingsVisibility = new QHBoxLayout;
    layoutSettingsVisibility->addWidget(expandAndCollapseButton);
    layoutSettingsVisibility->addWidget(new QLabel(tr("Show:")));
    layoutSettingsVisibility->addWidget(compCheckBox);
    layoutSettingsVisibility->addWidget(assetCheckBox);
    layoutSettingsVisibility->addSpacerItem(new QSpacerItem(20, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    layout->addLayout(layoutFilterAndSort);
    layout->addWidget(treeWidget);
    layout->addLayout(layoutSettingsVisibility);
    
    // Connect to widget signals
    connect(assetCheckBox, SIGNAL(toggled(bool)), SLOT(ShowAssetReferences(bool)));
    connect(compCheckBox, SIGNAL(toggled(bool)), SLOT(ShowComponents(bool)));
    connect(sortComboBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(Sort(const QString &)));
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

    if (newScene == 0)
    {
        disconnect(scene.lock().get());
        Clear();
        return;
    }

    scene = newScene;
    treeWidget->SetScene(newScene);

    Scene *scenePtr = scene.lock().get();
    connect(scenePtr, SIGNAL(EntityAcked(Entity *, entity_id_t)), SLOT(AckEntity(Entity *, entity_id_t)));
    connect(scenePtr, SIGNAL(EntityCreated(Entity *, AttributeChange::Type)), SLOT(AddEntity(Entity *)));
    connect(scenePtr, SIGNAL(EntityTemporaryStateToggled(Entity *)), SLOT(UpdateEntityTemporaryState(Entity *)));
    connect(scenePtr, SIGNAL(EntityRemoved(Entity *, AttributeChange::Type)), SLOT(RemoveEntity(Entity *)));
    connect(scenePtr, SIGNAL(ComponentAdded(Entity *, IComponent *, AttributeChange::Type)),
        SLOT(AddComponent(Entity *, IComponent *)));
    connect(scenePtr, SIGNAL(ComponentRemoved(Entity *, IComponent *, AttributeChange::Type)),
        SLOT(RemoveComponent(Entity *, IComponent *)));

    Populate();
}

void SceneStructureWindow::ShowComponents(bool show)
{
    showComponents = show;
    treeWidget->showComponents =show;

    treeWidget->setSortingEnabled(false);

    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        for(int j = 0; j < item->childCount(); ++j)
            item->child(j)->setHidden(!showComponents);
    }

    if (showAssets)
    {
        ClearAssetReferences();
        CreateAssetReferences();
    }

    treeWidget->setSortingEnabled(true);

    if (!showAssets && !showComponents)
        expandAndCollapseButton->setEnabled(false);
    else
        expandAndCollapseButton->setEnabled(true);
}

void SceneStructureWindow::ShowAssetReferences(bool show)
{
    showAssets = show;
    //treeWidget->showAssets = show;

    treeWidget->setSortingEnabled(false);

    if (scene.expired())
    {
        Clear();
        return;
    }

    if (!showAssets)
        ClearAssetReferences();
    else
        CreateAssetReferences();

    treeWidget->setSortingEnabled(true);

    if (!showAssets && !showComponents)
        expandAndCollapseButton->setEnabled(false);
    else
        expandAndCollapseButton->setEnabled(true);
}

void SceneStructureWindow::SetEntitySelected(const EntityPtr &entity, bool selected)
{
    if (entity)
        for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
            if (!eItem)
                continue;
            EntityPtr itemEntity = eItem->Entity();
            if (itemEntity && itemEntity == entity)
            {
                QFont font = eItem->font(0);
                font.setBold(selected);
                eItem->setFont(0, font);
                break;
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

void SceneStructureWindow::CreateAssetReferences()
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (!eItem)
            continue;

        EntityPtr entity = scene.lock()->GetEntity(eItem->Id());
        if (!entity)
            continue;

        // Create asset ref items, if any exist.
        // If component items are visible, create asset ref items as children of them.
        if (showComponents)
        {
            int entityChildCount = eItem->childCount();
            for(int j = 0; j < entityChildCount; ++j)
            {
                ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
                if (!cItem)
                    continue;

                ComponentPtr comp = entity->GetComponent(cItem->typeName, cItem->name);
                if (!comp)
                    continue;

                foreach(IAttribute *attr, comp->Attributes())
                    if (attr && (attr->TypeId() == cAttributeAssetReference || attr->TypeId() == cAttributeAssetReferenceList))
                        CreateAssetItem(cItem, attr);
            }
        }
        else
        {
            // Create asset ref items as children of entity items.
            const Entity::ComponentMap &components = entity->Components();
            for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                foreach(IAttribute *attr, i->second->Attributes())
                    if (attr && (attr->TypeId() == cAttributeAssetReference || attr->TypeId() == cAttributeAssetReferenceList))
                        CreateAssetItem(eItem, attr);
        }
    }
}

void SceneStructureWindow::ClearAssetReferences()
{
    QList<AssetRefItem *> assetItems;
    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AssetRefItem *aItem = dynamic_cast<AssetRefItem *>(*it);
        if (aItem)
            assetItems << aItem;
        ++it;
    }

    foreach(AssetRefItem *aItem, assetItems)
    {
        QTreeWidgetItem *p = aItem->parent();
        p->removeChild(aItem);
        SAFE_DELETE(aItem);
    }
}

void SceneStructureWindow::AddEntity(Entity* entity)
{
    EntityItem *item = new EntityItem(entity->shared_from_this());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    treeWidget->addTopLevelItem(item);

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
    for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
    {
        EntityItem *entItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (entItem && (entItem->Id() == entity->Id()))
        {
            entItem->SetText(entity);

            for(int j = 0; j < entItem->childCount(); ++j)
            {
                ComponentItem *compItem = dynamic_cast<ComponentItem *>(entItem->child(j));
                if (compItem && compItem->Component().get() && compItem->Parent() == entItem)
                    compItem->SetText(compItem->Component().get());
            }

            break;
        }
    }
}

void SceneStructureWindow::RemoveEntity(Entity* entity)
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *item = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (item && (item->Id() == entity->Id()))
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void SceneStructureWindow::RemoveEntityById(entity_id_t id)
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *item = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (item && (item->Id() == id))
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void SceneStructureWindow::AddComponent(Entity* entity, IComponent* comp)
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->Id() == entity->Id()))
        {
            ComponentItem *cItem = new ComponentItem(comp->shared_from_this(), eItem);
            cItem->setHidden(!showComponents);

            eItem->addChild(cItem);

            connect(comp, SIGNAL(ComponentNameChanged(const QString &, const QString &)),
                SLOT(UpdateComponentName(const QString &, const QString &)));

            // If name component exists, retrieve name from it. Also hook up change signal so that UI keeps synch with the name.
            if (comp->TypeName() == EC_Name::TypeNameStatic())
            {
                eItem->SetText(entity);

                connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
                    SLOT(UpdateEntityName(IAttribute *)), Qt::UniqueConnection);
            }

            // If dynamic component exists, hook up its change signals in case AssetReference attribute is added/removed to it.
            if (comp->TypeName() == EC_DynamicComponent::TypeNameStatic())
            {
                connect(comp, SIGNAL(AttributeAdded(IAttribute *)), SLOT(AddAssetReference(IAttribute *)));
                connect(comp, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(RemoveAssetReference(IAttribute *)));
                connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
                    SLOT(UpdateAssetReference(IAttribute *)), Qt::UniqueConnection);
            }

            // Add possible asset references.
            foreach(IAttribute *attr, comp->Attributes())
                if (attr && (attr->TypeId() == cAttributeAssetReference || attr->TypeId() == cAttributeAssetReferenceList))
                    CreateAssetItem(cItem, attr);
        }
    }

    // If we have an ongoing search, make sure that the new item is compared too.
    QString searchFilter = searchField->text().trimmed();
    if (!searchFilter.isEmpty() && searchFilter != tr("Search..."))
        TreeWidgetSearch(treeWidget, 0, searchFilter);
}

void SceneStructureWindow::RemoveComponent(Entity* entity, IComponent* comp)
{
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->Id() == entity->Id()))
        {
            for(int j = 0; j < eItem->childCount(); ++j)
            {
                ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
                if (cItem && (cItem->typeName == comp->TypeName()) && (cItem->name == comp->Name()))
                {
                    eItem->removeChild(cItem);
                    SAFE_DELETE(cItem);
                    break;
                }
            }

            if (comp->TypeName() == EC_Name::TypeNameStatic())
                eItem->setText(0, QString("%1").arg(entity->Id()));
        }
    }
}

void SceneStructureWindow::CreateAssetItem(QTreeWidgetItem *parentItem, IAttribute *attr)
{
    Attribute<AssetReference> *assetRef = dynamic_cast<Attribute<AssetReference> *>(attr);
    if (assetRef)
    {
        AssetRefItem *aItem = new AssetRefItem(attr, parentItem);
        aItem->setHidden(!showAssets);
        parentItem->addChild(aItem);
    }
    else
    {
        Attribute<AssetReferenceList> *assetRefList = dynamic_cast<Attribute<AssetReferenceList> *>(attr);
        if (assetRefList)
        {
            AssetReferenceList refs = assetRefList->Get();
            for(int i = 0; i < refs.Size(); ++i)
            {
                AssetRefItem *aItem = new AssetRefItem(attr->Name(), refs[i].ref, parentItem);
                aItem->setHidden(!showAssets);
                parentItem->addChild(aItem);
            }
        }
    }
}

void SceneStructureWindow::AddAssetReference(IAttribute *attr)
{
    if (!dynamic_cast<Attribute<AssetReference> *>(attr) && !dynamic_cast<Attribute<AssetReferenceList> *>(attr))
        return;

    EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent *>(sender());
    if (!dc)
        return;

    if (!showAssets)
        return;

    QTreeWidgetItem *parentItem = 0;
    if (showComponents)
    {
        // Find parent component item
        for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
            if (eItem)
                for(int j = 0; j < eItem->childCount(); ++j)
                {
                    ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
                    if (cItem && (cItem->typeName == dc->TypeName()) && (cItem->name == dc->Name()))
                    {
                        parentItem = cItem;
                        break;
                    }
                }
        }
    }
    else
    {
        // Find parent entity item.
        Entity *entity = dc->ParentEntity();
        assert(entity);
        if (!entity)
            return;

        for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
            if (eItem && (eItem->Id() == entity->Id()))
            {
                parentItem = eItem;
                break;
            }
        }
    }

    assert(parentItem);
    if (!parentItem)
    {
        //LogError("");
        return;
    }

    CreateAssetItem(parentItem, attr);
}

void SceneStructureWindow::RemoveAssetReference(IAttribute *attr)
{
    EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent *>(sender());
    if (!dc)
        return;

    AssetReferenceList assetRefs;
    if (dynamic_cast<Attribute<AssetReference> *>(attr))
        assetRefs.Append(dynamic_cast<Attribute<AssetReference> *>(attr)->Get());
    else if (dynamic_cast<Attribute<AssetReferenceList> *>(attr))
        assetRefs = dynamic_cast<Attribute<AssetReferenceList> *>(attr)->Get();
    else
        return;

    for(int i = 0; i < assetRefs.Size(); ++i)
    {
        //AssetRefItem *assetItem = 0;
        QTreeWidgetItemIterator it(treeWidget);
        while(*it)
        {
            AssetRefItem *a = dynamic_cast<AssetRefItem *>(*it);
            if (a && (/*a->type == assetRef->Get().type && */a->id == assetRefs[i].ref))
            {
                //assetItem = a;
                SAFE_DELETE(a);
                break;
            }

            ++it;
        }
    }

//    SAFE_DELETE(assetItem);
}

void SceneStructureWindow::UpdateAssetReference(IAttribute *attr)
{
    Attribute<AssetReference> *assetRef = dynamic_cast<Attribute<AssetReference> *>(attr);
    if (!assetRef)
        return;

    // Find parent item for the asset reference item.
    IComponent *ownerComp = assetRef->Owner();
    assert(ownerComp);
    Entity *parentEntity = assetRef->Owner()->ParentEntity();
    assert(parentEntity);

    QTreeWidgetItem *parentItem = 0;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem)
        {
            if (!showComponents)
                if (eItem->Id() == parentEntity->Id())
                {
                    parentItem = eItem;
                    break;
                }

            if (showComponents)
                for(int j = 0; j < eItem->childCount(); ++j)
                {
                    ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
                    if (cItem && (cItem->typeName == ownerComp->TypeName()) && (cItem->name == ownerComp->Name()))
                    {
                        parentItem = eItem;
                        break;
                    }
                }

            if (parentItem)
                break;
        }
    }

    // Find asset item with the matching parent.
    AssetRefItem *aItem = 0;
    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AssetRefItem *a = dynamic_cast<AssetRefItem *>(*it);
        /// \todo Should check that the parent matches also?
        if (a && (a->name == assetRef->Name())) //(a->parent() == parentItem))
        {
            aItem = a;
            break;
        }

        ++it;
    }

    assert(aItem);
    if (aItem)
        aItem->SetText(attr);
}

void SceneStructureWindow::UpdateEntityName(IAttribute *attr)
{
    EC_Name *nameComp = dynamic_cast<EC_Name *>(sender());
    if (!nameComp || (attr != &nameComp->name) || (nameComp->ParentEntity() == 0))
        return;

    Entity *entity = nameComp->ParentEntity();
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *item = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (item && (item->Id() == entity->Id()))
        {
            item->SetText(entity);
        }
    }
}

void SceneStructureWindow::UpdateComponentName(const QString &oldName, const QString &newName)
{
    IComponent *comp = dynamic_cast<IComponent *>(sender());
    if (!comp)
        return;

    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *eItem = treeWidget->topLevelItem(i);
        for(int j = 0; j < eItem->childCount(); ++j)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
            if (cItem && (cItem->typeName == comp->TypeName()) && (cItem->name == oldName))
            {
                cItem->SetText(comp);
            }
        }
    }
}

void SceneStructureWindow::Sort(const QString &criteria)
{
    Qt::SortOrder order = treeWidget->header()->sortIndicatorOrder();
    if (order != Qt::AscendingOrder && order != Qt::DescendingOrder)
        order = Qt::AscendingOrder;

    if (criteria == tr("ID"))
        treeWidget->sortItems(0, order);
    if (criteria == tr("Name"))
        treeWidget->sortItems(1, order);
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

void SceneStructureWindow::CheckTreeExpandStatus(QTreeWidgetItem *item)
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
