/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AssetTreeWidget.cpp
    @brief  Tree widget showing all available assets. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetTreeWidget.h"
#include "SceneTreeWidgetItems.h"
#include "AddContentWindow.h"
#include "SupportedFileTypes.h"
#include "RequestNewAssetDialog.h"
#include "CloneAssetDialog.h"
#include "FunctionDialog.h"

#include "AssetsWindow.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetStorage.h"
#include "Scene/Scene.h"
#include "AssetCache.h"
#include "FileUtils.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "FunctionInvoker.h"
#include "ArgumentType.h"
#include "IAssetTypeFactory.h"
#include "Win.h"

#include "MemoryLeakCheck.h"

AssetTreeWidget::AssetTreeWidget(Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    contextMenu(0)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setDragDropMode(QAbstractItemView::DropOnly/*DragDrop*/);
    setDropIndicatorShown(true);
}

void AssetTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    // Create context menu and show it.
    SAFE_DELETE(contextMenu);
    contextMenu = new QMenu(this);

    AddAvailableActions(contextMenu);

    contextMenu->popup(e->globalPos());
}

void AssetTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        foreach(QUrl url, data->urls())
            if (!framework->Asset()->GetResourceTypeFromAssetRef(url.path()).isEmpty())
                e->acceptProposedAction();
    }
    else
        QTreeWidget::dragEnterEvent(e);
}

void AssetTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        foreach(QUrl url, data->urls())
            if (!framework->Asset()->GetResourceTypeFromAssetRef(url.path()).isEmpty())
                e->acceptProposedAction();
    }
    else
        QTreeWidget::dragMoveEvent(e);
}

void AssetTreeWidget::dropEvent(QDropEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        QStringList filenames;
        foreach(QUrl url, data->urls())
            if (!framework->Asset()->GetResourceTypeFromAssetRef(url.path()).isEmpty())
            {
                QString filename = url.path();
#ifdef _WINDOWS
                // We have '/' as the first char on windows and the filename
                // is not identified as a file properly. But on other platforms the '/' is valid/required.
                filename = filename.mid(1);
#endif
                filenames << filename;
            }

        if (!filenames.isEmpty())
        {
            e->acceptProposedAction();
            Upload(filenames);
        }
    }
    else
        QTreeWidget::dropEvent(e);
}

void AssetTreeWidget::AddAvailableActions(QMenu *menu)
{
    AssetTreeWidgetSelection sel = SelectedItems();

    QList<QObject *> targets;

    targets << parentWidget();
    foreach(AssetItem *item, sel.assets)
        targets.append(item->Asset().get());
    foreach(AssetStorageItem *item, sel.storages)
        targets.append(item->Storage().get());

    framework->Ui()->EmitContextMenuAboutToOpen(menu, targets);
}

AssetTreeWidgetSelection AssetTreeWidget::SelectedItems() const
{
    AssetTreeWidgetSelection sel;
    foreach(QTreeWidgetItem *item, selectedItems())
    {
        AssetItem *aItem = dynamic_cast<AssetItem *>(item);
        if (aItem)
            sel.assets << aItem;
        else
        {
            AssetStorageItem* sItem = dynamic_cast<AssetStorageItem *>(item);
            if (sItem)
                sel.storages << sItem;
        }
    }

    return sel;
}

void AssetTreeWidget::Upload(const QStringList &files)
{
    Scene *scene = framework->Scene()->MainCameraScene();
    assert(scene);
    AddContentWindow *addContent = new AddContentWindow(scene->shared_from_this(), framework->Ui()->MainWindow());
    addContent->setWindowFlags(Qt::Tool);
    addContent->AddAssets(files);
    addContent->show();
}
