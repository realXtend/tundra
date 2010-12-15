/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.cpp
 *  @brief  
 *
 *          Detailed.
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

AssetsWindow::AssetsWindow(Foundation::Framework *fw) :
    framework(fw)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Assets"));
    resize(300, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

//    QLabel *entityLabel = new QLabel(tr("The following entities will be created:"));

    treeWidget = new AssetTreeWidget(framework, this);
    treeWidget->setHeaderHidden(true);
//    treeWidget ->setColumnCount(3);
//    treeWidget ->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
//    treeWidget ->header()->setResizeMode(QHeaderView::ResizeToContents);

    QHBoxLayout *hlayout= new QHBoxLayout;
    QLabel *searchLabel = new QLabel(tr("Search filter:"), this);
    QLineEdit *searchField = new QLineEdit(this);
    QPushButton *expandAndCollapseButton = new QPushButton(tr("Expand/collapse all"), this);
    hlayout->addWidget(searchLabel);
    hlayout->addWidget(searchField);
    hlayout->addWidget(expandAndCollapseButton);

    layout->addLayout(hlayout);
    layout->addWidget(treeWidget);

    // Create "No provider" for assets without storage.
    noProviderItem = new QTreeWidgetItem;
    noProviderItem->setText(0, tr("No provider"));

    PopulateTreeWidget();

    connect(searchField, SIGNAL(textChanged(const QString &)), SLOT(Search(const QString &)));
    connect(expandAndCollapseButton, SIGNAL(clicked()), SLOT(ExpandOrCollapseAll()));

    connect(framework->Asset(), SIGNAL(AssetCreated(AssetPtr)), SLOT(AddAsset(AssetPtr)));
    connect(framework->Asset(), SIGNAL(AssetAboutToBeRemoved(AssetPtr)), SLOT(RemoveAsset(AssetPtr)));
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
    foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, storage->ToString());
        treeWidget->addTopLevelItem(item);
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
        for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
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
    TreeWidgetSearch(treeWidget, filter);
}

void AssetsWindow::ExpandOrCollapseAll()
{
    TreeWidgetExpandOrCollapseAll(treeWidget);
}
