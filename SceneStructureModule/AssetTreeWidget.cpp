/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetTreeWidget.cpp
 *  @brief  Tree widget showing all available assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetTreeWidget.h"
#include "AddContentWindow.h"
#include "SupportedFileTypes.h"

#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetCache.h"
#include "IAssetTypeFactory.h"
#include "QtUtils.h"

#ifdef _WINDOWS
#include <windows.h>
#endif

#include "MemoryLeakCheck.h"

// AssetItem

AssetItem::AssetItem(const AssetPtr &asset, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
        assetPtr(asset)
{
    setText(0, asset->Name());
}

AssetPtr AssetItem::Asset() const
{
    return assetPtr.lock();
}

// AssetTreeWidget

AssetTreeWidget::AssetTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    contextMenu(0)
{
//    setDragDropMode(QAbstractItemView::DropOnly/*DragDrop*/);
//    setDropIndicatorShown(true);
}

AssetTreeWidget::~AssetTreeWidget()
{
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
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
                e->acceptProposedAction();
    }
    else
        QTreeWidget::dropEvent(e);
}

void AssetTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        foreach(QUrl url, data->urls())
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
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
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
                filenames << url.path();

        if (!filenames.isEmpty())
        {
            e->acceptProposedAction();
            //Upload(filenames);
        }
    }
    else
        QTreeWidget::dropEvent(e);
}

void AssetTreeWidget::AddAvailableActions(QMenu *menu)
{
    QList<AssetItem *> items = GetSelection();
    if (!items.isEmpty())
    {
        QMenu *deleteMenu = new QMenu(tr("Delete"), menu);
        QAction *deleteSourceAction = new QAction(tr("Delete from source"), deleteMenu);
        deleteSourceAction->setDisabled(true);
        QAction *deleteCacheAction = new QAction(tr("Delete from cache"), deleteMenu);
        QAction *forgetAction = new QAction(tr("Forget asset"), deleteMenu);

        deleteMenu->addAction(deleteSourceAction);
        deleteMenu->addAction(deleteCacheAction);
        deleteMenu->addAction(forgetAction);
        menu->addMenu(deleteMenu);

        connect(deleteSourceAction, SIGNAL(triggered()), SLOT(DeleteFromSource()));
        connect(deleteCacheAction, SIGNAL(triggered()), SLOT(DeleteFromCache()));
        connect(forgetAction, SIGNAL(triggered()), SLOT(Forget()));

        QMenu *reloadMenu = new QMenu(tr("Reload"), menu);
        QAction *reloadFromSourceAction = new QAction(tr("Reload from source"), deleteMenu);
        QAction *reloadFromCacheAction = new QAction(tr("Reload from cache"), deleteMenu);
        QAction *unloadAction = new QAction(tr("Unload"), deleteMenu);

        // Reload from cache is not possible if asset's disk source is empty.
        foreach(AssetItem *item, items)
            if (item->Asset() && item->Asset()->DiskSource().trimmed().isEmpty())
            {
                reloadFromCacheAction->setDisabled(true);
                break;
            }

        reloadMenu->addAction(reloadFromSourceAction);
        reloadMenu->addAction(reloadFromCacheAction);
        reloadMenu->addAction(unloadAction);
        menu->addMenu(reloadMenu);

        connect(reloadFromSourceAction, SIGNAL(triggered()), SLOT(ReloadFromSource()));
        connect(reloadFromCacheAction, SIGNAL(triggered()), SLOT(ReloadFromCache()));
        connect(unloadAction, SIGNAL(triggered()), SLOT(Unload()));
    /*
        QAction *exportAction = new QAction(tr("Export..."), menu);
        menu->addAction(exportAction);
        connect(exportAction, SIGNAL(triggered()), SLOT(Export()));
    */
        QAction *openFileLocationAction = new QAction(tr("Open file location"), menu);
        menu->addAction(openFileLocationAction);

        connect(openFileLocationAction, SIGNAL(triggered()), SLOT(OpenFileLocation()));
    }

    QAction *importAction = new QAction(tr("Import..."), menu);
    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    menu->addAction(importAction);
}

QList<AssetItem *> AssetTreeWidget::GetSelection() const
{
    QList<AssetItem *> assetItems;
    foreach(QTreeWidgetItem *item, selectedItems())
    {
        // Omit top-level i.e. asset storage items
        AssetItem *aitem = dynamic_cast<AssetItem *>(item);
        if (aitem)
            assetItems << aitem;
    }

    return assetItems;
}

void AssetTreeWidget::DeleteFromSource()
{
/*
    foreach(AssetItem *item, GetSelection())
        framework->Asset()->ForgetAsset(asset, false);
*/
}

void AssetTreeWidget::DeleteFromCache()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            framework->Asset()->GetAssetCache()->DeleteAsset(item->Asset()->Name());
}

void AssetTreeWidget::Forget()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            framework->Asset()->ForgetAsset(item->Asset(), false);
}

void AssetTreeWidget::Unload()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
        {
            item->Asset()->Unload();
            QTreeWidgetItem *parent = item->parent();
            parent->removeChild(item);
            SAFE_DELETE(item);
            ///\todo Preferrably use the AssetDeleted() or similar signal from AssetAPI for deleting items.
        }
}

void AssetTreeWidget::ReloadFromCache()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            item->Asset()->LoadFromCache();
}

void AssetTreeWidget::ReloadFromSource()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
        {
            QString assetRef = item->Asset()->Name();
            // The next line will delete IAsset from the system, so dereferencing item->Asset() after that would be invalid.
            framework->Asset()->ForgetAsset(item->Asset(), false);
            framework->Asset()->RequestAsset(assetRef);
        }
}

void AssetTreeWidget::Import()
{
    QtUtils::OpenFileDialogNonModal(cAllTypesFileFilter, tr("Import"), "", 0, this, SLOT(OpenFileDialogClosed(int)), true);
}

void AssetTreeWidget::OpenFileDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    if (dialog->selectedFiles().isEmpty())
        return;

    AddContentWindow *addContent = new AddContentWindow(framework, framework->GetDefaultWorldScene());
    addContent->AddFiles(dialog->selectedFiles());
    addContent->show();
}

void AssetTreeWidget::RequestNewAsset()
{
    QStringList types;
    foreach(AssetTypeFactoryPtr factory, framework->Asset()->GetAssetTypeFactories())
        types << factory->Type();
}

void AssetTreeWidget::Export()
{
}

void AssetTreeWidget::Upload()
{
}

void AssetTreeWidget::OpenFileLocation()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset() && !item->Asset()->DiskSource().isEmpty())
        {
#ifdef _WINDOWS
            // Craft command line string
            QString path = boost::filesystem::path(item->Asset()->DiskSource().toStdString()).branch_path().string().c_str();
            WCHAR commandLineStr[256] = {};
            WCHAR wcharPath[256] = {};
            mbstowcs(wcharPath, QDir::toNativeSeparators(path).toStdString().c_str(), 254);
            wsprintf(commandLineStr, L"explorer.exe %s", wcharPath);

            STARTUPINFO startupInfo;
            memset(&startupInfo, 0, sizeof(STARTUPINFO));
            startupInfo.cb = sizeof(STARTUPINFO);
            PROCESS_INFORMATION processInfo;
            memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
            /*BOOL success = */CreateProcessW(NULL, commandLineStr, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
                NULL, NULL, &startupInfo, &processInfo);

            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
#endif
        }
}
