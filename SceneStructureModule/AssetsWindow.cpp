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

#include "Framework.h"
#include "AssetAPI.h"
#include "IAsset.h"

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
    std::pair<QString, AssetPtr> pair;
    foreach(pair, framework->Asset()->GetAllAssets())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, pair.first);
        treeWidget->addTopLevelItem(item);
    }
}
