// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KristalliProtocolModule_KristalliProtocolModule_h
#define incl_KristalliProtocolModule_KristalliProtocolModule_h

#include "Foundation.h"
#include "IModule.h"
#include "KristalliProtocolModuleApi.h"
#include "ModuleLoggingFunctions.h"
#include "UserConnection.h"

#include "kNet.h"

#include <QList>
#include <QMap>
#include <QMutableMapIterator>

namespace KristalliProtocol
{
    //  warning C4275: non dll-interface class 'IMessageHandler' used as base for dll-interface class 'KristalliProtocolModule'
    // Tämän voi ignoroida, koska base classiin ei tarvitse kajota ulkopuolelta - restrukturoin jos/kun on tarvetta.
    class KRISTALLIPROTOCOL_MODULE_API KristalliProtocolModule : public IModule, public kNet::IMessageHandler, public kNet::INetworkServerListener
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

#ifdef KNET_USE_QT
        ConsoleCommandResult OpenKNetLogWindow(const StringVector &);
#endif

        /// Connects to the Kristalli server at the given address.
        void Connect(const char *ip, unsigned short port, kNet::SocketTransportLayer transport);

        void Disconnect();

        // Multiconnection disconnect. Takes bool and connection number as a parameter. If bool is false user
        // disconnected at will and connection properties should be removed immediately.
        void Disconnect(bool fail, unsigned short con);

        /// Starts a Kristalli server at the given port/transport.
        /// \return true if successful
        bool StartServer(unsigned short port, kNet::SocketTransportLayer transport);
        
        /// Stops Kristalli server
        void StopServer();
        
        /// Invoked by the Network library for each received network message.
        void HandleMessage(kNet::MessageConnection *source, kNet::message_id_t id, const char *data, size_t numBytes);

        /// Invoked by the Network library for each new connection
        void NewConnectionEstablished(kNet::MessageConnection* source);
        
        /// Invoked by the Network library for disconnected client
        void ClientDisconnected(kNet::MessageConnection* source);

        bool Connected() const { return serverConnection != 0; }

        /// @return Module name. Needed for logging.
        static const std::string &NameStatic();

        bool HandleEvent(event_category_id_t category_id,
            event_id_t event_id, 
            IEventData* data);

        void SubscribeToNetworkEvents();

        /// Return message connection, for use by other modules (null if no connection made)
        kNet::MessageConnection *GetMessageConnection(const unsigned short connection) { return serverConnection_map_[connection].ptr(); }

        // Returns iterator to serverConnection_map_
        QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> GetConnectionArray() { return QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> (serverConnection_map_); }

        /// Return server, for use by other modules (null if not running)
        kNet::NetworkServer* GetServer() const { return server; }
        
        /// Return whether we are a server
        bool IsServer() const { return server != 0; }
        
        /// Returns all user connections for a server
        UserConnectionList& GetUserConnections() { return connections; }
        
        /// Gets user by message connection. Returns null if no such connection
        UserConnection* GetUserConnection(kNet::MessageConnection* source);
        /// Gets user by connection ID. Returns null if no such connection
        UserConnection* GetUserConnection(u8 id);

        /// What trasport layer to use. Read on startup from --protocol udp/tcp. Defaults to TCP if no start param was given.
        kNet::SocketTransportLayer defaultTransport;
        
    private:
        /// This variable stores the server ip address we are desiring to connect to.
        /// This is used to remember where we need to reconnect in case the connection goes down.
        std::string serverIp;

        /// Store the port number we are desiring to connect to. Used for reconnecting
        unsigned short serverPort;

        /// Store the transport type. Used for reconnecting
        kNet::SocketTransportLayer serverTransport;

        /// Amount of retries remaining for reconnection. Is low for the initial connection, higher for reconnection
        int reconnectAttempts;

        /// This timer tracks when we perform the next reconnection attempt when the connection is lost.
        kNet::PolledTimer reconnectTimer;

        /// Messageconnection to server
        Ptr(kNet::MessageConnection) serverConnection;

        /// If true, the connection attempt we've started has not yet been established, but is waiting
        /// for a transition to OK state. When this happens, the MsgLogin message is sent.
        bool connectionPending;

        void PerformConnection();

        /// Allocate a  connection ID for new connection
        u8 AllocateNewConnectionID() const;
        
        kNet::Network network;

        kNet::NetworkServer *server;
        
        /// Users that are connected to server
        UserConnectionList connections;

        event_category_id_t networkEventCategory;
        
        /// Event manager.
        EventManagerPtr eventManager_ ;

        /// Id for "NetworkIn" event category.
        event_category_id_t networkInEventCategory_;

        /// Id for "NetworkState" event category.
        event_category_id_t networkStateEventCategory_;
        
        /// Id for "Framework" event category.
        event_category_id_t frameworkEventCategory_;

        /// Messageconnection properties array: IP
        QMap<unsigned short, std::string> serverIp_list_;

        /// Messageconnection properties array: Port
        QMap<unsigned short, unsigned short> serverPort_list_;

        /// Messageconnection properties array: serverTransport
        QMap<unsigned short, kNet::SocketTransportLayer> serverTransport_list_;

        /// Messageconnections properties array: reconnectAttempts
        QMap<unsigned short, int> reconnectAttempts_list_;

        /// Messageconnections properties array: Timers
        QMap<unsigned short, kNet::PolledTimer> reconnectTimer_list_;

        /// Messageconnections properties array: Messageconnections
        QMap< unsigned short, Ptr(kNet::MessageConnection) > serverConnection_map_;

        // Flag for connection cleanup. True when last item from connection array is being updated.
        bool cleanup;

        // List for connections to be cleaned up.
        QList<unsigned short> cleanupList;

        // update function for array of connections
        void connectionArrayUpdate();

        // This iterates through connection properties and removes items marked for deletion in cleanupList
        void removeConnectionProperties();

        // This method is pretty much like original PerformConnection-method but it uses iterator
        // to access current processed connection. Higher powers decide if this is wise.
        void PerformReconnection(QMutableMapIterator<unsigned short, Ptr(kNet::MessageConnection)> &, unsigned short);

    };
}

#endif

