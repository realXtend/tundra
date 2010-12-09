/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.cpp
 *  @brief  
 *
 *          Detailed.
 */

#include "StableHeaders.h"
#include "AssetsWindow.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetStorage.h"

#include "MemoryLeakCheck.h"

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

    treeWidget = new QTreeWidget;
    treeWidget->setHeaderHidden(true);
//    treeWidget ->setColumnCount(3);
//    treeWidget ->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
//    treeWidget ->header()->setResizeMode(QHeaderView::ResizeToContents);

    layout->addWidget(treeWidget);

    PopulateTreeWidget();
}

AssetsWindow::~AssetsWindow()
{
    // Disable ResizeToContents, Qt goes sometimes into eternal loop after
    // ~AddContentWindow() if we have lots (hudreds or thousands) of items.
    treeWidget->header()->setResizeMode(QHeaderView::Interactive);

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        QTreeWidgetItem *item = *it;
        SAFE_DELETE(item);
        ++it;
    }
}

void AssetsWindow::PopulateTreeWidget()
{
    foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, storage->Name());
        treeWidget->addTopLevelItem(item);
    }

    // Create "No provider" for assets without storage.
    QTreeWidgetItem *noProviderItem = new QTreeWidgetItem;
    noProviderItem->setText(0, tr("No provider"));
    treeWidget->addTopLevelItem(noProviderItem);

    std::pair<QString, AssetPtr> pair;
    foreach(pair, framework->Asset()->GetAllAssets())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, pair.first);

        AssetStoragePtr storage = pair.second->GetAssetStorage();
        bool storageFound = false;
        if (storage)
            for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
            {
                QTreeWidgetItem *storageItem = treeWidget->topLevelItem(i);
                if (storageItem->text(0) == storage->Name())
                {
                    storageItem->addChild(item);
                    storageFound = true;
                    break;
                }
            }

        if (!storageFound)
            noProviderItem->addChild(item);
    }

    if (noProviderItem->childCount() == 0)
        SAFE_DELETE(noProviderItem);
}
