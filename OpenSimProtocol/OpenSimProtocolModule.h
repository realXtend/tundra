// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_h
#define incl_OpenSimProtocolModule_h

#include "ModuleInterface.h"

#include "NetMessageManager.h"
#include "NetworkConnection.h"
#include "INetMessageListener.h"
#include "PocoXMLRPC.h"
#include "RexUUID.h"

#include <boost/shared_ptr.hpp>

using namespace Core;
using namespace boost;

namespace Foundation
{
   class Framework;
}

struct ClientParameters
{
    RexUUID     agentID;
    RexUUID     sessionID;
    RexUUID     regionID;
    uint32_t    circuitCode;
};

namespace OpenSimProtocol
{
    //! interface for modules
    class VIEWER_API OpenSimProtocolModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OpenSimProtocolModule();
        virtual ~OpenSimProtocolModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();
		
		MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        
        //! Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Network;
		
		/// Get state of the network module.
		const u8 GetNetworkState() const { return networkState_; }
		
		/// Get the message template filename from xml configuration file.
		//const std::string &GetTemplateFilename();
        
        /// Adds listener to the network module.
        void AddListener(INetMessageListener *listener);
        void RemoveListener(INetMessageListener *listener);
 
        /// Connects to a reX server.
        bool ConnectToRexServer(
            const char *first_name,
		    const char *last_name,
		    const char *password,
		    const char *address,
		    int port,
		    ClientParameters *params);
        
        /// Disconnects from a reX server.
       	void DisconnectFromRexServer();

  	    /// State of the network connection.
        enum NetworkState
        {	
	        State_Connected,
	        State_Disconnected,
        };
        
        void DumpNetworkMessage(NetMsgID id, NetInMessage *msg);

        /// Start building a new outbound message.
        /// @return An empty message holder where the message can be built.
        NetOutMessage *StartMessageBuilding(NetMsgID msgId);
        
        /// Finish (send) the message.
        void FinishMessageBuilding(NetOutMessage *msg);

    private:
        /// Initializes a login to a reX server that is not using a separate authentication server.
       	void PerformXMLRPCLogin(
       	    const char *first_name,
       	    const char *last_name,
       	    const char *password,
       	    const char *address,
       	    int port,
       	    ClientParameters *params);
       
        Foundation::Framework *framework_;
        
	    /// Handles the UDP communications with the reX server.
	    shared_ptr<NetMessageManager> networkManager_;
	    
	    /// Handles the initial XMLRPC messaging.
	    shared_ptr<PocoXMLRPCConnection> rpcConnection_;
	    
		/// State of the network connection.
		NetworkState networkState_;
    };
}

#endif
