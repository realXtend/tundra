// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_h
#define incl_OpenSimProtocolModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "EventDataInterface.h"

//#include "OpenSimProtocolModule_API.h"
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
   class EventDataInterface;
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
    /// Event data interface for inbound messages
    class NetworkEventInboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventInboundData(NetMsgID id, NetInMessage *msg)
        : message(msg), messageID(id) {}
        virtual ~NetworkEventInboundData() {}
        
        NetMsgID messageID;
        NetInMessage *message;
    };
    
    /// Event data interface for outbound messages
    class NetworkEventOutboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventOutboundData(NetMsgID id, const NetOutMessage *msg)
        : message(msg), messageID(id) {}
        virtual ~NetworkEventOutboundData() {}
        
        NetMsgID messageID;
        const NetOutMessage *message;
    };
        
    //! interface for modules
    class MODULE_API OpenSimProtocolModule : public Foundation::ModuleInterface_Impl, public INetMessageListener
    {
    public: 
        OpenSimProtocolModule();
        virtual ~OpenSimProtocolModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update();
		
		MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        
        //! Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Network;
        
        virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg);
        
        virtual void OnNetworkMessageSent(const NetOutMessage *msg);
		
        /// Adds listener to the network module. todo Remove
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

        void DumpNetworkMessage(NetMsgID id, NetInMessage *msg);

        /// Start building a new outbound message.
        /// @return An empty message holder where the message can be built.
        NetOutMessage *StartMessageBuilding(NetMsgID msgId);
        
        /// Finishes (sends) the message. The passed msg pointer will be invalidated after calling this, so don't
        /// access it or hold on to it afterwards. The user doesn't have to do any deallocation, it is all managed by
        /// this class.
        void FinishMessageBuilding(NetOutMessage *msg);

        /// Network event ID.
        static const Core::event_id_t EVENT_NETWORK_IN = 0x1;
        /// NetworkOutStats event ID.
        static const Core::event_id_t EVENT_NETWORK_OUT = 0x2;   

    private:
        /// Initializes a login to a reX server that is not using a separate authentication server.
       	void PerformXMLRPCLogin(
       	    const char *first_name,
       	    const char *last_name,
       	    const char *password,
       	    const char *address,
       	    int port,
       	    ClientParameters *params);
        
	    /// Handles the UDP communications with the reX server.
	    shared_ptr<NetMessageManager> networkManager_;
	    
	    /// Handles the initial XMLRPC messaging.
	    shared_ptr<PocoXMLRPCConnection> rpcConnection_;
	    
		/// State of the network connection.
		bool bConnected_;
        
        /// Event manager      
        Foundation::EventManagerPtr eventManager_;
        
        /// Network event category for inbound messages.
        Core::event_category_id_t networkEventInCategory_;		
        
        /// Network event category for outbound messages.
        Core::event_category_id_t networkEventOutCategory_;		
    };
}

#endif
