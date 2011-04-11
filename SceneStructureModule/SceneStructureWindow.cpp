/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.cpp
 *  @brief  Window with tree view of contents of scene.
 *
 *          This class will only handle adding and removing of entities and components and updating
 *          their names. The SceneTreeWidget implements most of the functionality.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"
#include "SceneTreeWidgetItems.h"
#include "TreeWidgetUtils.h"

#include "Framework.h"
#include "SceneManager.h"
#include "Entity.h"
#include "EC_Name.h"
#include "AssetReference.h"
//#ifdef EC_DynamicComponent_ENABLED
#include "EC_DynamicComponent.h"
//#endif

#include <QTreeWidgetItemIterator>

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneStructureWindow")

#include "MemoryLeakCheck.h"

using namespace Scene;

SceneStructureWindow::SceneStructureWindow(Foundation::Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    showComponents(true),
    showAssets(true),
    treeWidget(0),
    expandAndCollapseButton(0),
    searchField(0)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // Init main widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(300, 400);

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
    treeWidget->SetScene(s);

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

    treeWidget->setSortingEnabled(false);

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
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

    for(SceneManager::iterator it = s->begin(); it != s->end(); ++it)
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
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (!eItem)
            continue;

        Scene::EntityPtr entity = scene.lock()->GetEntity(eItem->Id());
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

                foreach(IAttribute *attr, comp->GetAttributes())
                    if (attr->TypeName() == "assetreference" || attr->TypeName() == "assetreferencelist")
                        CreateAssetItem(cItem, attr);
            }
        }
        else
        {
            // Create asset ref items as children of entity items.
            foreach(ComponentPtr comp, entity->Components())
                foreach(IAttribute *attr, comp->GetAttributes())
                    if (attr->TypeName() == "assetreference" || attr->TypeName() == "assetreferencelist")
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

void SceneStructureWindow::AddEntity(Scene::Entity* entity)
{
    EntityItem *item = new EntityItem(entity->shared_from_this());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    DecorateEntityItem(entity, item);

    treeWidget->addTopLevelItem(item);

    foreach(ComponentPtr c, entity->Components())
        AddComponent(entity, c.get());
}

void SceneStructureWindow::RemoveEntity(Scene::Entity* entity)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *item = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (item && (item->Id() == entity->GetId()))
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
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->Id() == entity->GetId()))
        {
            ComponentItem *cItem = new ComponentItem(comp->shared_from_this(), eItem);
            cItem->setHidden(!showComponents);

            DecorateComponentItem(comp, cItem);

            eItem->addChild(cItem);

            connect(comp, SIGNAL(ComponentNameChanged(const QString &, const QString &)),
                SLOT(UpdateComponentName(const QString &, const QString &)));

            // If name component exists, retrieve name from it. Also hook up change signal so that UI keeps synch with the name.
            if (comp->TypeName() == EC_Name::TypeNameStatic())
            {
                eItem->SetText(entity);
                DecorateEntityItem(entity, eItem);

                connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
                    SLOT(UpdateEntityName(IAttribute *)), Qt::UniqueConnection);
            }

//#ifdef EC_DynamicComponent_ENABLED
            // If dynamic component exists, hook up its change signals in case AssetReference attribute is added/removed to it.
            if (comp->TypeName() == EC_DynamicComponent::TypeNameStatic())
            {
                connect(comp, SIGNAL(AttributeAdded(IAttribute *)), SLOT(AddAssetReference(IAttribute *)));
                connect(comp, SIGNAL(AttributeAboutToBeRemoved(IAttribute *)), SLOT(RemoveAssetReference(IAttribute *)));
                connect(comp, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
                    SLOT(UpdateAssetReference(IAttribute *)), Qt::UniqueConnection);
            }
//#endif
            // Add possible asset references.
            foreach(IAttribute *attr, comp->GetAttributes())
                if (attr->TypeName() == "assetreference" || attr->TypeName() == "assetreferencelist")
                    CreateAssetItem(cItem, attr);
        }
    }
}

void SceneStructureWindow::RemoveComponent(Scene::Entity* entity, IComponent* comp)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem && (eItem->Id() == entity->GetId()))
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
                eItem->setText(0, QString("%1").arg(entity->GetId()));
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
                AssetRefItem *aItem = new AssetRefItem(attr->GetName(), refs[i].ref, parentItem);
                aItem->setHidden(!showAssets);
                parentItem->addChild(aItem);
            }
        }
    }
}

void SceneStructureWindow::DecorateEntityItem(Scene::Entity *entity, QTreeWidgetItem *item) const
{
    bool local = entity->IsLocal();
    bool temp = entity->IsTemporary();

    QString info;
    if (local)
    {
        item->setTextColor(0, QColor(Qt::blue));
        info.append("Local");
    }

    if (temp)
    {
        item->setTextColor(0, QColor(Qt::red));
        if (!info.isEmpty())
            info.append(" ");
        info.append("Temporary");
    }

    if (!info.isEmpty())
    {
        QString text = item->text(0);
        if (text.size() > 0 && text[text.size()-1] != ' ')
            text.append(" ");
        info.prepend("(");
        info.append(")");
        item->setText(0, text + info);
    }
}

void SceneStructureWindow::DecorateComponentItem(IComponent *comp, QTreeWidgetItem *item) const
{
    bool sync = comp->GetNetworkSyncEnabled();
    bool temporary = comp->IsTemporary();

    QString info;
    if (!sync)
    {
        item->setTextColor(0, QColor(Qt::blue));
        info.append("Local");
    }

    if (temporary)
    {
        item->setTextColor(0, QColor(Qt::red));
        if (!info.isEmpty())
            info.append(" ");
        info.append("Temporary");
    }

    if (comp->GetUpdateMode() == AttributeChange::LocalOnly)
    {
        if (!info.isEmpty())
            info.append(" ");
        info.append("UpdateMode:LocalOnly");
    }

    if (comp->GetUpdateMode() == AttributeChange::Disconnected)
    {
        if (!info.isEmpty())
            info.append(" ");
        info.append("UpdateMode:Disconnected");
    }

    if (!info.isEmpty())
    {
        QString text = item->text(0);
        if (text.size() > 0 && text[text.size()-1] != ' ')
            text.append(" ");
        info.prepend("(");
        info.append(")");
        item->setText(0, text + info);
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
        for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
            if (eItem)
                for (int j = 0; j < eItem->childCount(); ++j)
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
        Scene::Entity *entity = dc->GetParentEntity();
        assert(entity);
        if (!entity)
            return;

        for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
            if (eItem && (eItem->Id() == entity->GetId()))
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
        assetRefs.Append(dynamic_cast<Attribute<AssetReference> *>(attr)->Get().ref);
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
    IComponent *ownerComp = assetRef->GetOwner();
    assert(ownerComp);
    Scene::Entity *parentEntity = assetRef->GetOwner()->GetParentEntity();
    assert(parentEntity);

    QTreeWidgetItem *parentItem = 0;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *eItem = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (eItem)
        {
            if (!showComponents)
                if (eItem->Id() == parentEntity->GetId())
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
        if (a && (a->name == assetRef->GetName())) //(a->parent() == parentItem))
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
    if (!nameComp || (attr != &nameComp->name) || (nameComp->GetParentEntity() == 0))
        return;

    Entity *entity = nameComp->GetParentEntity();
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        EntityItem *item = dynamic_cast<EntityItem *>(treeWidget->topLevelItem(i));
        if (item && (item->Id() == entity->GetId()))
        {
            item->SetText(entity);
            DecorateEntityItem(entity, item);
        }
    }
}

void SceneStructureWindow::UpdateComponentName(const QString &oldName, const QString &newName)
{
    IComponent *comp = dynamic_cast<IComponent *>(sender());
    if (!comp)
        return;

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *eItem = treeWidget->topLevelItem(i);
        for (int j = 0; j < eItem->childCount(); ++j)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
            if (cItem && (cItem->typeName == comp->TypeName()) && (cItem->name == oldName))
            {
                cItem->SetText(comp);
                DecorateComponentItem(comp, cItem);
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
    bool treeExpanded = TreeWidgetExpandOrCollapseAll(treeWidget);
    expandAndCollapseButton->setText(treeExpanded ? tr("Collapse All") : tr("Expand All"));
}

void SceneStructureWindow::CheckTreeExpandStatus(QTreeWidgetItem *item)
{
    bool anyExpanded = false;
    QTreeWidgetItemIterator iter(treeWidget, QTreeWidgetItemIterator::HasChildren);
    while (*iter) 
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

    if (anyExpanded)
        expandAndCollapseButton->setText(tr("Collapse All"));
    else
        expandAndCollapseButton->setText(tr("Expand All"));
}
