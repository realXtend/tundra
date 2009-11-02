// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file WebdavInventoryDataModel.cpp
 *  @brief Data model representing the webdav model
 */

#include "StableHeaders.h"
#include "AbstractInventoryItem.h"
#include "InventoryModule.h"
#include "RexLogicModule.h"
#include "InventorySkeleton.h"
#include "RexUUID.h"

#include "WebdavInventoryDataModel.h"
#include <QDir>

namespace Inventory
{

	WebdavInventoryDataModel::WebdavInventoryDataModel(const QString &identityUrl, const QString &hostUrl)
		: identityUrl_(identityUrl), hostUrl_(hostUrl), rootFolder_(0)
	{
		if ( InitPythonQt() )
		{
			if ( FetchWebdavUrlWithIdentity() )
				FetchRootFolder();
			else
				ErrorOccurredCreateEmptyRootFolder();
		}
		else 
			ErrorOccurredCreateEmptyRootFolder();
	}

	WebdavInventoryDataModel::~WebdavInventoryDataModel()
	{

	}

	/// AbstractInventoryDataModel INTERFACE

	AbstractInventoryItem *WebdavInventoryDataModel::GetFirstChildFolderByName(const QString &searchName) const
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetFirstChildFolderByName");
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildFolderById(const QString &searchId) const
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetChildFolderById");
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildAssetById(const QString &searchId) const
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetChildAssetById");
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildById(const QString &searchId) const
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetChildById");
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name, const bool &notify_server)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetOrCreateNewFolder");
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in GetOrCreateNewAsset");
		return new InventoryAsset("", "");
	}

	void WebdavInventoryDataModel::FetchInventoryDescendents(AbstractInventoryItem *folder)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in FetchInventoryDescendents");
	}

	void WebdavInventoryDataModel::NotifyServerAboutItemMove(AbstractInventoryItem *item)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in NotifyServerAboutItemMove");
	}
	
	void WebdavInventoryDataModel::NotifyServerAboutItemCopy(AbstractInventoryItem *item)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in NotifyServerAboutItemCopy");
	}

	void WebdavInventoryDataModel::NotifyServerAboutItemRemove(AbstractInventoryItem *item)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in NotifyServerAboutItemRemove");
	}

	void WebdavInventoryDataModel::NotifyServerAboutItemUpdate(AbstractInventoryItem *item)
	{
		InventoryModule::LogInfo("[Webdav Inventory] You are in NotifyServerAboutItemUpdate");
	}

	/// PUBLIC SLOTS

	void WebdavInventoryDataModel::ItemSelectedFetchContent(AbstractInventoryItem *item)
	{
		if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
		{
			InventoryFolder *selected = dynamic_cast<InventoryFolder *>(item);
			if (item)
			{
				// Delete children
				InventoryModule::LogInfo("Webdav | Deleting all childer from folder\n");
				//QListIterator<AbstractInventoryItem *> it(selected->GetChildList());
				//while(it.hasNext())
				//{
				//	AbstractInventoryItem *item = it.next();
				//	if (item)
				//		delete item;
				//}
				selected->GetChildList().clear();

				// Get path
				QString path = selected->GetID();
				if (path == "/")
					path = "";
				QString debugmsg = QString("Webdav | Fetching folder content for path %1 \n").arg(path);
				InventoryModule::LogInfo(debugmsg.toStdString());
				
				// Get child list
				QStringList children = webdavclient_.call("listResources", QVariantList() << path).toStringList();
				
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
					QString path;
					QString name;
					QString type;

					// Add items as children
					for (QMap<QString, QString>::iterator iter = childMap.begin(); iter!=childMap.end(); ++iter)
					{
						path = iter.key();
						name = path.midRef(0, path.lastIndexOf("/")).toString();
						name = path.midRef(path.lastIndexOf("/")).toString();
						type = iter.value();
						if (name != "")
						{
							if (type == "resource")
								newItem = new InventoryAsset(path, "", name, selected);
							else
								newItem = new InventoryFolder(path, name, selected, true);
							QString debugmsg = QString("Webdav | Adding child with path(%1) name(%2) type(%3)\n").arg(path, name, type);
							InventoryModule::LogInfo(debugmsg.toStdString());
							selected->AddChild(newItem);
						}
					}
				}

			}
		}
		else if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
		{
			InventoryAsset *selected = dynamic_cast<InventoryAsset *>(item);
			if (item)
			{
				// Do nothing, cant fetch item content...
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

	void WebdavInventoryDataModel::PrintQVariantInPython(QVariant result)
	{
		QStringList list = result.toStringList();
		for (int i=0; i<=list.count(); i++)
		{
			QString s("print '[QVariant loop] ");
			s.append(list.value(i));
			s.append("'");
			pythonQtMainModule_.evalScript(s);
		}
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
		if ( resultList.count() >= 1 )
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
					parentFolder = new InventoryFolder("/", QString("My Inventory"), false, rootFolder_);
					rootFolder_->AddChild(parentFolder);
					rootFolder_->SetDirty(true);
				}

				AbstractInventoryItem *newItem;
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
}