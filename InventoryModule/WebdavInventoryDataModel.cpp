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
#include "InventorySkeleton.h"
#include "RexUUID.h"

namespace Inventory
{
	/// AbstractInventoryDataModel INTERFACE

	WebdavInventoryDataModel::WebdavInventoryDataModel(boost::weak_ptr<PythonScript::PythonScriptModule> pythonModule, const QString &identityUrl)
		: weakPythonModule_(pythonModule), identityUrl_(identityUrl), rootFolder_(0)
	{
		if ( FetchWebdavUrlWithIdentity() )
			FetchRootFolder();
	}

	WebdavInventoryDataModel::~WebdavInventoryDataModel()
	{

	}

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

	bool WebdavInventoryDataModel::FetchWebdavUrlWithIdentity()
	{
		pythonModule_ = weakPythonModule_.lock();
		if (pythonModule_.get())
		{
			//boost::shared_ptr<PythonScript::PythonEngine> test = pythonModule_->engineAccess;
			//PythonScript::PythonEngine *pythonService = (PythonScript::PythonEngine *)pythonModule_->engineAccess;
			//pythonService->RunString("print hello world");
			return true;
		}
		else
			return false;
	}

	void WebdavInventoryDataModel::FetchRootFolder()
	{

	}
}