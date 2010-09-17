// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include "ConsoleCommandServiceInterface.h"
#include "AssetProviderInterface.h"
#include "AssetModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

namespace Asset
{
    class AssetManager;
    typedef boost::shared_ptr<AssetManager> AssetManagerPtr;
    
    /** \defgroup AssetModuleClient AssetModule Client Interface
        This page lists the public interface of the AssetModule,
        which consists of implementing Foundation::AssetServiceInterface and
        Foundation::AssetInterface

        For details on how to use the public interface, see \ref AssetModule "Using the asset module".
    */

    //! Asset module.
    class ASSET_MODULE_API AssetModule : public IModule
    {
    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Load();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        virtual void SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule);
        void UnsubscribeNetworkEvents();

        MODULE_LOGGING_FUNCTIONS

        //! callback for console command
        Console::CommandResult ConsoleRequestAsset(const StringVector &params);

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

    private:
        //! Type name of the module.
        static std::string type_name_static_;

        //! UDP asset provider
        Foundation::AssetProviderPtr udp_asset_provider_;

        //! XMLRPC asset provider
        Foundation::AssetProviderPtr xmlrpc_asset_provider_;

        //! Local asset provider
        Foundation::AssetProviderPtr local_asset_provider_;
        
        //! Http asset provider
        Foundation::AssetProviderPtr http_asset_provider_;

        //! asset manager
        AssetManagerPtr manager_;

        //! category id for incoming messages
        event_category_id_t inboundcategory_id_;

        //! category id for network state events
        event_category_id_t network_state_category_id_;

        //! framework id for internal events
        event_category_id_t framework_category_id_;

        //! Pointer to current ProtocolModule
        boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule_;
    };
}

#endif
