// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_h
#define incl_OpenSimProtocolModule_h

#include "Foundation.h"
#include "ModuleInterface.h"

#include "OpenSimProtocolModuleApi.h"
#include "NetMessageManager.h"
#include "NetworkConnection.h"
#include "INetMessageListener.h"
#include "OpenSimLoginThread.h"
#include "RexUUID.h"
#include "NetworkEvents.h"

namespace OpenSimProtocol
{
    /** \defgroup OpenSimProtocolClient OpenSimProtocol Client Interface
        This page lists the public interface of the OpenSimProtocol module. Use
        this module to track the server connection state as well as to 
        communicate with the server using the SLUDP protocol.

        For an introduction to how to work with this module, see
        \ref OpenSimProtocolConnection "Managing the OpenSim connection state"
        and \ref SLUDP "Interfacing with the OpenSim world using SLUDP messages."

        @{
    */

    /// OpenSimProtocolModule exposes other modules with the funtionality of
    /// communicating with the OpenSim server using the SLUDP protocol. It
    /// also handles the XMLRPC handshakes with the server.
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

        /// Passes outbound network events to listeners. Used for stats/debugging.
        virtual void OnNetworkMessageSent(const NetOutMessage *msg);

        /**
         * Logs in to a reX server without the authentication procedure.
         * 
         * @param first_name is first part of given username. 
         * @param last_name is second part of given username.
         * @param address is world server ip-address (or dns-name?) does not contain port number.
         * @param port is a world server port (where connection is done). 
         * @return true if login was successfull false if not. 
         *
         * */
        void LoginToServer(
            const std::string& first_name,
            const std::string& last_name,
            const std::string& password,
            const std::string& address,
            int port,
            ConnectionThreadState *thread_state);

        /// Cable Beach style login, setting authentication done unlike normal LoginToServer
        void LoginToCBServer(
            const std::string& first_name,
            const std::string& last_name,
			const std::string& identityUrl,
            const std::string& address,
            int port,
            ConnectionThreadState *thread_state);

        /**
         * Logs in to a reX server using the authentication procedure.
         * 
         * @param first_name is first part of given username. 
         * @param last_name is second part of given username.
         * @param address is world server ip-address (or dns-name?) does not contain port number.
         * @param port is a world server port (where connection is done). 
         * @param auth_server_address is authentication server ip-address (contains port number). 
         * @param auth_login is a login name which will be used to login authentication server. 
         * @return true if login was successfull false if not. 
         *
         * */

        bool LoginUsingRexAuthentication(const std::string& first_name,
            const std::string& last_name,
            const std::string& password,
            const std::string& address,
            int port,
            const std::string& auth_server_address, 
            const std::string& auth_login,
            ConnectionThreadState *thread_state);

        /// Creates the UDP connection to the server.
        ///@ return True, if the connection was succesfull, false otherwise.
        bool CreateUDPConnection(const char *address, int port);

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

        /// Sets new capability.
        /// @param name Name of capability.
        /// @param url URL of the capability.
        void SetCapability(const std::string &name, const std::string &url);

        /// Returns URL of the requested capability, or null string if the capability doens't exist.
        /// @param name Name of the capability.
        /// @return Capability URL.
        std::string GetCapability(const std::string &name);

        ///@return True if connection exists.
        bool IsConnected() const { return connected_; }

        ///@return Connection::State enum of the connection state.
        Connection::State GetConnectionState() const { return loginWorker_.GetState(); }

    private:
        /// Requests capabilities from the server.
        /// @param seed Seed capability URL.
        void RequestCapabilities(const std::string &seed);

        /// Extracts capabilities from XML string
        /// @param xml XML string from the server.
        void ExtractCapabilitiesFromXml(std::string xml);

        /// Thread for the login process.
        Core::Thread thread_;

        /// Object which handles the XML-RPC login procedure.
        OpenSimLoginThread loginWorker_;

        /// Handles the UDP communications with the reX server.
        boost::shared_ptr<NetMessageManager> networkManager_;

        /// State of the network connection.
        bool connected_;

        /// Authentication type (Taiga/OpenSim/RealXtend)
        AuthenticationType authenticationType_;

		/// Identity URL
		std::string identityUrl_;

		std::string hostUrl_;

        /// Event manager.
        Foundation::EventManagerPtr eventManager_;

        /// Network state event category.
        Core::event_category_id_t networkStateEventCategory_;

        /// Network event category for inbound messages.
        Core::event_category_id_t networkEventInCategory_;

        /// Network event category for outbound messages.
        Core::event_category_id_t networkEventOutCategory_;

        /// Current connection client-spesific parameters.
        ClientParameters clientParameters_;

        ///Typedefs for capability map.
        typedef std::map<std::string, std::string> caps_map_t;
        typedef std::map<std::string, std::string>::iterator caps_map_it_t;

        /// Server-spesific capabilities.
        caps_map_t capabilities_;
    };

    /// @}
}


#endif
