/** @file
	@author LudoCraft Oy

	Copyright 2009 LudoCraft Oy.
	All rights reserved.

	@brief
*/
#ifndef incl_KristalliProtocolModule_KristalliProtocolModule_h
#define incl_KristalliProtocolModule_KristalliProtocolModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "KristalliProtocolModuleApi.h"
#include "ModuleLoggingFunctions.h"

#include "clb/Network/Network.h"
#include "clb/Time/PolledTimer.h"

namespace KristalliProtocol
{
    struct UserConnection
    {
        UserConnection() :
            id(0),
            connection(0),
            authenticated(false)
        {
        }
        
        u8 id;
        MessageConnection* connection;
        bool authenticated;
        std::string userName;
    };
    
    typedef std::list<UserConnection> UserConnectionList;
    
    //  warning C4275: non dll-interface class 'IMessageHandler' used as base for dll-interface class 'KristalliProtocolModule'
    // Tämän voi ignoroida, koska base classiin ei tarvitse kajota ulkopuolelta - restrukturoin jos/kun on tarvetta.
    class KRISTALLIPROTOCOL_MODULE_API KristalliProtocolModule : public Foundation::ModuleInterface, public IMessageHandler, public INetworkServerListener
    {
    public:
        KristalliProtocolModule();
        ~KristalliProtocolModule();

        void Load();
        void Unload();
        void PreInitialize();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS;

        /// Connects to the Kristalli server at the given address.
        void Connect(const char *ip, unsigned short port, SocketTransportLayer transport);

        void Disconnect();

        /// Starts a Kristalli server at the given port/transport.
        /// \return true if successful
        bool StartServer(unsigned short port, SocketTransportLayer transport);
        
        /// Stops Kristalli server
        void StopServer();
        
        /// Invoked by the Network library for each received network message.
        void HandleMessage(MessageConnection *source, message_id_t id, const char *data, size_t numBytes);

        /// Invoked by the Network library for each new connection
        void NewConnectionEstablished(MessageConnection* source);
        
        /// Invoked by the Network library for disconnected client
        void ClientDisconnected(MessageConnection* source);

        bool Connected() const { return serverConnection != 0; }

        /// @return Module name. Needed for logging.
        static const std::string &NameStatic();

        bool HandleEvent(event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);

        void SubscribeToNetworkEvents();

        /// Return message connection, for use by other modules (null if no connection made)
        MessageConnection* GetMessageConnection() const { return serverConnection; }
        
        /// Return server, for use by other modules (null if not running)
        NetworkServer* GetServer() const { return server; }
        
        /// Return whether we are a server
        bool IsServer() const { return server != 0; }
        
        /// Returns user connections for a server
        UserConnectionList& GetUserConnections() { return connections; }
        
        /// Gets user by message connection. Returns null if no such connection
        UserConnection* GetUserConnection(MessageConnection* source);
        /// Gets user by connection ID. Returns null if no such connection
        UserConnection* GetUserConnection(u8 id);
        
    private:
        /// This timer tracks when we perform the next reconnection attempt when the connection is lost.
        clb::PolledTimer reconnectTimer;

        void PerformConnection();

        /// Allocate a  connection ID for new connection
        u8 AllocateNewConnectionID() const;
        
        /// If true, the connection attempt we've started has not yet been established, but is waiting
        /// for a transition to OK state. When this happens, the MsgLogin message is sent.
        bool connectionPending;
        
        /// This variable stores the server ip address we are desiring to connect to.
        /// This is used to remember where we need to reconnect in case the connection goes down.
        std::string serverIp;
        /// Store the port number we are desiring to connect to. Used for reconnecting
        unsigned short serverPort;
        /// Store the transport type. Used for reconnecting
        SocketTransportLayer serverTransport;

        Network network;
        MessageConnection *serverConnection;
        NetworkServer *server;
        
        /// Users that are connected to server
        UserConnectionList connections;

        event_category_id_t networkEventCategory;
        
        /// Event manager.
        Foundation::EventManagerPtr eventManager_ ;

        /// Id for "NetworkIn" event category.
        event_category_id_t networkInEventCategory_;

        /// Id for "NetworkState" event category.
        event_category_id_t networkStateEventCategory_;
        
        /// Id for "Framework" event category.
        event_category_id_t frameworkEventCategory_;
    };
}

#endif

