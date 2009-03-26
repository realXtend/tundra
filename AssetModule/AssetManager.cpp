#include "StableHeaders.h"
#include "AssetModule.h"
#include "AssetInterface.h"
#include "OpenSimProtocolModule.h"
#include "RexProtocolMsgIDs.h"
#include "AssetDefines.h"
#include "AssetManager.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    AssetManager::AssetManager(Foundation::Framework* framework) : 
        framework_(framework),
        net_interface_(NULL)
    {
    }
    
    AssetManager::~AssetManager()
    {
    }
    
    bool AssetManager::Initialize()
    {
        if (initialized_)
            return true;
            
        net_interface_ = dynamic_cast<OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol));
        if (!net_interface_)
        {
            AssetModule::LogError("Getting network interface did not succeed."); 
            return false;
        }
        
        initialized_ = true;
        return true;
    }
    
    Foundation::AssetPtr AssetManager::GetAsset(const std::string& asset_id)
    {
        Foundation::AssetPtr no_asset;
        return no_asset;
    }
    
    void AssetManager::RequestAsset(const RexUUID& asset_id, Core::uint asset_type)
    {
        if (!initialized_)
            return;
        
        const ClientParameters& client = net_interface_->GetClientParameters();

        // texture
        if (asset_type == RexAT_Texture)
        {
            NetOutMessage *m = net_interface_->StartMessageBuilding(RexNetMsgRequestImage);
            assert(m);
            
            m->AddUUID(client.agentID);
            m->AddUUID(client.sessionID);
            
            m->SetVariableBlockCount(1);
            m->AddUUID(asset_id); // Image UUID
            m->AddS8(0); // Discard level
            m->AddF32(100.0); // Download priority
            m->AddU32(0); // Starting packet
            m->AddU8(RexIT_Normal); // Image type
            
            net_interface_->FinishMessageBuilding(m);
        }
        // other assettypes
        else
        {
            NetOutMessage *m = net_interface_->StartMessageBuilding(RexNetMsgTransferRequest);
            assert(m);
            
            RexUUID transfer_id;
            transfer_id.Random();
            
            m->AddUUID(transfer_id); // Transfer ID
            m->AddS32(RexAC_Asset); // Asset channel type
            m->AddS32(RexAS_Asset); // Asset source type
            m->AddF32(100.0); // Download priority
            
            Core::u8 asset_info[20]; // Asset info block with UUID and type
            memcpy(&asset_info[0], &asset_id.data, 16);
            memcpy(&asset_info[16], &asset_type, 4);
            m->AddBuffer(20, asset_info);
            
            net_interface_->FinishMessageBuilding(m);
        }
    }
}