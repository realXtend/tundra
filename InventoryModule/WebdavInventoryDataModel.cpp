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
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildFolderById(const QString &searchId) const
	{
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildAssetById(const QString &searchId) const
	{
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetChildById(const QString &searchId) const
	{
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name, const bool &notify_server)
	{
		return new InventoryAsset("", "");
	}

	AbstractInventoryItem *WebdavInventoryDataModel::GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name)
	{
		return new InventoryAsset("", "");
	}

	void WebdavInventoryDataModel::FetchInventoryDescendents(AbstractInventoryItem *folder)
	{

	}

	void WebdavInventoryDataModel::NotifyServerAboutItemMove(AbstractInventoryItem *item)
	{

	}
	
	void WebdavInventoryDataModel::NotifyServerAboutItemCopy(AbstractInventoryItem *item)
	{

	}

	void WebdavInventoryDataModel::NotifyServerAboutItemRemove(AbstractInventoryItem *item)
	{

	}

	void WebdavInventoryDataModel::NotifyServerAboutItemUpdate(AbstractInventoryItem *item)
	{

	}

	/// WEBDAV RELATED (private)

	bool WebdavInventoryDataModel::InitPythonQt()
	{
		QString myPath = QString("%1/pymodules/webdavinventory").arg(QDir::currentPath());

		pythonQtMainModule_ = PythonQt::self()->getMainModule();
		pythonQtMainModule_.evalScript("print '[PythonQt] Webdav inventory fetched me...'");
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
		PythonQtObjectPtr webdavclient = pythonQtMainModule_.evalScript("connection.WebDavClient()\n", Py_eval_input);
		if (webdavclient)
		{
			// Set urls
			webdavclient.call("setHostAndUser", QVariantList() << webdavIdentityUrl_ << webdavUrl_);
			// Connect to webdav
			webdavclient.call("setupConnection");
			// Fetch root resources
			QStringList rootResources = webdavclient.call("listResources").toStringList();
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