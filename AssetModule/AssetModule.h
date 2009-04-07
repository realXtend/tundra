// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "ModuleInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventDataInterface.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

namespace Asset
{
    class AssetManager;
    typedef boost::shared_ptr<AssetManager> AssetManagerPtr;
    
    /// event data interface for asset messages
    class AssetEventData : public Foundation::EventDataInterface
    {
    public:
        AssetEventData(const std::string& asset_id, Core::asset_type_t asset_type) :
            asset_id_(asset_id),
            asset_type_(asset_type)
        {
        }
        
        virtual ~AssetEventData() 
        {
        }
        
        std::string asset_id_;
        Core::asset_type_t asset_type_;
    };
    
    //! asset module
    class MODULE_API AssetModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();
        virtual void PostInitialize();
        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }

        MODULE_LOGGING_FUNCTIONS

        //! callback for console command
        Console::CommandResult ConsoleRequestAsset(const Core::StringVector &params);
        
        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Asset;

    private:
        //! asset manager
        AssetManagerPtr manager_;
        
         //! network interface
        OpenSimProtocol::OpenSimProtocolModule *net_interface_;
        
        //! category id for incoming messages
        Core::event_category_id_t inboundcategory_id_;
    };
}

#endif
