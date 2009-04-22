// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_h
#define incl_OpenSimProtocolModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "EventDataInterface.h"

#include "OpenSimProtocolModuleApi.h"
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
    RexUUID agentID;
    RexUUID sessionID;
    RexUUID regionID;
    uint32_t circuitCode;
	std::string sessionHash;
	std::string gridUrl;
	std::string avatarStorageUrl;
};

namespace OpenSimProtocol
{
    ///\todo These (event id's, ClienParameters struct and event data classes) could be moved somewhere else?
    namespace Events
    {
        static const Core::event_id_t EVENT_SERVER_CONNECTED = 0x02;
        static const Core::event_id_t EVENT_SERVER_DISCONNECTED = 0x01;
    }

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
        
    /// Interface for modules.
    class OSPROTO_MODULE_API OpenSimProtocolModule : public Foundation::ModuleInterfaceImpl, public INetMessageListener
    {
    public: 
        OpenSimProtocolModule();
        virtual ~OpenSimProtocolModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update(Core::f64 frametime);
		
		MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        
        //! Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_OpenSimProtocol;
        
        /// Passes inbound network events to listeners.
        virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg);
        
        /// Passes outbound network events to listeners. Used for stats/Debugging.
        virtual void OnNetworkMessageSent(const NetOutMessage *msg);
		
        /**
		 * Connects directly to reX server without authentication procedure. Uses PerformXMLRPC() -function. 
		 * @return true if login was successful false if not. 
		 *
		 */
        
		bool ConnectToRexServer(
			const std::string& first_name,
			const std::string& last_name,
			const std::string& password,
			const std::string& address,
		    int port);
        
		
		/**
		 * Connects to reX server through authentication procedure. Uses PerformXMLRPC() -function. 
		 * 
		 * @param first_name is first part of given username. 
		 * 
		 * @param last_name is second part of given username.
		 *
		 * @param address is world server ip-address (or dns-name?) does not contain port number.
		 *
		 * @param port is a world server port (where connection is done). 
		 *
		 * @param auth_server_address is authentication server ip-address (contains port number). 
		 *
		 * @param auth_login is a login name which will be used to login authentication server. 
		 * 
		 * @return true if login was successfull false if not. 
		 *
		 * */

		bool ConnectUsingAuthenticationServer(const std::string& first_name,
			const std::string& last_name,
			const std::string& password,
			const std::string& address,
			int port,
			const std::string& auth_server_address, 
			const std::string& auth_login);


        /// Disconnects from a reX server.
       	void DisconnectFromRexServer();
        
        /// Dumps network message to the console.
        void DumpNetworkMessage(NetMsgID id, NetInMessage *msg);

        /// Start building a new outbound message.
        /// @return An empty message holder where the message can be built.
        NetOutMessage *StartMessageBuilding(NetMsgID msgId);
        
        /// Finishes (sends) the message. The passed msg pointer will be invalidated after calling this, so don't
        /// access it or hold on to it afterwards. The user doesn't have to do any deallocation, it is all managed by
        /// this class.
        void FinishMessageBuilding(NetOutMessage *msg);
        
        /// Returns client parameters of current connection
        const ClientParameters& GetClientParameters() const { return clientParameters_; }
        
        ///@return True if connection exists.
        bool IsConnected() const { return bConnected_; }
        
    private:
        /**
		 * Perform XMLRPC Login to using authentication server or direct rexserver depending of 
		 * given @p callMethod param and @p authentication param. All data which will be got from authentication of
		 * login into world server is saved in @p ClientParameters struct. 
		 *
		 *
		 * @param first_name is username first part
		 *
		 * @param last_name is username last part (second name)
		 *
		 * @param password is a password which will be used to login into simulator and authentication server.
		 *
		 * @param worldAddress is a address of world (sim) server without port value. 
		 * 
		 * @param worldPort is a port of world (sim) server. 
		 *
		 * @param callMethod is a function which will be "called" through xmlrpc-epi interface. Right now possible values are login_to_simulator 
		 * or ClientAuthentication.
		 *
		 * @param authentication_login is a login name (can be diffrent then a first_name + second_name). 
		 *
		 * @param authentication_address is a address to authentication server without port number. 
		 *
		 * @param authentication_port is a port of authentication server.
		 *
		 * @param authentication is a flag which defines is authentication done. 
		 * 
		 * @return true if login (or authentication) was successful.
		 */
		bool PerformXMLRPCLogin(const std::string& first_name, 
			const std::string& last_name, 
			const std::string& password,
			const std::string& worldAddress,
			const std::string& worldPort,
			const std::string& callMethod,
			const std::string& authentication_login = "",
			const std::string& authentication_address = "",
			const std::string& authentication_port = "",
			bool authentication = false);

	    /// Handles the UDP communications with the reX server.
	    shared_ptr<NetMessageManager> networkManager_;
	    
	    /// Handles the initial XMLRPC messaging.
	    shared_ptr<PocoXMLRPCConnection> rpcConnection_;
	    
		/// State of the network connection.
		bool bConnected_;
        
        /// Event manager.
        Foundation::EventManagerPtr eventManager_;
        
        /// Network state event category.
        Core::event_category_id_t networkStateEventCategory_;
        
        /// Network event category for inbound messages.
        Core::event_category_id_t networkEventInCategory_;		
        
        /// Network event category for outbound messages.
        Core::event_category_id_t networkEventOutCategory_;		
        
        /// Current connection client parameters
        ClientParameters clientParameters_;
    };
}

#endif
