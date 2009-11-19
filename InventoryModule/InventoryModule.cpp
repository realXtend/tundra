// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModule.cpp
 *  @brief Inventory module.
 */

#include "StableHeaders.h"
#include "InventoryModule.h"
#include "RexLogicModule.h"
#include "InventoryWindow.h"
#include "NetworkEvents.h"
#include "Inventory/InventoryEvents.h"
#include "AssetUploader.h"
#include "QtUtils.h"
#include "AssetEvents.h"
#include "OpenSimInventoryDataModel.h"

#include <QStringList>
#include <QVector>

namespace Inventory
{

InventoryModule::InventoryModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_Inventory),
    inventoryEventCategory_(0),
    networkStateEventCategory_(0),
    assetEventCategory_(0),
    resourceEventCategory_(0),
    frameworkEventCategory_(0),
    inventoryWindow_(0)
{
}

// virtual
InventoryModule::~InventoryModule()
{
}

void InventoryModule::Load()
{
    LogInfo("System " + Name() + " loaded.");
}

void InventoryModule::Unload()
{
    LogInfo("System " + Name() + " unloaded.");
}

void InventoryModule::Initialize()
{
    // Refister event category and events.
    eventManager_ = framework_->GetEventManager();
    inventoryEventCategory_ = eventManager_->RegisterEventCategory("Inventory");
    eventManager_->RegisterEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_DESCENDENT, "InventoryDescendent");
    eventManager_->RegisterEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_UPLOAD, "InventoryUpload");
    eventManager_->RegisterEvent(inventoryEventCategory_, Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, "InventoryUploadBuffer");

    // Register console commands.
    boost::shared_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>
        (Foundation::Service::ST_ConsoleCommand).lock();
    if (console)
    {
        console->RegisterCommand(Console::CreateCommand("Upload",
            "Upload an asset. Usage: Upload(AssetType, Name, Description)",
            Console::Bind(this, &Inventory::InventoryModule::UploadAsset)));

        console->RegisterCommand(Console::CreateCommand("MultiUpload", "Upload multiple assets.",
            Console::Bind(this, &Inventory::InventoryModule::UploadMultipleAssets)));
    }

    // Create inventory window.
    inventoryWindow_ = new InventoryWindow(framework_);

    LogInfo("System " + Name() + " initialized.");
}

void InventoryModule::PostInitialize()
{
    frameworkEventCategory_ = eventManager_->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");

    assetEventCategory_ = eventManager_->QueryEventCategory("Asset");
    if (assetEventCategory_ == 0)
        LogError("Failed to query \"Asset\" event category");

    resourceEventCategory_ = eventManager_->QueryEventCategory("Resource");
    if (resourceEventCategory_ == 0)
        LogError("Failed to query \"Resource\" event category");
}

void InventoryModule::Uninitialize()
{
//    assetUploader_.reset();
    SAFE_DELETE(inventoryWindow_);
    LogInfo("System " + Name() + " uninitialized.");
}

void InventoryModule::SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule)
{
    networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
    if (networkStateEventCategory_ == 0)
        LogError("Failed to query \"NetworkState\" event category");
    else
        LogInfo("System " + Name() + " subscribed to [NetworkIn]");
}

void InventoryModule::Update(Core::f64 frametime)
{
}

bool InventoryModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    if (category_id == networkStateEventCategory_)
    {
        // Connected to server. Initialize inventory tree model.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            ProtocolUtilities::AuthenticationEventData *auth_data = dynamic_cast<ProtocolUtilities::AuthenticationEventData *>(data);
            if (!auth_data)
                return false;

            switch(auth_data->type)
            {
            case ProtocolUtilities::AT_Taiga:
                // Check if python module is loaded and has taken care of PythonQt::init()
                if (!framework_->GetModuleManager()->HasModule(Foundation::Module::MT_PythonScript))
                    LogError("Python module not in use. WebDav inventory can't be used!");
                else
                    dataModel_ = inventoryWindow_->InitWebDavInventoryTreeModel(0, auth_data->identityUrl, auth_data->hostUrl);
                break;
            case ProtocolUtilities::AT_OpenSim:
            case ProtocolUtilities::AT_RealXtend:
            {
                dataModel_ = inventoryWindow_->InitOpenSimInventoryTreeModel(this, GetCurrentWorldStream());
                OpenSimInventoryDataModel *osmodel = static_cast<OpenSimInventoryDataModel *>(dataModel_);
                osmodel->GetAssetUploader()->SetWorldStream(currentWorldStream_);
                break;
            }
            default:
                break;
            }

            return false;
        }

        // Disconnected from server. Hide inventory and reset tree model.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (inventoryWindow_)
            {
                inventoryWindow_->Hide();
                inventoryWindow_->ResetInventoryTreeModel();
            }
        }

        return false;
    }

    if (category_id == inventoryEventCategory_)
    {
        // Add new items to inventory.
        if (event_id == Inventory::Events::EVENT_INVENTORY_DESCENDENT)
        {
            InventoryItemEventData *item_data = dynamic_cast<InventoryItemEventData *>(data);
            if (!item_data)
                return false;
            inventoryWindow_->HandleInventoryDescendent(item_data);
        }

        // Upload request from other modules.
        if (event_id == Inventory::Events::EVENT_INVENTORY_UPLOAD)
        {
            InventoryUploadEventData *upload_data = dynamic_cast<InventoryUploadEventData *>(data);
            if (!upload_data)
                return false;

            // Convert to QStringList
            ///\todo Use QStringList all the way...
            QStringList filenames;
            Core::StringList list = upload_data->filenames;
            for (Core::StringList::iterator it = list.begin(); it != list.end(); ++it)
                filenames << QString((*it).c_str());

            dataModel_->UploadFiles(filenames, 0);
        }

        // Upload request from other modules, using buffers.
        if (event_id == Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER)
        {
            InventoryUploadBufferEventData *upload_data = dynamic_cast<InventoryUploadBufferEventData *>(data);
            if (!upload_data)
                return false;

            // Convert to QList and QVector
            ///\todo Use Qt types all the way...
            QStringList filenames;
            QVector<QVector<Core::u8>> buffers;

            Core::StringList::iterator it;
            for(it = upload_data->filenames.begin(); it != upload_data->filenames.end(); ++it)
                filenames << QString((*it).c_str());

            std::vector<std::vector<Core::u8>>::iterator iter;
            for(iter = upload_data->buffers.begin(); iter != upload_data->buffers.end(); ++iter)
                buffers << QVector<uchar>::fromStdVector(*iter);

            dataModel_->UploadFilesFromBuffer(filenames, buffers, 0);
        }

        return false;
    }

    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            Foundation::NetworkingRegisteredEvent *event_data = dynamic_cast<Foundation::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents(event_data->currentProtocolModule);
            return false;
        }
        else if (event_id == Foundation::WORLD_STREAM_READY)
        {
            Foundation::WorldStreamReadyEvent *event_data = dynamic_cast<Foundation::WorldStreamReadyEvent *>(data);
            if (event_data)
            {
                currentWorldStream_ = event_data->WorldStream;
                inventoryWindow_->SetWorldStreamToDataModel(currentWorldStream_);
            }

            return false;
        }
    }

    if (category_id == assetEventCategory_ && event_id == Asset::Events::ASSET_READY)
        inventoryWindow_->HandleResourceReady(false, data);

    if (category_id == resourceEventCategory_ && event_id == Resource::Events::RESOURCE_READY)
        inventoryWindow_->HandleResourceReady(true, data);

    return false;
}

Console::CommandResult InventoryModule::UploadAsset(const Core::StringVector &params)
{
    return Console::ResultFailure("Single upload disabled for now.");
/*
    using namespace RexTypes;
    using namespace OpenSimProtocol;

    std::string name = "(No Name)";
    std::string description = "(No Description)";

    if (params.size() < 1)
        return Console::ResultFailure("Invalid syntax. Usage: \"upload [asset_type] [name] [description]."
            "Name and description are optional. Supported asset types:\n"
            "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

    asset_type_t asset_type = GetAssetTypeFromTypeName(params[0]);
    if (asset_type == -1)
        return Console::ResultFailure("Invalid asset type. Supported parameters:\n"
            "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

    if (params.size() > 1)
        name = params[1];

    if (params.size() > 2)
        description = params[2];

    std::string filter = GetOpenFileNameFilter(asset_type);
    std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Open", Foundation::QtUtils::GetCurrentPath());
    if (filename == "")
        return Console::ResultFailure("No file chosen.");

    assetUploader_->UploadFile(asset_type, filename, name, description, folder_id)

    return Console::ResultSuccess();
*/
}

/*
AssetUploaderPtr InventoryModule::GetAssetUploader() const
{
    return assetUploader_;
}
*/

Console::CommandResult InventoryModule::UploadMultipleAssets(const Core::StringVector &params)
{
    if (!currentWorldStream_.get())
        return Console::ResultFailure("Not connected to server.");

    currentWorldStream_->SendAgentPausePacket();

    Core::StringList filenames = Foundation::QtUtils::GetOpenRexFileNames(Foundation::QtUtils::GetCurrentPath());
    if (filenames.empty())
        return Console::ResultFailure("No files chosen.");

    currentWorldStream_->SendAgentResumePacket();

    OpenSimInventoryDataModel *osmodel = dynamic_cast<OpenSimInventoryDataModel *>(dataModel_);
    if (osmodel)
    {
        AssetUploader *au = osmodel->GetAssetUploader();
        if (!au)
            return Console::ResultFailure("Asset uploader not initialized. Can't upload.");

        au->UploadFiles(filenames);
    }
    else
        return Console::ResultFailure("Console upload supported only for classic OpenSim inventory.");

    return Console::ResultSuccess();
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
