// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetTestLogicModule_h
#define incl_NetTestLogicModule_h

#include "ModuleInterface.h"

#include "OpenSimProtocolModule.h"

#include "NetInMessage.h"
#include "INetMessageListener.h"
#include "NetMessage.h"

namespace Foundation
{
   class Framework;
}

namespace OpenSimProtocol
{
	class OpenSimProtocolModule;
}

//! Contains unit tests
/*! All Core and Foundation classes should be unit tested.
*/
namespace NetTest
{
    //! interface for modules
    class NetTestLogicModule: public Foundation::ModuleInterface_Impl, public INetMessageListener
    {
    public:
        NetTestLogicModule();
        virtual ~NetTestLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::Type_Renderer;
        
        virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg);
        
    private:
        void PerformXMLRPCLogin(const char *first_name, const char *last_name, const char *password, const char *address, int port);
        
        Foundation::Framework *framework_;
		OpenSimProtocol::OpenSimProtocolModule *netInterface_;
		/// Temporary counter.
		u32 updateCounter;
		
		
    };
}
#endif
