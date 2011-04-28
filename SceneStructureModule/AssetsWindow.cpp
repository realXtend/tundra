/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.cpp
 *  @brief  The main UI for managing asset storages and assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetsWindow.h"
#include "AssetTreeWidget.h"
#include "TreeWidgetUtils.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetStorage.h"

#include "MemoryLeakCheck.h"

namespace
{
    bool HasSameRefAsPredecessors(QTreeWidgetItem *item)
    {
        QTreeWidgetItem *parent = 0, *child = item;
        while((parent = child->parent()) != 0)
        {
            if (parent->text(0).compare(child->text(0), Qt::CaseInsensitive) == 0)
                return true;
            child = parent;
        }

        return false;
    }
}

AssetsWindow::AssetsWindow(Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    searchField(0),
    expandAndCollapseButton(0)
{
    // Init main widget
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Assets"));
    resize(300, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

    // Create child widgets
    treeWidget = new AssetTreeWidget(framework, this);
    treeWidget->setHeaderHidden(true);

    searchField = new QLineEdit(this);
    searchField->setText(tr("Search..."));
    searchField->setStyleSheet("color:grey;");
    searchField->installEventFilter(this);

    expandAndCollapseButton = new QPushButton(tr("Expand All"), this);
    
    QHBoxLayout *hlayout= new QHBoxLayout;
    hlayout->addWidget(searchField);
    hlayout->addWidget(expandAndCollapseButton);

    layout->addLayout(hlayout);
    layout->addWidget(treeWidget);

    PopulateTreeWidget();

    connect(searchField, SIGNAL(textEdited(const QString &)), SLOT(Search(const QString &)));
    connect(expandAndCollapseButton, SIGNAL(clicked()), SLOT(ExpandOrCollapseAll()));

    connect(framework->Asset(), SIGNAL(AssetCreated(AssetPtr)), SLOT(AddAsset(AssetPtr)));
    connect(framework->Asset(), SIGNAL(AssetAboutToBeRemoved(AssetPtr)), SLOT(RemoveAsset(AssetPtr)));

    connect(treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(CheckTreeExpandStatus(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(CheckTreeExpandStatus(QTreeWidgetItem*)));
}

AssetsWindow::~AssetsWindow()
{
    // Disable ResizeToContents, Qt goes sometimes into eternal loop after
    // ~AssetsWindow() if we have lots (hudreds or thousands) of items.
    treeWidget->header()->setResizeMode(QHeaderView::Interactive);

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        QTreeWidgetItem *item = *it;
        SAFE_DELETE(item);
        ++it;
    }
}

void AssetsWindow::AddChildren(const AssetPtr &asset, QTreeWidgetItem *parent)
{
    foreach(AssetReference ref, asset->FindReferences())
    {
        AssetPtr asset = framework->Asset()->GetAsset(ref.ref);
        if (asset && alreadyAdded.find(asset) == alreadyAdded.end())
        {
            AssetItem *item = new AssetItem(asset, parent);
            parent->addChild(item);
            alreadyAdded.insert(asset);

            // Check for recursive dependencies.
            if (HasSameRefAsPredecessors(item))
                item->setText(0, tr("Recursive dependency to ") + asset->Name());
            else
                AddChildren(asset, item);
        }
    }
}

void AssetsWindow::PopulateTreeWidget()
{
    treeWidget->clear();
	alreadyAdded.clear();
    // Create "No provider" for assets without storage.
    noProviderItem = new QTreeWidgetItem;
    noProviderItem->setText(0, tr("No provider"));

    AssetStoragePtr defaultStorage = framework->Asset()->GetDefaultAssetStorage();

    foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, storage->ToString());
        treeWidget->addTopLevelItem(item);

        item->setData(0, Qt::UserRole, QVariant(storage->Name()));

        // The current default storage is bolded.
        if (storage == defaultStorage)
        {
            QFont font = item->font(0);
            font.setBold(true);
            item->setFont(0, font);
        }
    }

    std::pair<QString, AssetPtr> pair;
    foreach(pair, framework->Asset()->GetAllAssets())
        if (alreadyAdded.find(pair.second) == alreadyAdded.end())
            AddAsset(pair.second);

    treeWidget->addTopLevelItem(noProviderItem);
    noProviderItem->setHidden(noProviderItem->childCount() == 0);
}

void AssetsWindow::AddAsset(AssetPtr asset)
{
    ///\todo Check that the asset doesn't already exists
    AssetItem *item = new AssetItem(asset);
    AddChildren(asset, item);

    bool storageFound = false;
    AssetStoragePtr storage = asset->GetAssetStorage();
    if (storage)
        for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *storageItem = treeWidget->topLevelItem(i);
            if (storageItem->text(0) == storage->ToString())
            {
                storageItem->addChild(item);
                storageFound = true;
                break;
            }
        }

    if (!storageFound)
        noProviderItem->addChild(item);

    noProviderItem->setHidden(noProviderItem->childCount() == 0);
}

void AssetsWindow::RemoveAsset(AssetPtr asset)
{
    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        AssetItem *item = dynamic_cast<AssetItem *>(*it);
        if (item && item->Asset() && item->Asset() == asset)
        {
            QTreeWidgetItem *parent = item->parent();
            parent->removeChild(item);
            SAFE_DELETE(item);
        }

        ++it;
    }
}

void AssetsWindow::Search(const QString &filter)
{
    TreeWidgetSearch(treeWidget, 0, filter);
}

void AssetsWindow::ExpandOrCollapseAll()
{
    bool treeExpanded = TreeWidgetExpandOrCollapseAll(treeWidget);
    expandAndCollapseButton->setText(treeExpanded ? tr("Collapse All") : tr("Expand All"));
}

void AssetsWindow::CheckTreeExpandStatus(QTreeWidgetItem *item)
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

    if (anyExpanded)
        expandAndCollapseButton->setText(tr("Collapse All"));
    else
        expandAndCollapseButton->setText(tr("Expand All"));
}


bool AssetsWindow::eventFilter(QObject *obj, QEvent *e)
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
