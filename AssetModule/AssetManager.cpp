#include "StableHeaders.h"
#include "AssetModule.h"
#include "AssetInterface.h"
#include "OpenSimProtocolModule.h"
#include "AssetManager.h"


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
            
        net_interface_ = dynamic_cast<OpenSimProtocol::OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol));
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
}