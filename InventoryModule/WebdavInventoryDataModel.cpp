// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file WebdavInventoryDataModel.cpp
 *  @brief Data model representing the webdav model
 */

#include "StableHeaders.h"
#include "WebdavInventoryDataModel.h"
#include "AbstractInventoryItem.h"
#include "InventoryModule.h"
#include "RexLogicModule.h"
#include "Inventory/InventorySkeleton.h"
#include "RexUUID.h"

#include <QDir>

namespace Inventory
{

WebdavInventoryDataModel::WebdavInventoryDataModel(const QString &identityUrl, const QString &hostUrl) :
    identityUrl_(identityUrl), hostUrl_(hostUrl), rootFolder_(0)
{
    if (InitPythonQt())
    {
        if (FetchWebdavUrlWithIdentity())
            FetchRootFolder();
        else
            ErrorOccurredCreateEmptyRootFolder();
    }
    else
        ErrorOccurredCreateEmptyRootFolder();
}

WebdavInventoryDataModel::~WebdavInventoryDataModel()
{
    if (rootFolder_)
        delete rootFolder_;
}

/// AbstractInventoryDataModel INTERFACE
void WebdavInventoryDataModel::SetWorldStream(const ProtocolUtilities::WorldStreamPtr world_stream)
{
}

AbstractInventoryItem *WebdavInventoryDataModel::GetFirstChildFolderByName(const QString &searchName) const
{
    InventoryModule::LogInfo("Webdav | You are in GetFirstChildFolderByName() that is not implemented yet");
    return 0;
}

AbstractInventoryItem *WebdavInventoryDataModel::GetChildFolderById(const QString &searchId) const
{
    if ( RexUUID::IsValid(searchId.toStdString()) )
        return 0;
    else
        return rootFolder_->GetChildFolderById(searchId);
}

AbstractInventoryItem *WebdavInventoryDataModel::GetChildAssetById(const QString &searchId) const
{
    InventoryModule::LogInfo("Webdav | You are in GetChildAssetById() that is not implemented yet");
    return 0;
}

AbstractInventoryItem *WebdavInventoryDataModel::GetChildById(const QString &searchId) const
{
    return rootFolder_->GetChildById(searchId);
}

AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
        const QString &name, const bool &notify_server)
{
    if (parentFolder.GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        InventoryFolder *parent = dynamic_cast<InventoryFolder *>(&parentFolder);
        if (parent)
        {
            if ( RexUUID::IsValid(id.toStdString()) )
            {
                // This is a new folder if the id is RexUUID 
                // RexUUID generated in data model, cant do nothing about this...
                QString parentPath = parent->GetID();
                QString newFolderName = name;
                QStringList result = webdavclient_.call("createDirectory", QVariantList() << parentPath << newFolderName).toStringList();
                if (result.count() >= 1)
                    if (result[0] == "True")
                    {
                        //ItemSelectedFetchContent(&parentFolder);
                        FetchInventoryDescendents(&parentFolder);
                        InventoryModule::LogInfo(QString("Webdav | Created folder named %1 to path %2\n").arg(newFolderName, parentPath).toStdString());
                    }
                    else
                        InventoryModule::LogInfo(QString("Webdav | Could not create folder named %1 to path %2\n").arg(newFolderName, parentPath).toStdString());
            }
            else
            {
                // If its not RexUUID or and existing item in this folder,
                // then its a move command. Lets do that to the webdav server then...
                InventoryFolder *existingItem = parent->GetChildFolderById(id);
                if (existingItem == 0)
                {
                    InventoryFolder *currentFolder = rootFolder_->GetChildFolderById(name);
                    if (currentFolder)
                    {
                        InventoryFolder *currentFolder = rootFolder_->GetChildFolderById(name);
                        if (currentFolder)
                        {
                            QString currentPath = ValidateFolderPath(currentFolder->GetParent()->GetID());
                            QString newPath = ValidateFolderPath(parent->GetID());
                            QString folderName = name;
                            QString deepCopy = "False";
                            if (currentFolder->GetChildList().count() > 0)
                                deepCopy = "True";
                            QStringList result = webdavclient_.call("moveResource", QVariantList() << currentPath << newPath << folderName).toStringList();
                            if (result.count() >= 1)
                                if (result[0] == "True")
                                {
                                    //ItemSelectedFetchContent(&parentFolder);
                                    FetchInventoryDescendents(&parentFolder);
                                    InventoryModule::LogInfo(QString("Webdav | Moved folder %1 from %2 to %3\nNote: This fucntionality is experimental,").append( 
                                                                     "dont assume it went succesfull\n").arg(folderName, currentPath, newPath).toStdString());
                                }
                                else
                                    InventoryModule::LogInfo(QString("Webdav | Failed to move folder %1 from %2 to %3\n").arg(folderName, currentPath, newPath).toStdString());
                        }
                    }
                }
                else
                    return existingItem;
            }
        }
    }
    // Return 0 to data model, we just updated 
    // folder content from webdav, no need to return the real item
    return 0;
}

AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
        AbstractInventoryItem &parentFolder, const QString &name)
{
    InventoryModule::LogInfo("Webdav | You are in GetOrCreateNewAsset() that is not implemented yet");
    return 0;
}

void WebdavInventoryDataModel::FetchInventoryDescendents(AbstractInventoryItem *item)
{
    //InventoryModule::LogInfo("Webdav | You are in FetchInventoryDescendents() that is not implemented yet");

    InventoryFolder *selected = dynamic_cast<InventoryFolder *>(item);
    if (!selected)
        return;

    // Delete children
    selected->GetChildList().clear();
    //QListIterator<AbstractInventoryItem *> it(selected->GetChildList());
    //while(it.hasNext())
    //{
    //    AbstractInventoryItem *item = it.next();
    //    if (item)
    //        delete item;
    //}

    QString itemPath = selected->GetID();
    QStringList children = webdavclient_.call("listResources", QVariantList() << itemPath).toStringList();
    if ( children.count() >=1 )
    {
        // Process child list to map
        QMap<QString, QString> childMap;
        for (int index=0; index<=children.count(); index++)
        {
            childMap[children.value(index)] = children.value(index+1);
            index++;
        }

        AbstractInventoryItem *newItem = 0;
        QString path, name, type;
        for (QMap<QString, QString>::iterator iter = childMap.begin(); iter!=childMap.end(); ++iter)
        {
            path = iter.key();
            name = path.midRef(path.lastIndexOf("/")+1).toString();
            type = iter.value();
            if (name != "")
            {
                if (type == "resource")
                    newItem = new InventoryAsset(path, "", name, selected);
                else
                    newItem = new InventoryFolder(path, name, selected, true);
                selected->AddChild(newItem);
            }
        }

        InventoryModule::LogInfo(QString("Webdav | Fetched %1 children to path /%2").arg(QString::number(childMap.count()), itemPath).toStdString());
    }
}

void WebdavInventoryDataModel::NotifyServerAboutItemMove(AbstractInventoryItem *item)
{
    InventoryModule::LogInfo("Webdav | You are in NotifyServerAboutItemMove() that is not implemented yet");
}

void WebdavInventoryDataModel::NotifyServerAboutItemCopy(AbstractInventoryItem *item)
{
    InventoryModule::LogInfo("Webdav | You are in NotifyServerAboutItemCopy() that is not implemented yet");
}

void WebdavInventoryDataModel::NotifyServerAboutItemRemove(AbstractInventoryItem *item)
{
    QString parentPath = ValidateFolderPath(item->GetParent()->GetID());
    QString itemName = item->GetName();
    QString methodName;
    QStringList result;

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        methodName = "deleteDirectory";
    else if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
        methodName = "deleteResource";
    else
        return;

    result = webdavclient_.call(methodName, QVariantList() << parentPath << itemName).toStringList();
    if (result.count() >= 1)
        if (result[0] == "True")
        {
            InventoryModule::LogInfo(QString("Webdav | Removed item from %1\n").arg(item->GetID()).toStdString());
            //ItemSelectedFetchContent(item->GetParent());
            FetchInventoryDescendents(item->GetParent());
        }
        else
            InventoryModule::LogInfo(QString("Webdav | Could not remove item from %1\n").arg(item->GetID()).toStdString());
}

void WebdavInventoryDataModel::NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name)
{
    QString parentPath = ValidateFolderPath(item->GetParent()->GetID());
    QString newName = item->GetName();
    QString oldName = old_name;

    QStringList result = webdavclient_.call("renameResource", QVariantList() << parentPath << newName << oldName).toStringList();
    if (result.count() >= 1)
        if (result[0] == "True")
        {
            //ItemSelectedFetchContent(item->GetParent());
            FetchInventoryDescendents(item->GetParent());
            InventoryModule::LogInfo(QString("Webdav | Renamed folder from %0 to %1 in path %3\n").arg(oldName, newName, parentPath).toStdString());
        }
        else
            InventoryModule::LogInfo(QString("Webdav | Could not rename folder from %0 to %1 in path %3\n").arg(oldName, newName, parentPath).toStdString());
}

/// PUBLIC SLOTS
/*
void WebdavInventoryDataModel::ItemSelectedFetchContent(AbstractInventoryItem *item)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        InventoryFolder *selected = dynamic_cast<InventoryFolder *>(item);
        if (selected)
        {
            // Delete children
            selected->GetChildList().clear();
            //QListIterator<AbstractInventoryItem *> it(selected->GetChildList());
            //while(it.hasNext())
            //{
            //    AbstractInventoryItem *item = it.next();
            //    if (item)
            //        delete item;
            //}

            QString itemPath = selected->GetID();
            QStringList children = webdavclient_.call("listResources", QVariantList() << itemPath).toStringList();
            if ( children.count() >=1 )
            {
                // Process child list to map
                QMap<QString, QString> childMap;
                for (int index=0; index<=children.count(); index++)
                {
                    childMap[children.value(index)] = children.value(index+1);
                    index++;
                }

                AbstractInventoryItem *newItem;
                QString path, name, type;
                for (QMap<QString, QString>::iterator iter = childMap.begin(); iter!=childMap.end(); ++iter)
                {
                    path = iter.key();
                    name = path.midRef(path.lastIndexOf("/")+1).toString();
                    type = iter.value();
                    if (name != "")
                    {
                        if (type == "resource")
                            newItem = new InventoryAsset(path, "", name, selected);
                        else
                            newItem = new InventoryFolder(path, name, selected, true);
                        selected->AddChild(newItem);
                    }
                }
                InventoryModule::LogInfo(QString("Webdav | Fetched %1 children to path /%2").arg(QString::number(childMap.count()), itemPath).toStdString());
            }
        }
    }
}
*/

void WebdavInventoryDataModel::UploadFile(const QString &file_path, AbstractInventoryItem *parent_folder)
{
    if (parent_folder->GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(parent_folder);
        if (parentFolder)
        {
            QString filePath = file_path;
            QString filename = filePath.split("/").last();
            QString parentPath = ValidateFolderPath(parentFolder->GetID());
            QStringList result = webdavclient_.call("uploadFile", QVariantList() << filePath << parentPath << filename).toStringList();
            if (result.count() >= 1)
                if (result[0] == "True")
                {
                    //ItemSelectedFetchContent(parent_folder);
                    FetchInventoryDescendents(parent_folder);
                    InventoryModule::LogInfo(QString("Webdav | Upload of file %1 to path %2%3 succeeded\n").arg(filePath, parentPath, filename).toStdString());
                }
                else
                    InventoryModule::LogInfo(QString("Webdav | Upload of file %1 to path %2%3 failed\n").arg(filePath, parentPath, filename).toStdString());
        }
    }
}

void WebdavInventoryDataModel::DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item)
{
    if (selected_item->GetItemType() == AbstractInventoryItem::Type_Asset)
    {
        InventoryAsset *item = dynamic_cast<InventoryAsset *>(selected_item);
        if (item)
        {
            QString storePath = store_folder;
            QString parentPath = ValidateFolderPath(item->GetParent()->GetID());
            QString filename = item->GetName();
            QStringList result = webdavclient_.call("downloadFile", QVariantList() << storePath << parentPath << filename).toStringList();
            if (result.count() >= 1)
                if (result[0] == "True")
                    InventoryModule::LogInfo(QString("Webdav | Downloaded file %1%2 to path %3\n").arg(parentPath, filename, storePath).toStdString());
                else
                    InventoryModule::LogInfo(QString("Webdav | Downloaded of file %1%2 to path %3 failed\n").arg(parentPath, filename, storePath).toStdString());
        }
    }
}

/// WEBDAV RELATED (private)

bool WebdavInventoryDataModel::InitPythonQt()
{
    QString myPath = QString("%1/pymodules/webdavinventory").arg(QDir::currentPath());

    pythonQtMainModule_ = PythonQt::self()->getMainModule();
    pythonQtMainModule_.evalScript(QString("print '[PythonQt] Webdav inventory fetched me...'"));
    pythonQtMainModule_.evalScript(QString("import sys\n"));
    pythonQtMainModule_.evalScript(QString("sys.path.append('%1')\n").arg(myPath));
    pythonQtMainModule_.evalScript(QString("print '[PythonQt] Added %1 to sys.path for imports'").arg(myPath));

    Q_ASSERT(!pythonQtMainModule_.isNull());
    return true;
}

bool WebdavInventoryDataModel::FetchWebdavUrlWithIdentity()
{
    pythonQtMainModule_.evalScript("import connection\n");
    PythonQtObjectPtr httpclient = pythonQtMainModule_.evalScript("connection.HTTPClient()\n", Py_eval_input);

    // Some url verification, remove http:// and everything after the port
    int index = hostUrl_.indexOf("http://");
    if (index != -1)
        hostUrl_ = hostUrl_.midRef(index+7).toString();
    index = hostUrl_.indexOf("/");
    if (index != -1)
        hostUrl_ = hostUrl_.midRef(0, index).toString();

    // Set up HTTP connection to Taiga WorldServer
    httpclient.call("setupConnection", QVariantList() << hostUrl_ << "openid" << identityUrl_);
    // Get needed webdav access urls from Taiga WorldServer
    QStringList resultList = httpclient.call("requestIdentityAndWebDavURL").toStringList();
    // Store results
    if (resultList.count() >= 1)
    {
        webdavIdentityUrl_ = resultList.value(0);
        webdavUrl_ = resultList.value(1);
        return true;
    }
    else
        return false;
}

void WebdavInventoryDataModel::FetchRootFolder()
{
    webdavclient_ = pythonQtMainModule_.evalScript("connection.WebDavClient()\n", Py_eval_input);
    if (webdavclient_)
    {
        // Set urls
        webdavclient_.call("setHostAndUser", QVariantList() << webdavIdentityUrl_ << webdavUrl_);
        // Connect to webdav
        webdavclient_.call("setupConnection");
        // Fetch root resources
        QStringList rootResources = webdavclient_.call("listResources").toStringList();
        if ( rootResources.count() >=1 )
        {
            QMap<QString, QString> folders;
            InventoryFolder *parentFolder;
            for (int index=0; index<=rootResources.count(); index++)
            {
                folders[rootResources.value(index)] = rootResources.value(index+1);
                index++;
            }

            if (!rootFolder_)
            {
                rootFolder_ = new InventoryFolder("root", "Webdav Inventory", false, 0);
                parentFolder = new InventoryFolder("", QString("My Inventory"), false, rootFolder_);
                rootFolder_->AddChild(parentFolder);
                rootFolder_->SetDirty(true);
            }

            AbstractInventoryItem *newItem = 0;
            QString path;
            QString name;
            QString type;

            for (QMap<QString, QString>::iterator iter = folders.begin(); iter!=folders.end(); ++iter)
            {
                path = iter.key();
                name = path.midRef(0, path.lastIndexOf("/")).toString();
                name = path.midRef(path.lastIndexOf("/")).toString();
                type = iter.value();
                if (name != "")
                {
                    if (type == "resource")
                        newItem = new InventoryAsset(path, "", name, parentFolder);
                    else
                        newItem = new InventoryFolder(path, name, parentFolder, true);
                    parentFolder->AddChild(newItem);
                }
            }
        }
        else
        {
            ErrorOccurredCreateEmptyRootFolder();
        }
    }
    else
        ErrorOccurredCreateEmptyRootFolder();
}

void WebdavInventoryDataModel::ErrorOccurredCreateEmptyRootFolder()
{
    if (!rootFolder_)
        rootFolder_ = new InventoryFolder("root", "Error while fetching Webdav Inventory", false, 0);
    InventoryFolder *parentFolder = new InventoryFolder("/", QString("My Inventory"), false, rootFolder_);
    rootFolder_->AddChild(parentFolder);
    rootFolder_->SetDirty(true);
}

QString WebdavInventoryDataModel::ValidateFolderPath(const QString &path)
{
    QString validPath = path;
    if (!validPath.endsWith("/") && !validPath.isEmpty())
        validPath.append("/");
    return validPath;
}

}
