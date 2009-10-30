// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file WebdavInventoryDataModel.h
 *  @brief Data model representing the webdav model
 */

#ifndef incl_InventoryModule_WebdavInventoryDataModel_h
#define incl_InventoryModule_WebdavInventoryDataModel_h

#include "AbstractInventoryDataModel.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"

#include "PythonQt.h"

namespace RexLogic
{
	class RexLogicModule;
}

namespace OpenSimProtocol
{
	class InventorySkeleton;
	class InventoryFolderSkeleton;
}

namespace Inventory
{
	class WebdavInventoryDataModel : public AbstractInventoryDataModel
	{

	Q_OBJECT

	public:
		/// Constructor
		WebdavInventoryDataModel(const QString &identityUrl, const QString &hostUrl);

		/// Deconstructor
		virtual ~WebdavInventoryDataModel();

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested name or null if the folder isn't found.
        AbstractInventoryItem *GetFirstChildFolderByName(const QString &searchName) const;

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested id or null if the folder isn't found.
        AbstractInventoryItem *GetChildFolderById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// @return First item by the requested id or null if the item isn't found.
        AbstractInventoryItem *GetChildAssetById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// Returns pointer to requested child item.
        /// @param searchId Search ID.
        /// @return Pointer to the requested item, or null if not found.
        AbstractInventoryItem *GetChildById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// Returns folder by requested id, or creates a new one if the folder doesnt exist,
        /// or returns null if the parent folder is invalid.
        /// @param id ID.
        /// @param parent Parent folder.
        /// @param notify_server Do we want to notify server.
        /// @return Pointer to the existing or just created folder.
        AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name = "New Folder", const bool &notify_server = true);

        /// AbstractInventoryDataModel override.
        /// Returns asset requested id, or creates a new one if the folder doesnt exist.
        /// @param inventory_id Inventory ID.
        /// @param asset_id Asset ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created asset.
        AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name = "New Asset");

        /// AbstractInventoryDataModel override.
        void FetchInventoryDescendents(AbstractInventoryItem *folder);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemMove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemCopy(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemRemove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemUpdate(AbstractInventoryItem *item);

        /// @return Inventory root folder.
        AbstractInventoryItem *GetRoot() const { return rootFolder_; }

	private:
		Q_DISABLE_COPY(WebdavInventoryDataModel);

		// Init PythonQt for usage
		bool InitPythonQt();

		/// Debug prints, can only run python in release ;(
		void PrintQVariantInPython(QVariant result);

		/// Fetch webdav inventory url with users identity
		/// @return bool true if succeeded otherwise false
		bool FetchWebdavUrlWithIdentity();

		/// Fetch initial root folder
		void FetchRootFolder();

		/// Fallback on error situations
		void ErrorOccurredCreateEmptyRootFolder();

		/// Related urls to store for fetching webdav url and accessing webdav
		QString identityUrl_;
		QString hostUrl_;
		QString webdavIdentityUrl_;
		QString webdavUrl_;

		/// The root folder.
        InventoryFolder *rootFolder_;

		// Pointer to PythonQts main module
		PythonQtObjectPtr pythonQtMainModule_;

	};
}

#endif // incl_InventoryModule_WebdavInventoryDataModel_h