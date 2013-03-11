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
    AddContentWindow *addContent = new AddContentWindow(framework->Scene()->MainCameraScene()->shared_from_this(), framework->Ui()->MainWindow());
    addContent->setWindowFlags(Qt::Tool);
    addContent->AddAssets(files);
    addContent->show();
}

void AssetTreeWidget::OpenFileLocation()
{
    QList<AssetItem *> selection = SelectedItems().assets;
    if (selection.isEmpty() || selection.size() < 1)
        return;

    AssetItem *item = selection.first();
    if (item->Asset() && !item->Asset()->DiskSource().isEmpty())
    {
        bool success = false;
#ifdef _WINDOWS
        // Custom code for Windows, as we want to use explorer.exe with the /select switch.
        // Craft command line string, use the full filename, not directory.
        QString path = QDir::toNativeSeparators(item->Asset()->DiskSource());
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
        QString path = QDir::toNativeSeparators(QFileInfo(item->Asset()->DiskSource()).dir().path());
        success = QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
#endif
        if (!success)
            LogError("AssetTreeWidget::OpenFileLocation: failed to open " + path);
    }
}

void AssetTreeWidget::OpenInExternalEditor()
{
    QList<AssetItem *> selection = SelectedItems().assets;
    if (selection.isEmpty() || selection.size() < 1)
        return;

    AssetItem *item = selection.first();
    if (item->Asset() && !item->Asset()->DiskSource().isEmpty())
        if (!QDesktopServices::openUrl(QUrl("file:///" + item->Asset()->DiskSource(), QUrl::TolerantMode)))
            LogError("AssetTreeWidget::OpenInExternalEditor: failed to open " + item->Asset()->DiskSource());
}

void AssetTreeWidget::OpenFunctionDialog()
{
    AssetTreeWidgetSelection sel = SelectedItems();
    if (sel.HasAssets() && sel.HasStorages())
        return;

    QObjectWeakPtrList objs;
    if (sel.HasAssets())
        foreach(AssetItem *item, SelectedItems().assets)
            objs << dynamic_pointer_cast<QObject>(item->Asset());
    else if (sel.HasStorages())
        foreach(AssetStorageItem *item, SelectedItems().storages)
            objs << dynamic_pointer_cast<QObject>(item->Storage());

    if (objs.size())
    {
        FunctionDialog *d = new FunctionDialog(objs, this);
        connect(d, SIGNAL(finished(int)), SLOT(FunctionDialogFinished(int)));
        d->show();
    }
}

void AssetTreeWidget::FunctionDialogFinished(int result)
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

