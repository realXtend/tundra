/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AssetItemMenuHandler.cpp
    @brief  Handles populating of assets and asset storages context menus and their chosen actions. */

#include "StableHeaders.h"

#include "AssetItemMenuHandler.h"
#include "ArgumentType.h"
#include "RequestNewAssetDialog.h"
#include "CloneAssetDialog.h"
#include "FunctionDialog.h"
#include "FunctionInvoker.h"
#include "AssetsWindow.h"
#include "AddContentWindow.h"
#include "SupportedFileTypes.h"

#include "UiAPI.h"
#include "AssetAPI.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "IAsset.h"
#include "IAssetTypeFactory.h"
#include "AssetCache.h"
#include "UiMainWindow.h"
#include "FileUtils.h"

#include "MemoryLeakCheck.h"

AssetItemMenuHandler::AssetItemMenuHandler(Framework *fw) :
    framework_(fw),
    sender_(0)
{
    connect(framework_->Ui(), SIGNAL(ContextMenuAboutToOpen(QMenu *, QList<QObject *>)), this, SLOT(AddAssetMenuItems(QMenu *, QList<QObject *>)), Qt::UniqueConnection);
}

void AssetItemMenuHandler::AddAssetMenuItems(QMenu * menu, QList<QObject *> targets)
{
    AssetAndStorageItems items;

    foreach (QObject *target, targets)
    {
        IAsset * asset = dynamic_cast<IAsset*>(target);
        if (asset)
            items.assets << asset;
        else
        {
            IAssetStorage* storage = dynamic_cast<IAssetStorage*>(target);
            if (storage)
                items.storages << storage;
            else
                if (dynamic_cast<AssetsWindow*> (target))
                    sender_ = target;
        }
    }

    if (items.assets.isEmpty() && items.storages.isEmpty())
        return;

    targets_ = items;

    if (!targets_.assets.isEmpty())
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
        
        // Reload from cache & delete from cache are not possible for e.g. local assets don't have a cached version of the asset,
        // Even if the asset is an HTTP asset, these options are disable if there does not exist a cached version of that asset in the cache.
        foreach(IAsset *item, targets_.assets)
        {
            if (item && framework_->Asset()->GetAssetCache()->FindInCache(item->Name()).isEmpty())
            {
                reloadFromCacheAction->setDisabled(true);
                deleteCacheAction->setDisabled(true);
                break;
            }
        }
        reloadMenu->addAction(reloadFromSourceAction);
        reloadMenu->addAction(reloadFromCacheAction);
        reloadMenu->addAction(unloadAction);
        menu->addMenu(reloadMenu);
        
        connect(reloadFromSourceAction, SIGNAL(triggered()), SLOT(ReloadFromSource()));
        connect(reloadFromCacheAction, SIGNAL(triggered()), SLOT(ReloadFromCache()));
        connect(unloadAction, SIGNAL(triggered()), SLOT(Unload()));
        
        QAction *openFileLocationAction = new QAction(tr("Open file location"), menu);
        menu->addAction(openFileLocationAction);
        connect(openFileLocationAction, SIGNAL(triggered()), SLOT(OpenFileLocation()));
        
        QAction *openInExternalEditor = new QAction(tr("Open in external editor"), menu);
        menu->addAction(openInExternalEditor);
        connect(openInExternalEditor, SIGNAL(triggered()), SLOT(OpenInExternalEditor()));
        
        // Delete from Source, Delete from Cache, Reload from Source, Unload, Open File Location, and Open in external editor
        // are not applicable for assets which have been created programmatically (disk source is empty).
        ///\todo Currently disk source is empty for unloaded assets, and open file location is disabled for them. This should not happen.
        foreach(IAsset *item, targets_.assets)
        {
            if (item && item->DiskSource().trimmed().isEmpty())
            {
                deleteCacheAction->setDisabled(true);
                // If asset is an external URL, do not disable deleteFromSource & reloadFromSource
                if (AssetAPI::ParseAssetRef(item->Name()) != AssetAPI::AssetRefExternalUrl)
                {
                    deleteSourceAction->setDisabled(true);
                    reloadFromSourceAction->setDisabled(true);
                }
                unloadAction->setDisabled(true);
                openFileLocationAction->setDisabled(true);
                openInExternalEditor->setDisabled(true);
                
                break;
            }
        }

        menu->addSeparator();
        
        if (targets_.assets.count() == 1)
        {
            QAction *copyAssetRefAction = new QAction(tr("Copy asset reference to clipboard"), menu);
            menu->addAction(copyAssetRefAction);
            connect(copyAssetRefAction, SIGNAL(triggered()), SLOT(CopyAssetRef()));
        }
        
        QAction *cloneAction = new QAction(tr("Clone..."), menu);
        menu->addAction(cloneAction);
        connect(cloneAction, SIGNAL(triggered()), SLOT(Clone()));
        
        QAction *exportAction = new QAction(tr("Export..."), menu);
        menu->addAction(exportAction);
        connect(exportAction, SIGNAL(triggered()), SLOT(Export()));
    }

    QAction *functionsAction = new QAction(tr("Functions..."), menu);
    connect(functionsAction, SIGNAL(triggered()), SLOT(OpenFunctionDialog()));
    menu->addAction(functionsAction);
    // "Functions..." is disabled if we have both assets and storages selected simultaneously.
    if (!targets_.assets.isEmpty() && !targets_.storages.isEmpty())
        functionsAction->setDisabled(true);

    QAction *importAction = new QAction(tr("Import..."), menu);
    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    menu->addAction(importAction);

    QAction *requestNewAssetAction = new QAction(tr("Request new asset..."), menu);
    connect(requestNewAssetAction, SIGNAL(triggered()), SLOT(RequestNewAsset()));
    menu->addAction(requestNewAssetAction);

    QMenu *createMenu = new QMenu(tr("Create"), menu);
    menu->addMenu(createMenu);
    foreach(const AssetTypeFactoryPtr &factory, framework_->Asset()->GetAssetTypeFactories())
    {
        QAction *createAsset = new QAction(factory->Type(), createMenu);
        createAsset->setObjectName(factory->Type());
        connect(createAsset, SIGNAL(triggered()), SLOT(CreateAsset()));
        createMenu->addAction(createAsset);
    }

    if (!targets_.storages.isEmpty())
    {
        if (targets_.storages.count() == 1)
        {
            QAction *makeDefaultStorageAction = new QAction(tr("Make default storage"), menu);
            connect(makeDefaultStorageAction, SIGNAL(triggered()), SLOT(MakeDefaultStorage()));
            menu->addAction(makeDefaultStorageAction);
        }

        if (targets_.storages.count() > 0)
        {
            QAction *removeStorageAction = new QAction(tr("Remove storage"), menu);
            connect(removeStorageAction, SIGNAL(triggered()), SLOT(RemoveStorage()));
            menu->addAction(removeStorageAction);
        }
    }
}

void AssetItemMenuHandler::RequestNewAsset()
{
    RequestNewAssetDialog *dialog = new RequestNewAssetDialog(framework_->Asset(), NULL);
    connect(dialog, SIGNAL(finished(int)), SLOT(RequestNewAssetDialogClosed(int)));
    dialog->show();
}

void AssetItemMenuHandler::RequestNewAssetDialogClosed(int result)
{
    RequestNewAssetDialog *dialog = qobject_cast<RequestNewAssetDialog*>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    framework_->Asset()->RequestAsset(dialog->Source(), dialog->Type());
}

void AssetItemMenuHandler::CreateAsset()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    QString assetType = action->objectName();
    QString assetName = framework_->Asset()->GenerateUniqueAssetName(assetType, tr("New"));
    framework_->Asset()->CreateNewAsset(assetType, assetName);
}

void AssetItemMenuHandler::OpenFileLocation()
{
    if (targets_.assets.isEmpty() || targets_.assets.size() < 1)
        return;
    
    IAsset *item = targets_.assets.first();
    if (item && !item->DiskSource().isEmpty())
    {
        bool success = false;
#ifdef _WINDOWS
        // Custom code for Windows, as we want to use explorer.exe with the /select switch.
        // Craft command line string, use the full filename, not directory.
        QString path = QDir::toNativeSeparators(item->DiskSource());
        WCHAR commandLineStr[256] = {};
        WCHAR wcharPath[256] = {};
        mbstowcs(wcharPath, path.toStdString().c_str(), 254);
        wsprintf(commandLineStr, L"explorer.exe /select,%s", wcharPath);
        
        STARTUPINFO startupInfo;
        memset(&startupInfo, 0, sizeof(STARTUPINFO));
        startupInfo.cb = sizeof(STARTUPINFO);
        PROCESS_INFORMATION processInfo;
        memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
        success = CreateProcessW(NULL, commandLineStr, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
                                 NULL, NULL, &startupInfo, &processInfo);
        
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
#else
        QString path = QDir::toNativeSeparators(QFileInfo(item->DiskSource()).dir().path());
        success = QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
#endif
        if (!success)
            LogError("AssetTreeWidget::OpenFileLocation: failed to open " + path);
    }
}

void AssetItemMenuHandler::OpenInExternalEditor()
{
    if (targets_.assets.isEmpty() || targets_.assets.size() < 1)
        return;
    
    IAsset *item = targets_.assets.first();
    if (item && !item->DiskSource().isEmpty())
        if (!QDesktopServices::openUrl(QUrl("file:///" + item->DiskSource(), QUrl::TolerantMode)))
            LogError("AssetTreeWidget::OpenInExternalEditor: failed to open " + item->DiskSource());
}

void AssetItemMenuHandler::Clone()
{
    if (targets_.assets.isEmpty())
        return;

    AssetPtr asset(targets_.assets.first());
    CloneAssetDialog *dialog = new CloneAssetDialog(asset, framework_->Asset());
    connect(dialog, SIGNAL(finished(int)), SLOT(CloneAssetDialogClosed(int)));
    dialog->show();
}

void AssetItemMenuHandler::CloneAssetDialogClosed(int result)
{
    CloneAssetDialog *dialog = qobject_cast<CloneAssetDialog *>(sender());
    if (!dialog)
        return;
    
    if (result != QDialog::Accepted)
        return;
    
    if (!dialog->Asset().expired())
        dialog->Asset().lock()->Clone(dialog->NewName());
}

void AssetItemMenuHandler::Import()
{
    OpenFileDialogNonModal(cAllTypesFileFilter, tr("Import"), "", 0, this, SLOT(OpenFileDialogClosed(int)), true);
}

void AssetItemMenuHandler::OpenFileDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;
    
    if (result != QDialog::Accepted)
        return;
    
    if (dialog->selectedFiles().isEmpty())
        return;

    Scene *scene = framework_->Scene()->MainCameraScene();
    assert(scene);
    AddContentWindow *addContent = new AddContentWindow(scene->shared_from_this(), framework_->Ui()->MainWindow());
    addContent->setWindowFlags(Qt::Tool);
    addContent->AddAssets(dialog->selectedFiles());
    addContent->show();
}

void AssetItemMenuHandler::Export()
{
    if (targets_.assets.isEmpty())
        return;
    
    if (targets_.assets.size() == 1)
    {
        QString ref = targets_.assets.first() ? targets_.assets.first()->Name() : "";
        QString assetName= AssetAPI::ExtractFilenameFromAssetRef(ref);
        SaveFileDialogNonModal("", tr("Save Asset As"), assetName, 0, this, SLOT(SaveAssetDialogClosed(int)));
    }
    else
    {
        DirectoryDialogNonModal(tr("Select Directory"), "", 0, this, SLOT(SaveAssetDialogClosed(int)));
    }
}

void AssetItemMenuHandler::SaveAssetDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());

    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty())
        return;
    
    QStringList files = dialog->selectedFiles();
    bool isDir = QDir(files[0]).exists();
    
    if ((targets_.assets.size() == 1 && isDir) || (targets_.assets.size() > 1 && !isDir))
    {
        // should not happen normally, so just log error. No prompt for user.
        //        LogError("Could not save asset: no such directory.");
        return;
    }
    
    foreach(IAsset *item, targets_.assets)
    if (item)
    {
        // if saving multiple assets, append filename to directory
        QString filename = files[0];
        if (isDir)
        {
            QString assetName = AssetAPI::ExtractFilenameFromAssetRef(item->Name());
            //while(QFile::exists(filename))
            //filename.append("_");
            filename += QDir::separator() + assetName;
        }
        
        QString param;
        if (item->Type().contains("texture", Qt::CaseInsensitive))
            param = filename.right(filename.size() - filename.lastIndexOf('.') - 1);
        
        item->SaveToFile(filename, param);
    }
}

void AssetItemMenuHandler::CopyAssetRef()
{
    if (targets_.assets.size() == 1 && targets_.assets.first())
        QApplication::clipboard()->setText(targets_.assets.first()->Name());
}

void AssetItemMenuHandler::ReloadFromSource()
{
    foreach(IAsset *item, targets_.assets)
        if (item)
        {
            QString assetRef = item->Name();
            // Make a 'forced request' of the existing asset. This will cause a full re-download of the asset
            // and the newly downloaded data will be deserialized to the existing asset object.
            framework_->Asset()->RequestAsset(assetRef, item->Type(), true);
        }
}

void AssetItemMenuHandler::ReloadFromCache()
{
    foreach(IAsset *item, targets_.assets)
    if (item)
        item->LoadFromCache();
}


void AssetItemMenuHandler::Unload()
{
    foreach(IAsset *item, targets_.assets)
    if (item)
        item->Unload();
}

void AssetItemMenuHandler::Forget()
{
    foreach(IAsset *item, targets_.assets)
    if (item)
        framework_->Asset()->ForgetAsset(item->shared_from_this(), false);
}

void AssetItemMenuHandler::DeleteFromSource()
{
    // AssetAPI::DeleteAssetFromStorage() signals will start deletion of tree widget asset items:
    // Gather the asset refs to a separate list beforehand in order to prevent crash.
    QStringList assetRefs, assetsToBeDeleted;
    foreach(IAsset *item, targets_.assets)
    if (item)
    {
        assetRefs << item->Name();
        assetsToBeDeleted << item->DiskSource();
    }
    
    QMessageBox msgBox(QMessageBox::Warning, tr("Delete From Source"),
                       tr("Are you sure want to delete the selected asset(s) permanently from the source?\n"),
                       QMessageBox::Ok | QMessageBox::Cancel, NULL);
    msgBox.setDetailedText(assetsToBeDeleted.join("\n"));
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
        foreach(QString ref, assetRefs)
        framework_->Asset()->DeleteAssetFromStorage(ref);
}

void AssetItemMenuHandler::DeleteFromCache()
{
    if (!framework_->Asset()->GetAssetCache())
    {
        LogError("Cannot delete asset from cache: Not running Tundra with an asset cache!");
        return;
    }
    foreach(IAsset *item, targets_.assets)
    if (item)
        framework_->Asset()->GetAssetCache()->DeleteAsset(item->Name());
}

void AssetItemMenuHandler::OpenFunctionDialog()
{
    if (!targets_.assets.isEmpty() && !targets_.storages.isEmpty())
        return;

    QObjectWeakPtrList objs;
    if (!targets_.assets.isEmpty())
        foreach(IAsset *item, targets_.assets)
            objs << dynamic_pointer_cast<QObject>(item->shared_from_this());
    else if (!targets_.storages.isEmpty())
        foreach(IAssetStorage *item, targets_.storages)
            objs << dynamic_pointer_cast<QObject>(item->shared_from_this());
    
    if (objs.size())
    {
        FunctionDialog *d = new FunctionDialog(objs, NULL);
        connect(d, SIGNAL(finished(int)), SLOT(FunctionDialogFinished(int)));
        d->show();
    }
}

void AssetItemMenuHandler::FunctionDialogFinished(int result)
{
    FunctionDialog *dialog = qobject_cast<FunctionDialog *>(sender());
    if (!dialog)
        return;
    
    if (result == QDialog::Rejected)
        return;
    
    // Get the list of parameters we will pass to the function we are invoking,
    // and update the latest values to them from the editor widgets the user inputted.
    QVariantList params;
    foreach(IArgumentType *arg, dialog->Arguments())
    {
        arg->UpdateValueFromEditor();
        params << arg->ToQVariant();
    }
    
    // Clear old return value from the dialog.
    dialog->SetReturnValueText("");
    
    foreach(const QObjectWeakPtr &o, dialog->Objects())
    if (!o.expired())
    {
        QObject *obj = o.lock().get();
        
        QString objName = obj->metaObject()->className();
        QString objNameWithId = objName;
        IAsset *asset = dynamic_cast<IAsset *>(obj);
        if (asset)
            objNameWithId.append('(' + asset->Name() + ')');
        
        QString errorMsg;
        QVariant ret;
        FunctionInvoker::Invoke(obj, dialog->Function(), params, &ret, &errorMsg);
        
        QString retValStr;
        ///\todo For some reason QVariant::toString() cannot convert QStringList to QString properly.
        /// Convert it manually here.
        if (ret.type() == QVariant::StringList)
            foreach(QString s, ret.toStringList())
            retValStr.append("\n" + s);
        else
            retValStr = ret.toString();
        
        if (errorMsg.isEmpty())
            dialog->AppendReturnValueText(objNameWithId + ' ' + retValStr);
        else
            dialog->AppendReturnValueText(objNameWithId + ' ' + errorMsg);
    }
}


void AssetItemMenuHandler::MakeDefaultStorage()
{
    if (targets_.storages.size() == 1)
    {
        framework_->Asset()->SetDefaultAssetStorage(targets_.storages.first()->shared_from_this());
        //QString storageName = selected.first()->data(0, Qt::UserRole).toString();
        //framework_->Asset()->SetDefaultAssetStorage(framework_->Asset()->GetAssetStorageByName(storageName));
    }
    
    AssetsWindow *parent = dynamic_cast<AssetsWindow*>(sender_);
    if (parent)
        parent->PopulateTreeWidget();
}

void AssetItemMenuHandler::RemoveStorage()
{
    foreach(IAssetStorage *item, targets_.storages)
    {
        //QString storageName = item->data(0, Qt::UserRole).toString();
        //framework_->Asset()->RemoveAssetStorage(storageName);
        if (item)
            framework_->Asset()->RemoveAssetStorage(item->Name());
    }
    
    AssetsWindow *parent = dynamic_cast<AssetsWindow*>(sender_);
        parent->PopulateTreeWidget();
}