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
#include "RequestNewAssetDialog.h"

#include "AssetsWindow.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetCache.h"
#include "QtUtils.h"
#include "UiAPI.h"

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
    MarkUnloaded(!asset->IsLoaded());
}

AssetPtr AssetItem::Asset() const
{
    return assetPtr.lock();
}

void AssetItem::MarkUnloaded(bool value)
{
    QString unloaded = QApplication::translate("AssetItem", " (Unloaded)");
    if (value)
        setText(0, text(0) + unloaded);
    else
        setText(0, text(0).remove(unloaded));
}

// AssetTreeWidget

AssetTreeWidget::AssetTreeWidget(Foundation::Framework *fw, QWidget *parent) :
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
            if (!AssetAPI::GetResourceTypeFromAssetRef(url.path()).isEmpty())
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
            if (!AssetAPI::GetResourceTypeFromAssetRef(url.path()).isEmpty())
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
            if (!AssetAPI::GetResourceTypeFromAssetRef(url.path()).isEmpty())
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
    QList<AssetItem *> items = GetSelection();
    if (!items.isEmpty())
    {
        QMenu *deleteMenu = new QMenu(tr("Delete"), menu);
        QAction *deleteSourceAction = new QAction(tr("Delete from source"), deleteMenu);
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

        QAction *openFileLocationAction = new QAction(tr("Open file location"), menu);
#ifndef _WINDOWS
        openFileLocationAction->setDisabled(true);
#endif
        menu->addAction(openFileLocationAction);

        connect(openFileLocationAction, SIGNAL(triggered()), SLOT(OpenFileLocation()));

        menu->addSeparator();

        QAction *exportAction = new QAction(tr("Export..."), menu);
        menu->addAction(exportAction);
        connect(exportAction, SIGNAL(triggered()), SLOT(Export()));
    }

    QAction *importAction = new QAction(tr("Import..."), menu);
    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    menu->addAction(importAction);

    QAction *requestNewAssetAction = new QAction(tr("Request new asset..."), menu);
    connect(requestNewAssetAction, SIGNAL(triggered()), SLOT(RequestNewAsset()));
    menu->addAction(requestNewAssetAction);

    if (selectedItems().count() == 1)
    {
        QAction *makeDefaultStorageAction = new QAction(tr("Make default storage"), menu);
        connect(makeDefaultStorageAction, SIGNAL(triggered()), SLOT(MakeDefaultStorage()));
        menu->addAction(makeDefaultStorageAction);
    }

    if (selectedItems().count() > 0)
    {
        QAction *removeStorageAction = new QAction(tr("Remove storage"), menu);
        connect(removeStorageAction, SIGNAL(triggered()), SLOT(RemoveStorage()));
        menu->addAction(removeStorageAction);
    }

    // Let other instances add their possible functionality.
    QList<QObject *> targets;
    foreach(AssetItem *item, items)
        targets.append(item->Asset().get());
    framework->Ui()->EmitContextMenuAboutToOpen(menu, targets);
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
    int ret = QMessageBox::warning(
        this,
        tr("Delete From Source"),
        tr("Are you sure want to delete the selected asset(s) permanently from the source?"),
        QMessageBox::Ok | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (ret == QMessageBox::Ok)
    {
        // AssetAPI::DeleteAssetFromStorage() signals will start deletion of tree widget asset items:
        // Gather the asset refs to a separate list beforehand in order to prevent crash.
        QStringList assetRefs;
        foreach(AssetItem *item, GetSelection())
            if (item->Asset())
                assetRefs << item->Asset()->Name();

        foreach(QString ref, assetRefs)
            framework->Asset()->DeleteAssetFromStorage(ref);
    }
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
            // Do not delete item, instead mark it as unloaded in AssetsWindow.
            //QTreeWidgetItem *parent = item->parent();
            //parent->removeChild(item);
            //SAFE_DELETE(item);
            ///\todo Preferably use the AssetDeleted() or similar signal from AssetAPI for deleting items.
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

    Upload(dialog->selectedFiles());
}

void AssetTreeWidget::RequestNewAsset()
{
    RequestNewAssetDialog *dialog = new RequestNewAssetDialog(framework->Asset(), this);
    connect(dialog, SIGNAL(finished(int)), SLOT(RequestNewAssetDialogClosed(int)));
    dialog->show();
}

void AssetTreeWidget::MakeDefaultStorage()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.size() == 1)
    {
        QString storageName = selected.first()->data(0, Qt::UserRole).toString();
        framework->Asset()->SetDefaultAssetStorage(framework->Asset()->GetAssetStorage(storageName));
    }

    AssetsWindow *parent = dynamic_cast<AssetsWindow*>(parentWidget());
    if (parent)
        parent->PopulateTreeWidget();
}

void AssetTreeWidget::RemoveStorage()
{
    foreach(QTreeWidgetItem *item, selectedItems())
    {
        QString storageName = item->data(0, Qt::UserRole).toString();
        framework->Asset()->RemoveAssetStorage(storageName);
    }

    AssetsWindow *parent = dynamic_cast<AssetsWindow*>(parentWidget());
    if (parent)
        parent->PopulateTreeWidget();
}

void AssetTreeWidget::RequestNewAssetDialogClosed(int result)
{
    RequestNewAssetDialog *dialog = qobject_cast<RequestNewAssetDialog*>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    framework->Asset()->RequestAsset(dialog->Source(), dialog->Type());
}

void AssetTreeWidget::Export()
{
    QList<AssetItem *> sel = GetSelection();
    if (sel.isEmpty())
        return;

    if (sel.size() == 1)
    {
        QString ref = sel.first()->Asset() ? sel.first()->Asset()->Name() : "";
        QString assetName= AssetAPI::ExtractFilenameFromAssetRef(ref);
        QtUtils::SaveFileDialogNonModal("", tr("Save Asset As"), assetName, 0, this, SLOT(SaveAssetDialogClosed(int)));
    }
    else
    {
        QtUtils::DirectoryDialogNonModal(tr("Select Directory"), "", 0, this, SLOT(SaveAssetDialogClosed(int)));
    }
}

void AssetTreeWidget::SaveAssetDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);

    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty())
        return;

    QStringList files = dialog->selectedFiles();

    QList<AssetItem *> sel = GetSelection();

    bool isDir = QDir(files[0]).exists();

    if ((sel.size() == 1 && isDir) || (sel.size() > 1 && !isDir))
    {
        // should not happen normally, so just log error. No prompt for user.
//        LogError("Could not save asset: no such directory.");
        return;
    }

    foreach(AssetItem *item, sel)
        if (item->Asset())
        {
            // if saving multiple assets, append filename to directory
            QString filename = files[0];
            if (isDir)
            {
                QString assetName = AssetAPI::ExtractFilenameFromAssetRef(item->Asset()->Name());
               //while(QFile::exists(filename))
                    //filename.append("_");
                filename += QDir::separator() + assetName;
            }

            QString param;
            if (item->Asset()->Type().contains("texture", Qt::CaseInsensitive))
                param = filename.right(filename.size() - filename.lastIndexOf('.') - 1);

            item->Asset()->SaveToFile(filename, param);
        }
}

void AssetTreeWidget::Upload(const QStringList &files)
{
    AddContentWindow *addContent = new AddContentWindow(framework, framework->Scene()->GetDefaultScene());
    addContent->AddFiles(files);
    addContent->show();
}

void AssetTreeWidget::OpenFileLocation()
{
    QList<AssetItem *> selection = GetSelection();
    if (selection.isEmpty() || selection.size() < 1)
        return;

    AssetItem *item = selection.first();
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
