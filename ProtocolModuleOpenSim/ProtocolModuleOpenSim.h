// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolModuleOpenSim_ProtocolModuleOpenSim_h
#define incl_ProtocolModuleOpenSim_ProtocolModuleOpenSim_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "ProtocolModuleOpenSimApi.h"
#include "OpenSimLoginThread.h"

#include "Interfaces/INetMessageListener.h"
#include "Interfaces/ProtocolModuleInterface.h"
#include "NetworkEvents.h"

#include "CoreThread.h"
#include "RexUUID.h"

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
    class OSPROTO_MODULE_API ProtocolModuleOpenSim  :
        public IModule,
        public ProtocolUtilities::INetMessageListener,
        public ProtocolUtilities::ProtocolModuleInterface
    {
    public:
        ProtocolModuleOpenSim();
        virtual ~ProtocolModuleOpenSim();

        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        /// INetMessageListener override.
        /// Passes inbound network events to listeners.
        virtual void OnNetworkMessageReceived(ProtocolUtilities::NetMsgID msgID, ProtocolUtilities::NetInMessage *msg);

        /// INetMessageListener override.
        /// Passes outbound network events to listeners. Used for stats/debugging.
        virtual void OnNetworkMessageSent(const ProtocolUtilities::NetOutMessage *msg);

        /// Dumps network message to the console.
        void DumpNetworkMessage(ProtocolUtilities::NetMsgID id, ProtocolUtilities::NetInMessage *msg);

        /// Gets the modules loginworker
        /// @return loginworker_
        OpenSimLoginThread* GetLoginWorker() { return &loginWorker_; }

        /// ProtocolModuleInterface override
        virtual void RegisterNetworkEvents();

        /// ProtocolModuleInterface override
        virtual void UnregisterNetworkEvents();

        /// ProtocolModuleInterface override
        virtual bool CreateUdpConnection(const char *address, int port);

        /// ProtocolModuleInterface override
        virtual ProtocolUtilities::Connection::State GetConnectionState() const { return loginWorker_.GetState(); }

        /// ProtocolModuleInterface override
        virtual void SetConnectionState(ProtocolUtilities::Connection::State newstate) { loginWorker_.SetConnectionState(newstate); }
        
        /// ProtocolModuleInterface override
        virtual std::string &GetConnectionErrorMessage() const { return loginWorker_.GetErrorMessage(); }

        /// Returns client parameters of current connection
        virtual const ProtocolUtilities::ClientParameters& GetClientParameters() const { return clientParameters_; }

        /// ProtocolModuleInterface override
        virtual void SetCapability(const std::string &name, const std::string &url);

        /// ProtocolModuleInterface override
        virtual std::string GetCapability(const std::string &name) const;

        /// ProtocolModuleInterface override
        virtual void SetAuthenticationType(ProtocolUtilities::AuthenticationType aType) { authenticationType_ = aType; }

        /// ProtocolModuleInterface override
        virtual bool IsConnected() const { return connected_; }

        /// ProtocolModuleInterface override
        virtual void DisconnectFromServer();

        /// ProtocolModuleInterface override
        virtual ProtocolUtilities::NetOutMessage *StartMessageBuilding(ProtocolUtilities::NetMsgID msgId);

        /// ProtocolModuleInterface override
        virtual void FinishMessageBuilding(ProtocolUtilities::NetOutMessage *msg);

        /// ProtocolModuleInterface override
        virtual ProtocolUtilities::NetMessageManager *GetNetworkMessageManager() const { return networkManager_.get(); }

    private:
        ProtocolModuleOpenSim(const ProtocolModuleOpenSim &);
        void operator=(const ProtocolModuleOpenSim &);

        /// Requests capabilities from the server.
        /// @param seed Seed capability URL.
        void RequestCapabilities(const std::string &seed);

        /// Extracts capabilities from XML string
        /// @param xml XML string from the server.
        void ExtractCapabilitiesFromXml(std::string xml);

        //! Type name of this module.
        static std::string type_name_static_;

        /// Thread for the login process.
        Thread thread_;

        /// Object which handles the XML-RPC login procedure.
        OpenSimLoginThread loginWorker_;

        /// Handles the UDP communications with the reX server.
        boost::shared_ptr<ProtocolUtilities::NetMessageManager> networkManager_;

        /// State of the network connection.
        bool connected_;

        /// Authentication type (Taiga/OpenSim/RealXtend)
        ProtocolUtilities::AuthenticationType authenticationType_;

        /// Event manager.
        Foundation::EventManagerPtr eventManager_;

        /// Network state event category.
        event_category_id_t networkStateEventCategory_;

        /// Network event category for inbound messages.
        event_category_id_t networkEventInCategory_;

        /// Network event category for outbound messages.
        event_category_id_t networkEventOutCategory_;

        /// Current connection client-spesific parameters.
        ProtocolUtilities::ClientParameters clientParameters_;

        /// Server-spesific capabilities.
        CapsMap_t capabilities_;
    };
    /// @}
}


#endif // incl_ProtocolModuleOpenSim_ProtocolModuleOpenSim_h
