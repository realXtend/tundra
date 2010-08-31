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
    //  warning C4275: non dll-interface class 'IMessageHandler' used as base for dll-interface class 'KristalliProtocol::KristalliProtocolModule'
    // Tämän voi ignoroida, koska base classiin ei tarvitse kajota ulkopuolelta - restrukturoin jos/kun on tarvetta.
    class KRISTALLIPROTOCOL_MODULE_API KristalliProtocolModule : public Foundation::ModuleInterface, public IMessageHandler
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
        void Connect(const char *ip, unsigned short port);

        void Disconnect();

        /// Invoked by the Network library for each received network message.
        void HandleMessage(MessageConnection *source, message_id_t id, const char *data, size_t numBytes);

        bool Connected() const { return serverConnection != 0; }

        /// @return Module name. Needed for logging.
        static const std::string &NameStatic();

        bool HandleEvent(event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);

        void SubscribeToNetworkEvents();

        /// Return message connection, for use by other modules
        MessageConnection* GetMessageConnection() const { return serverConnection; }
        
    private:
        /// This timer tracks when we perform the next reconnection attempt when the connection is lost.
        clb::PolledTimer reconnectTimer;

        void PerformConnection();

        /// If true, the connection attempt we've started has not yet been established, but is waiting
        /// for a transition to OK state. When this happens, the MsgLogin message is sent.
        bool connectionPending;

        /// This variable stores the server ip address we are desiring to connect to.
        /// This is used to remember where we need to reconnect in case the connection goes down.
        std::string serverIp;
        /// The index to the port list where we try to connect to next.
        int nextPortAttempt;

        Network network;
        MessageConnection *serverConnection;

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

