/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryModule.cpp
 *  @brief  Inventory module. Inventory module is the owner of the inventory data model.
 *          Implement data model -specific event handling etc. here, not in InventoryWindow
 *          or InventoryItemModel classes.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InventoryModule.h"
#include "InventoryWindow.h"
//#include "UploadProgressWindow.h"
#include "OpenSimInventoryDataModel.h"
#include "WebdavInventoryDataModel.h"
#include "InventoryAsset.h"
#include "ItemPropertiesWindow.h"
#include "InventoryService.h"

#include "Framework.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "WorldStream.h"
#include "ConsoleCommandServiceInterface.h"
#include "NetworkEvents.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetInMessage.h"
#include "Inventory/InventoryEvents.h"
#include "AssetServiceInterface.h"
#include "AssetEvents.h"
#include "ResourceInterface.h"
#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/NotificationManager.h"

#include <QStringList>
#include <QVector>
#include "MemoryLeakCheck.h"

namespace Inventory
{

InventoryModule::InventoryModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_Inventory),
    inventoryEventCategory_(0),
    networkStateEventCategory_(0),
    networkInEventCategory_(0),
    assetEventCategory_(0),
    resourceEventCategory_(0),
    frameworkEventCategory_(0),
    inventoryWindow_(0),
//    uploadProgressWindow_(0),
    inventoryType_(IDMT_Unknown),
    service_(0),
    webdav_inv_creds_recieved_(false) // school
{
}

InventoryModule::~InventoryModule()
{
}

void InventoryModule::Initialize()
{
    // Register event category and events.
    eventManager_ = framework_->GetEventManager();
    inventoryEventCategory_ = eventManager_->RegisterEventCategory("Inventory");
    eventManager_->RegisterEvent(inventoryEventCategory_, Events::EVENT_INVENTORY_DESCENDENT, "InventoryDescendent");
    eventManager_->RegisterEvent(inventoryEventCategory_, Events::EVENT_INVENTORY_UPLOAD_FILE, "InventoryUpload");
    eventManager_->RegisterEvent(inventoryEventCategory_, Events::EVENT_INVENTORY_UPLOAD_BUFFER, "InventoryUploadBuffer");
    eventManager_->RegisterEvent(inventoryEventCategory_, Events::EVENT_INVENTORY_ITEM_OPEN, "InventoryItemOpen");
    eventManager_->RegisterEvent(inventoryEventCategory_, Events::EVENT_INVENTORY_ITEM_DOWNLOADED, "InventoryItemDownloaded");

    // Register console commands.
    RegisterConsoleCommand(Console::CreateCommand("Upload",
        "Upload an asset. Usage: Upload(AssetType, Name, Description)",
        Console::Bind(this, &Inventory::InventoryModule::UploadAsset)));

    RegisterConsoleCommand(Console::CreateCommand("MultiUpload", "Upload multiple assets.",
        Console::Bind(this, &Inventory::InventoryModule::UploadMultipleAssets)));

#ifdef _DEBUG
    RegisterConsoleCommand(Console::CreateCommand("InvTest", "Inventory service debug/testing command.",
        Console::Bind(this, &Inventory::InventoryModule::InventoryServiceTest)));
#endif
}

void InventoryModule::PostInitialize()
{
    frameworkEventCategory_ = eventManager_->QueryEventCategory("Framework");
    assetEventCategory_ = eventManager_->QueryEventCategory("Asset");
    resourceEventCategory_ = eventManager_->QueryEventCategory("Resource");
}

void InventoryModule::Uninitialize()
{
    SAFE_DELETE(inventoryWindow_);
//    SAFE_DELETE(uploadProgressWindow_);
    SAFE_DELETE(service_);
    DeleteAllItemPropertiesWindows();

    eventManager_.reset();
    currentWorldStream_.reset();
    inventory_.reset();
}

void InventoryModule::SubscribeToNetworkEvents()
{
    networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
    networkInEventCategory_ = eventManager_->QueryEventCategory("NetworkIn");
}

void InventoryModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool InventoryModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    // NetworkState
    if (category_id == networkStateEventCategory_)
    {

        // Connected to server. Initialize inventory_ tree model.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            ProtocolUtilities::AuthenticationEventData *auth = checked_static_cast<ProtocolUtilities::AuthenticationEventData *>(data);
            assert(auth);
            if (!auth)
                return false;

            // Create inventory and upload progress windows
            UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>("UiServices").lock();
            if (ui_module.get())
            {
                SAFE_DELETE(inventoryWindow_);
                inventoryWindow_ = new InventoryWindow(this);
                connect(inventoryWindow_, SIGNAL(OpenItemProperties(const QString &)), this, SLOT(OpenItemPropertiesWindow(const QString &)));

                /*proxyWidget_ = */ui_module->GetInworldSceneController()->AddWidgetToScene(inventoryWindow_,
                    UiServices::UiWidgetProperties(TR("Inventory::InventoryWindow", "Inventory"), UiServices::ModuleWidget));

                connect(inventoryWindow_, SIGNAL(Notification(CoreUi::NotificationBaseWidget *)), ui_module->GetNotificationManager(),
                    SLOT(ShowNotification(CoreUi::NotificationBaseWidget *)));
/*
            if (uploadProgressWindow_)
                SAFE_DELETE(uploadProgressWindow_);
            uploadProgressWindow_ = new UploadProgressWindow(this);
*/
            }

            switch(auth->type)
            {
            case ProtocolUtilities::AT_Taiga:
            {
                // Check if python module is loaded and has taken care of PythonQt::init()
                if (!framework_->GetModuleManager()->HasModule(Foundation::Module::MT_PythonScript))
                {
                    LogError("PythonScriptModule not present. WebDAV based inventory cannot be fetched and avatar export is disabled!");
                    inventoryType_ = IDMT_Unknown;
                }
                else
                {
                    if (!auth->webdav_host.empty())
                    {
                        webdav_host_ = QString(auth->webdav_host.c_str());

                        // SCHOOL

                        /*
                        bool credentials_ok = true;
                        if (auth->webdav_identity.empty())
                        {
                            QString inv_identity = QInputDialog::getText(0, "Inventory credentials", "<b>Please give your LiveID display name</b>\nSet your liveid email if you dont have a displayname.", QLineEdit::Normal, "", &credentials_ok);
                            if (credentials_ok && !inv_identity.isEmpty())
                            {
                                if (inv_identity.contains("@") && inv_identity.count(" ") == 0)
                                    auth->webdav_identity = inv_identity.toStdString() + " " + inv_identity.toStdString();
                                else
                                {
                                    QString live_id_email = QInputDialog::getText(0, "Inventory credentials", "Please provide your liveID email", QLineEdit::Normal, "", &credentials_ok);
                                    auth->webdav_identity = inv_identity.toStdString() + " " + live_id_email.toStdString();
                                }
                            }
                            else
                            {
                                credentials_ok = false;
                                LogError("Cannot get webdav inventory without identity.");
                            }
                        }

                        if (credentials_ok && auth->webdav_password.empty())
                        {
                            QString inv_password = QInputDialog::getText(0, "Inventory password", "Please provide your inventory password", QLineEdit::Password, "", &credentials_ok);
                            if (credentials_ok && !inv_password.isEmpty())
                                auth->webdav_password = inv_password.toStdString();
                            else
                            {
                                credentials_ok = false;
                                LogError("Cannot get webdav inventory without password.");
                            }
                        }

                        if (credentials_ok)
                        {
                            // Create WebDAV inventory model.
                            inventoryType_ = IDMT_WebDav;
                            inventory_ = InventoryPtr(new WebDavInventoryDataModel(auth->webdav_identity.c_str(), auth->webdav_host.c_str(), auth->webdav_password.c_str()));
                            inventoryWindow_->InitInventoryTreeModel(inventory_);
                            SAFE_DELETE(service_);
                            service_ = new InventoryService(inventory_.get());
                        }
                        else
                            LogError("Could not get valid credentials to access webdav inventory! Disabling inventory.");
                        */
                    }
                    else
                    {
                        LogError("Login response did not contain a valid webdav inventory url! Disabling inventory.");
                    }
                }
                break;
            }
            case ProtocolUtilities::AT_OpenSim:
            case ProtocolUtilities::AT_RealXtend:
            {
                // Create OpenSim inventory model.
                inventory_ = InventoryPtr(new OpenSimInventoryDataModel(this, auth->inventorySkeleton.get()));

                // Set world stream used for sending udp packets.
                static_cast<OpenSimInventoryDataModel *>(inventory_.get())->SetWorldStream(currentWorldStream_);

                inventoryType_ = IDMT_OpenSim;
                inventoryWindow_->InitInventoryTreeModel(inventory_);
                SAFE_DELETE(service_);
                service_ = new InventoryService(inventory_.get());
                break;
            }
            case ProtocolUtilities::AT_Unknown:
            default:
                inventoryType_ = IDMT_Unknown;
                break;
            }

//            ConnectSignals();

            return false;
        }

        // Disconnected from server. Close/delete inventory, upload progress, and all item properties windows.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            webdav_inv_creds_recieved_ = false;
            UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>("UiServices").lock();
            if (ui_module)
            {
                if (inventoryWindow_)
                {
                    ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(inventoryWindow_);
                    SAFE_DELETE_LATER(inventoryWindow_);
                }
/*
                if (uploadProgressWindow_)
                {
                    ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(uploadProgressWindow_);
                    uploadProgressWindow_->deleteLater();
                    uploadProgressWindow_ = 0;
                }
*/
                DeleteAllItemPropertiesWindows();
            }

            SAFE_DELETE(service_);
        }

        return false;
    }

    // NetworkIn
    if (category_id == networkInEventCategory_)
    {
        switch(event_id)
        {
        case RexNetMsgInventoryDescendents:
            HandleInventoryDescendents(data);
            return false;
        case RexNetMsgUpdateCreateInventoryItem:
            HandleUpdateCreateInventoryItem(data);
            return false;
        case RexNetMsgUUIDNameReply:
            HandleUuidNameReply(data);
            return false;
        case RexNetMsgUUIDGroupNameReply:
            HandleUuidGroupNameReply(data);
            return false;
        default:
            return false;
        }
    }

    // Inventory
    if (category_id == inventoryEventCategory_)
    {
        // Add new items to inventory_.
        if (event_id == Inventory::Events::EVENT_INVENTORY_DESCENDENT)
            if (inventoryType_ == IDMT_OpenSim)
                checked_static_cast<OpenSimInventoryDataModel *>(inventory_.get())->HandleInventoryDescendents(data);

        // Upload request from other modules.
        if (event_id == Inventory::Events::EVENT_INVENTORY_UPLOAD_FILE)
        {
            InventoryUploadEventData *upload_data = checked_static_cast<InventoryUploadEventData *>(data);
            if (!upload_data)
                return false;

            inventory_->UploadFiles(upload_data->filenames, upload_data->names, 0);
        }

        // Upload request from other modules, using buffers.
        if (event_id == Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER)
        {
            InventoryUploadBufferEventData *upload_data = checked_static_cast<InventoryUploadBufferEventData *>(data);
            if (!upload_data)
                return false;
            inventory_->UploadFilesFromBuffer(upload_data->filenames, upload_data->buffers, 0);
        }

        if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AUTH_RECIEVED)
        {
            if (webdav_inv_creds_recieved_)
                return false;

            Inventory::WebDavCredentials *creds = dynamic_cast<Inventory::WebDavCredentials*>(data);
            if (creds)
            {
                bool credentials_ok = true;
                QString identity = creds->first_name_ + " " + creds->last_name_;

                // check conf for pwd
                QSettings liveidcreds(QSettings::IniFormat, QSettings::UserScope, "realXtend", "credentials/liveid");
                QString inv_password = liveidcreds.value(creds->agent_id_ + "/Password").toString();

                // ... ask if not present
                if (inv_password.isEmpty() || inv_password.isNull())
                {
                    inv_password = QInputDialog::getText(0, "Inventory password", QString("Please provide your inventory password<br>for <b>%1</b>").arg(identity), QLineEdit::Password, "", &credentials_ok);
                     // Store pwd
                    if (credentials_ok)
                        liveidcreds.setValue(creds->agent_id_ + "/Password", inv_password);
                }

                if (credentials_ok)
                {
                    // Create WebDAV inventory model.
                    inventoryType_ = IDMT_WebDav;
                    inventory_ = InventoryPtr(new WebDavInventoryDataModel(identity, webdav_host_, inv_password));
                    inventoryWindow_->InitInventoryTreeModel(inventory_);
                    SAFE_DELETE(service_);
                    service_ = new InventoryService(inventory_.get());
                }
                else
                    LogError("Could not complete getting webdav inv credentials!");

                webdav_inv_creds_recieved_ = true;
            }
        }

        if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_REQUEST ||
            event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_REQUEST)
        {
            InventoryUploadBufferEventData *upload_data = checked_static_cast<InventoryUploadBufferEventData *>(data);
            if (!upload_data)
                return false;

            if (inventoryType_ == IDMT_WebDav)
            {
                boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
                AbstractInventoryItem *avatar_item = inventory_->GetChildFolderById("Avatar"); 

                // If Avatar folder does not exist, create it.
                if (avatar_item == 0)
                {
                    InventoryFolder *root_folder = dynamic_cast<InventoryFolder*>(inventory_->GetRoot());
                    if (root_folder)
                        avatar_item = inventory_->GetOrCreateNewFolder(RexUUID::CreateRandom().ToQString(), *root_folder->GetFirstChildFolderByName("My Inventory"), "Avatar");
                }

                if (avatar_item && asset_service)
                {
                    if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_REQUEST)
                    {
                        // Clean the whole Avatar folder before pushing new assets and xml into it,
                        // so it wont get full and messy over time
                        InventoryFolder *avatar_folder = dynamic_cast<InventoryFolder*>(avatar_item);
                        if (avatar_folder)
                        {
                            LogInfo("Clearing webdav avatar folder before exporting appearance");
                            // Fetch current child list
                            inventory_->FetchInventoryDescendents(avatar_folder);
                            // Get child list
                            QList<AbstractInventoryItem *> avatar_assets = avatar_folder->GetChildren();
                            // Remove all items from avatar folder
                            foreach(AbstractInventoryItem *asset, avatar_assets)
                                inventory_->NotifyServerAboutItemRemove(asset);
                        }
                        LogInfo("Uploading webdav avatar assets");
                    }
                    else if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_REQUEST)
                        LogInfo("Uploading webdav avatar xml description");

                    inventory_->UploadFilesFromBuffer(upload_data->filenames, upload_data->buffers, avatar_item);

                    Inventory::WebDavInventoryUploadedData data(upload_data->filenames);
                    if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_REQUEST)
                        eventManager_->SendEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_COMPLETE, &data);
                    else if (event_id = Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_REQUEST)
                        eventManager_->SendEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_COMPLETE, &data);
                }
                else
                    LogWarning("Could not find Avatar folder from webdav inventory");
            }
            else
                LogWarning("Trying to export webdav avatar without having access to webdav inventory. Are you sure PythonScriptModule is loaded?");
        }

        return false;
    }

    // Framework
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = checked_static_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents();
            return false;
        }

        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                currentWorldStream_ = event_data->WorldStream;

            return false;
        }
    }

    // Asset download related handlers.
    if (inventoryType_ == IDMT_OpenSim)
    {
        if (!inventory_.get())
            return false;

        OpenSimInventoryDataModel *osmodel = checked_static_cast<OpenSimInventoryDataModel *>(inventory_.get());
        if (osmodel->HasPendingDownloadRequests())
        {
            if (category_id == assetEventCategory_ && event_id == Asset::Events::ASSET_READY)
                osmodel->HandleAssetReadyForDownload(data);

            if (category_id == resourceEventCategory_ && event_id == Resource::Events::RESOURCE_READY)
                osmodel->HandleAssetReadyForDownload(data);

            return false;
        }

        if (osmodel->HasPendingOpenItemRequests())
        {
            if (category_id == assetEventCategory_ && event_id == Asset::Events::ASSET_READY)
                osmodel->HandleAssetReadyForOpen(data);

            return false;
        }
    }

    ///\todo Handle AssetReady for ItemPropertiesWindows

    return false;
}

Console::CommandResult InventoryModule::UploadAsset(const StringVector &params)
{
    using namespace RexTypes;

    if (!currentWorldStream_.get())
        return Console::ResultFailure("Not connected to server.");

    if (!inventory_.get())
        return Console::ResultFailure("Inventory doesn't exist. Can't upload!.");

    if (inventoryType_ != IDMT_OpenSim)
        return Console::ResultFailure("Console upload supported only for classic OpenSim inventory.");

    std::string name = "(No Name)";
    std::string description = "(No Description)";

    if (params.size() < 1)
        return Console::ResultFailure("Invalid syntax. Usage: \"upload(asset_type, name, description)."
            "Name and description are optional. Supported asset types:\n"
            "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

    asset_type_t asset_type = GetAssetTypeFromTypeName(params[0]);
    if (asset_type == RexAT_None)
        return Console::ResultFailure("Invalid asset type. Supported parameters:\n"
            "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

    if (params.size() > 1)
        name = params[1];

    if (params.size() > 2)
        description = params[2];

    std::string filter = GetOpenFileNameFilter(asset_type);
    std::string cat_name = GetCategoryNameForAssetType(asset_type);

    RexUUID folder_id = RexUUID(inventory_->GetFirstChildFolderByName(cat_name.c_str())->GetID().toStdString());
    if (folder_id.IsNull())
        return Console::ResultFailure("Inventory folder for this type of file doesn't exists. File can't be uploaded.");

    currentWorldStream_->SendAgentPausePacket();

    QString filename = QFileDialog::getOpenFileName(0, filter.c_str(), "Open", QDir::currentPath());
    if (filename.isEmpty())
        return Console::ResultFailure("No file chosen.");

    currentWorldStream_->SendAgentResumePacket();

    static_cast<OpenSimInventoryDataModel *>(inventory_.get())->UploadFile(asset_type, filename.toStdString(), name, description, folder_id);

    return Console::ResultSuccess();
}

Console::CommandResult InventoryModule::UploadMultipleAssets(const StringVector &params)
{
    if (!currentWorldStream_.get())
        return Console::ResultFailure("Not connected to server.");

    if (!inventory_.get())
        return Console::ResultFailure("Inventory doesn't exist. Can't upload!");

    if (inventoryType_ != IDMT_OpenSim)
        return Console::ResultFailure("Console upload supported only for classic OpenSim inventory.");

    currentWorldStream_->SendAgentPausePacket();

    QStringList filenames = QFileDialog::getOpenFileNames(0, "Open", QDir::currentPath(), RexTypes::rexFileFilters);
    if (filenames.empty())
        return Console::ResultFailure("No files chosen.");

    currentWorldStream_->SendAgentResumePacket();

    QStringList itemNames;
    static_cast<OpenSimInventoryDataModel *>(inventory_.get())->UploadFiles(filenames, itemNames, 0);

    return Console::ResultSuccess();
}

#ifdef _DEBUG
Console::CommandResult InventoryModule::InventoryServiceTest(const StringVector &params)
{
    if (!currentWorldStream_.get())
        return Console::ResultFailure("Not connected to server.");

    if (!inventory_.get())
        return Console::ResultFailure("Inventory doesn't exist.");

    if (!service_)
        return Console::ResultFailure("Inventory service doesn't exist.");

    asset_type_t asset_type = GetAssetTypeFromTypeName(params[0]);
    if (asset_type == RexTypes::RexAT_None)
        return Console::ResultFailure("Invalid asset type. Supported parameters:\n"
            "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

    ///\todo Proper test interface
    QList<const InventoryAsset *> assets = service_->GetAssetsByAssetType(RexTypes::RexAT_Texture);
    QListIterator<const InventoryAsset *> it(assets);
    while(it.hasNext())
        std::cout << it.next()->GetName().toStdString() << std::endl;

    return Console::ResultSuccess();
}
#endif

void InventoryModule::OpenItemPropertiesWindow(const QString &inventory_id)
{
    UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>("UiServices").lock();
    if (!ui_module.get())
        return;

    // Check that item properties window for this item doesn't already exists.
    // If it does, bring it to front and set focus to it.
    QMap<QString, ItemPropertiesWindow *>::iterator it = itemPropertiesWindows_.find(inventory_id);
    if (it != itemPropertiesWindows_.end())
    {
        ui_module->GetInworldSceneController()->BringProxyToFront(it.value());
        return;
    }

    InventoryAsset *asset = dynamic_cast<InventoryAsset *>(inventory_->GetChildById(inventory_id));
    if (!asset)
        return;

    ItemPropertiesWindow *wnd = new ItemPropertiesWindow(this);
    connect(wnd, SIGNAL(Closed(const QString &, bool)), this, SLOT(CloseItemPropertiesWindow(const QString &, bool)));
    wnd->SetItem(asset);
    itemPropertiesWindows_[inventory_id] = wnd;

    // Add widget to UI scene
    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(
        wnd, UiServices::UiWidgetProperties("Item Properties", UiServices::SceneWidget));
    QObject::connect(proxy, SIGNAL(Closed()), wnd, SLOT(Cancel()));
    proxy->show();
    ui_module->GetInworldSceneController()->BringProxyToFront(proxy);

    if (inventoryType_ == IDMT_OpenSim)
    {
        static_cast<OpenSimInventoryDataModel *>(inventory_.get())->SendNameUuidRequest(asset);

        // Get asset service interface and check if the asset is in cache.
        // If it is, show file size to item properties UI. SLUDP protocol doesn't support querying asset size
        // and we don't want download asset only just to know its size.
        ///\todo If WebDAV supports querying asset size without full download, utilize it.
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager->IsRegistered(Foundation::Service::ST_Asset))
            return;

        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
            service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();

        Foundation::AssetPtr assetPtr = asset_service->GetAsset(asset->GetAssetReference().toStdString(), GetTypeNameFromAssetType(asset->GetAssetType()));
        if (assetPtr && assetPtr->GetSize() > 0)
            wnd->SetFileSize(assetPtr->GetSize());
    }
}

void InventoryModule::CloseItemPropertiesWindow(const QString &inventory_id, bool save_changes)
{
    // Note: We only remove the pointer from the map here. The window deletes itself.
    ItemPropertiesWindow *wnd = itemPropertiesWindows_.take(inventory_id);
    if (!wnd)
        return;

    UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>("UiServices").lock();
    if (ui_module)
        ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(wnd);

    // If inventory item is modified notify server.
    if (save_changes)
    {
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(inventory_->GetChildById(inventory_id));
        if (asset)
            ///\todo WebDAV needs the old name and we don't have it here.
            inventory_->NotifyServerAboutItemUpdate(asset, asset->GetName());
    }

    SAFE_DELETE_LATER(wnd);
}

void InventoryModule::HandleInventoryDescendents(Foundation::EventDataInterface* event_data)
{
    ProtocolUtilities::NetworkEventInboundData *data = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(event_data);
    assert(data);
    if (!data)
        return;

    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    // AgentData
    RexUUID agent_id = msg.ReadUUID();
    RexUUID session_id = msg.ReadUUID();

    // Check that this packet is for us.
    if (agent_id != currentWorldStream_->GetInfo().agentID &&
        session_id != currentWorldStream_->GetInfo().sessionID)
    {
        LogError("Received InventoryDescendents packet with wrong AgentID and/or SessionID.");
        return;
    }

    msg.SkipToNextVariable();               //OwnerID UUID, owner of the folders creatd.
    msg.SkipToNextVariable();               //Version S32, version of the folder for caching
    int32_t descendents = msg.ReadS32();    //Descendents, count to help with caching
    if (descendents == 0)
        return;

    // For hackish protection against weird behaviour of 0.4 server. See below.
    bool exceptionOccurred = false;

    // FolderData, Variable block.
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        try
        {
            // Gather event data.
            Inventory::InventoryItemEventData folder_data(Inventory::IIT_Folder);
            folder_data.id = msg.ReadUUID();
            folder_data.parentId = msg.ReadUUID();
            folder_data.inventoryType = msg.ReadS8();
            folder_data.name = msg.ReadString();

            // Send event.
            eventManager_->SendEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_DESCENDENT, &folder_data);
        }
        catch(NetMessageException &)
        {
            exceptionOccurred = true;
        }
    }

    ///\note Hackish protection against weird behaviour of 0.4 server. It seems that even if the block instance count
    /// of FolderData should be 0, we read it as 1. Reset reading and skip first 5 variables. After that start reading
    /// data from block interpreting it as ItemData block. This problem doesn't happen with 0.5.
    if (exceptionOccurred)
    {
        msg.ResetReading();
        for(int i = 0; i < 5; ++i)
            msg.SkipToNextVariable();
    }

    // ItemData, Variable block.
    instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        try
        {
            // Gather event data.
            Inventory::InventoryItemEventData asset_data(Inventory::IIT_Asset);
            asset_data.id = msg.ReadUUID();
            asset_data.parentId = msg.ReadUUID();
            asset_data.creatorId = msg.ReadUUID();
            asset_data.ownerId = msg.ReadUUID();
            asset_data.groupId = msg.ReadUUID();

            ///\note Skipping some permission & sale related stuff.
            msg.SkipToFirstVariableByName("AssetID");
            asset_data.assetId = msg.ReadUUID();
            asset_data.assetType = msg.ReadS8();
            asset_data.inventoryType = msg.ReadS8();
            msg.SkipToFirstVariableByName("Name");
            asset_data.name = msg.ReadString();
            asset_data.description = msg.ReadString();

            asset_data.creationTime = msg.ReadS32();
            msg.SkipToNextInstanceStart();
            //msg.ReadU32(); //CRC

            // Send event.
            eventManager_->SendEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_DESCENDENT, &asset_data);
        }
        catch(NetMessageException &e)
        {
            LogError("Catched NetMessageException: " + e.What() + " while reading InventoryDescendents packet.");
        }
    }

    return;
}

void InventoryModule::HandleUpdateCreateInventoryItem(Foundation::EventDataInterface* event_data)
{
    ///\note It seems that this packet is only sent by 0.4 reX server.
    ///     Maybe drop support at some point?
    ProtocolUtilities::NetworkEventInboundData* data = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(event_data);
    assert(data);
    if (!data)
        return;

    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    // AgentData
    RexUUID agent_id = msg.ReadUUID();
    if (agent_id != currentWorldStream_->GetInfo().agentID)
    {
        LogError("Received UpdateCreateInventoryItem packet with wrong AgentID, ignoring packet.");
        return;
    }

    bool simApproved = msg.ReadBool();
    if (!simApproved)
    {
        LogInfo("Server did not approve your inventory item upload!");
        return;
    }

    msg.SkipToNextVariable(); // TransactionID, UUID

    // InventoryData, variable block.
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        try
        {
            // Gather event data.
            Inventory::InventoryItemEventData asset_data(Inventory::IIT_Asset);
            asset_data.id = msg.ReadUUID();
            asset_data.parentId = msg.ReadUUID();

            ///\note Skipping all permission & sale related stuff.
            msg.SkipToFirstVariableByName("AssetID");
            asset_data.assetId = msg.ReadUUID();
            asset_data.assetType = msg.ReadS8();
            asset_data.inventoryType = msg.ReadS8();
            msg.SkipToFirstVariableByName("Name");
            asset_data.name = msg.ReadString();
            asset_data.description = msg.ReadString();

            msg.SkipToNextInstanceStart();
            //msg.ReadS32(); //CreationDate
            //msg.ReadU32(); //CRC

            // Send event.
            eventManager_->SendEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_DESCENDENT, &asset_data);
        }
        catch (NetMessageException &e)
        {
            LogError("Catched NetMessageException: " + e.What() + " while reading UpdateCreateInventoryItem packet.");
        }
    }
}

void InventoryModule::HandleUuidNameReply(Foundation::EventDataInterface* event_data)
{
    ProtocolUtilities::NetworkEventInboundData *data = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(event_data);
    assert(data);
    if (!data)
        return;

    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    // UUIDNameBlock, variable block.
    QMap<RexUUID, QString> map;
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        RexUUID id = msg.ReadUUID();
        QString name(msg.ReadString().c_str());
        name.append(" ");
        name.append(msg.ReadString().c_str());
        map[id] = name;
    }

    // Pass UuidNameReplys to item properties windows.
    QMapIterator<QString, ItemPropertiesWindow *> it(itemPropertiesWindows_);
    while(it.hasNext())
        it.next().value()->HandleUuidNameReply(map);
}

void InventoryModule::HandleUuidGroupNameReply(Foundation::EventDataInterface* event_data)
{
    ProtocolUtilities::NetworkEventInboundData* data = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(event_data);
    assert(data);
    if (!data)
        return;

    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    // UUIDNameBlock, variable block.
    QMap<RexUUID, QString> map;
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    for(size_t i = 0; i < instance_count; ++i)
    {
        RexUUID id = msg.ReadUUID();
        QString name(msg.ReadString().c_str());
        map[id] = name;
    }

    // Pass UuidGroupNameReplys to item properties windows
    QMapIterator<QString, ItemPropertiesWindow *> it(itemPropertiesWindows_);
    while(it.hasNext())
        it.next().value()->HandleUuidNameReply(map);
}

void InventoryModule::DeleteAllItemPropertiesWindows()
{
    QMutableMapIterator<QString, ItemPropertiesWindow *> it(itemPropertiesWindows_);
    while(it.hasNext())
    {
        ItemPropertiesWindow *wnd = it.next().value();
        SAFE_DELETE(wnd);
        it.remove();
    }
}

void InventoryModule::ConnectSignals()
{
/*
    if (!uploadProgressWindow_)
        return;

    // Connect upload progress signals.
    QObject::connect(inventory_.get(), SIGNAL(MultiUploadStarted(size_t)),
        uploadProgressWindow_, SLOT(OpenUploadProgress(size_t)));

    QObject::connect(inventory_.get(), SIGNAL(UploadStarted(const QString &)),
        uploadProgressWindow_, SLOT(UploadStarted(const QString &)));

    connect(inventory_.get(), SIGNAL(UploadFailed(const QString &)),
        uploadProgressWindow_, SLOT(UploadProgress(const QString &)));

    connect(inventory_.get(), SIGNAL(UploadCompleted(const QString &)),
        uploadProgressWindow_, SLOT(UploadProgress(const QString &)));

    QObject::connect(inventory_.get(), SIGNAL(MultiUploadCompleted()),
        uploadProgressWindow_, SLOT(CloseUploadProgress()));
*/
}

} // namespace Inventory

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Inventory;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(InventoryModule)
POCO_END_MANIFEST
