// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "OpenSimProtocolModule.h"
#include "RexProtocolMsgIDs.h"
#include "RexUUID.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    AssetModule::AssetModule() : ModuleInterfaceImpl(type_static_),
        inboundcategory_id_(0),
        net_interface_(NULL)
    {
    }

    AssetModule::~AssetModule()
    {
    }

    // virtual
    void AssetModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
        
        AutoRegisterConsoleCommand(Console::CreateCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            Console::Bind(this, &AssetModule::ConsoleRequestAsset)));
    }

    // virtual
    void AssetModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void AssetModule::Initialize()
    {
        manager_ = AssetManagerPtr(new AssetManager(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Asset, manager_.get());
        
        LogInfo("Module " + Name() + " initialized.");
    }
    
    // virtual
    void AssetModule::PostInitialize()
    {
        inboundcategory_id_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        
        if (inboundcategory_id_ == 0 )
            LogWarning("Unable to find event category for OpenSimNetwork events!");
    }

    // virtual
    void AssetModule::Update(Core::f64 frametime)
    {
        if (manager_)
            manager_->Update(frametime);
    }

    // virtual 
    void AssetModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_.get());
        manager_.reset();
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    Console::CommandResult AssetModule::ConsoleRequestAsset(const Core::StringVector &params)
    {
        if (params.size() != 2)
        {
            return Console::ResultFailure("Usage: RequestAsset(uuid,assettype)");
        }

        try
        {
            int asset_type = Core::ParseString<int>(params[1]);

            manager_->GetAsset(params[0], asset_type);
        } catch (std::exception)
        {
            return Console::ResultInvalidParameters();
        }

        return Console::ResultSuccess();
    }
    
    bool AssetModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        if ((category_id == inboundcategory_id_) && (event_id == OpenSimProtocol::EVENT_NETWORK_IN))
        {
            NetworkEventInboundData *event_data = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            const NetMsgID msgID = event_data->messageID;
            NetInMessage *msg = event_data->message;
                
            switch(msgID)
            {
                case RexNetMsgImageData:
                manager_->HandleTextureHeader(msg);
                return true;
                
                case RexNetMsgImagePacket:
                manager_->HandleTextureData(msg);
                return true;
                
                case RexNetMsgImageNotInDatabase:
                manager_->HandleTextureCancel(msg);
                return true;
                
                case RexNetMsgTransferInfo:
                manager_->HandleAssetHeader(msg);
                return true;
                
                case RexNetMsgTransferPacket:
                manager_->HandleAssetData(msg);
                return true;
                
                case RexNetMsgTransferAbort:
                manager_->HandleAssetCancel(msg);
                return true;
            }
        }
        return false;
    }
}

using namespace Asset;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(AssetModule)
POCO_END_MANIFEST

