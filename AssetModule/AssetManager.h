// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetManager_h
#define incl_Asset_AssetManager_h

#include "AssetServiceInterface.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

namespace RexTypes
{   
    class RexUUID;
}

namespace Asset
{
    class AssetManager : public Foundation::AssetServiceInterface
    {
    public:
        AssetManager(Foundation::Framework* framework);
        virtual ~AssetManager();
        
        virtual Foundation::AssetPtr GetAsset(const std::string& asset_id);

        void RequestAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
        bool AssetManager::Initialize();
        
        bool IsInitialized() const { return initialized_; }
        
    private:
        //! Pointer to the network interface.
        OpenSimProtocol::OpenSimProtocolModule *net_interface_;
        
        //! framework we belong to
        Foundation::Framework* framework_;
        
        //! initialized flag
        bool initialized_;
    };
}


#endif