// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolModuleTaiga_ProtocolModuleTaiga_h
#define incl_ProtocolModuleTaiga_ProtocolModuleTaiga_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "ProtocolModuleTaigaApi.h"
#include "TaigaLoginThread.h"

#include "NetworkMessages/NetMessageManager.h"
#include "Interfaces/INetMessageListener.h"
#include "Interfaces/ProtocolModuleInterface.h"
#include "NetworkEvents.h"
#include "NetworkConnection.h"

#include "RexUUID.h"
#include "CoreThread.h"

namespace TaigaProtocol
{
    /** \defgroup TaigaProtocolClient TaigaProtocol Client Interface
        This page lists the public interface of the TaigaProtocol module. Use
        this module to track the server connection state as well as to 
        communicate with the server using the SLUDP protocol.

        For an introduction to how to work with this module, see
        \ref TaigaProtocolConnection "Managing the OpenSim connection state"
        and \ref SLUDP "Interfacing with the OpenSim world using SLUDP messages."

        @{
    */

    /// TaigaProtocolModule exposes other modules with the funtionality of
    /// communicating with the OpenSim server using the SLUDP protocol. It
    /// also handles the XMLRPC handshakes with the server.
    class TAIGAPROTO_MODULE_API ProtocolModuleTaiga 
        : public IModule, 
          public ProtocolUtilities::INetMessageListener, 
          public ProtocolUtilities::ProtocolModuleInterface
    {
    public: 
        ProtocolModuleTaiga();
        virtual ~ProtocolModuleTaiga();

        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        /// Passes inbound network events to listeners.
        virtual void OnNetworkMessageReceived(ProtocolUtilities::NetMsgID msgID, ProtocolUtilities::NetInMessage *msg);

        /// Passes outbound network events to listeners. Used for stats/debugging.
        virtual void OnNetworkMessageSent(const ProtocolUtilities::NetOutMessage *msg);

        /// Dumps network message to the console.
        void DumpNetworkMessage(ProtocolUtilities::NetMsgID id, ProtocolUtilities::NetInMessage *msg);

        /// Gets the modules loginworker
        /// @return loginworker_
        TaigaLoginThread* GetLoginWorker() { return &loginWorker_; }

        //! Function for registering network event
        virtual void RegisterNetworkEvents();
        
        //! Function for uniregistering networking
        virtual void UnregisterNetworkEvents();

        /// Creates the UDP connection to the server.
        ///@ return True, if the connection was succesfull, false otherwise.
        virtual bool CreateUdpConnection(const char *address, int port);

        ///@return Connection::State enum of the connection state.
        virtual ProtocolUtilities::Connection::State GetConnectionState() const { return loginWorker_.GetState(); }

        /// Set new state
        virtual void SetConnectionState(ProtocolUtilities::Connection::State newstate) { loginWorker_.SetConnectionState(newstate); }
        
        ///@return Connection::State enum of the connection state.
        virtual std::string &GetConnectionErrorMessage() const { return loginWorker_.GetErrorMessage(); }
       
        /// Returns client parameters of current connection
        virtual const ProtocolUtilities::ClientParameters& GetClientParameters() const { return clientParameters_; }

        /// Sets new capability.
        /// @param name Name of capability.
        /// @param url URL of the capability.
        virtual void SetCapability(const std::string &name, const std::string &url);

        /// Returns URL of the requested capability, or null string if the capability doens't exist.
        /// @param name Name of the capability.
        /// @return Capability URL.
        virtual std::string GetCapability(const std::string &name) const;
        
        /// Sets Authentication type
        /// @params authentivation type ProtocolUtilities::AuthenticationType
        virtual void SetAuthenticationType(ProtocolUtilities::AuthenticationType aType) { authenticationType_ = aType; }

        /// Set Identity Url
        /// @param new identity url as std::string
        virtual void SetIdentityUrl(const QString &newUrl) { identityUrl_ = newUrl.toStdString(); }

        /// Set Host Url
        /// @param new host url as std::string
        virtual void SetHostUrl(const QString &newUrl) { hostUrl_ = newUrl.toStdString(); }

        ///@return True if connection exists.
        virtual bool IsConnected() const { return connected_; }

        /// Disconnects from a reX server.
        virtual void DisconnectFromServer();
        
        /// Start building a new outbound message.
        /// @return An empty message holder where the message can be built.
        virtual ProtocolUtilities::NetOutMessage *StartMessageBuilding(ProtocolUtilities::NetMsgID msgId);

        /// Finishes (sends) the message. The passed msg pointer will be invalidated after calling this, so don't
        /// access it or hold on to it afterwards. The user doesn't have to do any deallocation, it is all managed by
        /// this class.
        virtual void FinishMessageBuilding(ProtocolUtilities::NetOutMessage *msg);

        virtual ProtocolUtilities::NetMessageManager *GetNetworkMessageManager() const { return networkManager_.get(); }

    private:
        /// Requests capabilities from the server.
        /// @param seed Seed capability URL.
        void RequestCapabilities(const std::string &seed);

        /// Extracts capabilities from XML string
        /// @param xml XML string from the server.
        void ExtractCapabilitiesFromXml(std::string xml);

        //! Returns type of this module. Needed for logging.
        static std::string type_name_static_;

        /// Thread for the login process.
        Thread thread_;

        /// Object which handles the XML-RPC login procedure.
        TaigaLoginThread loginWorker_;

        /// Handles the UDP communications with the reX server.
        boost::shared_ptr<ProtocolUtilities::NetMessageManager> networkManager_;

        /// State of the network connection.
        bool connected_;

        /// Authentication type (Taiga/OpenSim/RealXtend)
        ProtocolUtilities::AuthenticationType authenticationType_;

        /// Identity url for inventory event
        std::string identityUrl_;

        /// Host url for inventory event
        std::string hostUrl_;

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


#endif
