// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_CommunicationService_h
#define incl_Comm_CommunicationService_h

#include "CommunicationServiceInterface.h"
#include "ModuleLoggingFunctions.h"
#include "CoreTypes.h"
#include <CommunicationsService.h>

#include <QMap>

namespace Foundation
{
    class Framework;
}

class IEventData;

namespace Communication
{

    /**
     *
     *
     */
    class NetworkEventHandlerInterface
    {
    public:
        //! Handle network event
        virtual bool HandleNetworkEvent(IEventData* data) = 0;
        virtual bool HandleNetworkStateEvent(event_id_t event_id, IEventData* data) = 0;
    };

    /**
     *  Implements CommunicationServiceInterface 
     *
     */
    class CommunicationService : public CommunicationServiceInterface
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        //! returns name of this module. Needed for logging.
        static const std::string NameStatic() { return "Communication"; } // for logging functionality
    public:
        explicit CommunicationService(Foundation::Framework* framework);
        ~CommunicationService();
        static void CreateInstance(Foundation::Framework* framework);

        //! Deletes static object instance
        static void CleanUp();

        //! Static method to provide singleton CommunicationServiceInterface object 
        static CommunicationServiceInterface* GetInstance();

        //! Register a ConnectionProvider object to communication service
        //! Without any connection provider communication service cannot provide
        //! any communication protocols.
        virtual void RegisterConnectionProvider( ConnectionProviderInterface* const provider);

        //! Provides list of all currently supported protocols
        virtual QStringList GetSupportedProtocols() const;

        //! Create new Connection object accordingly given credentials
        virtual ConnectionInterface* OpenConnection(const CredentialsInterface &credentials);

        //! Return all Connection objects
        virtual ConnectionVector GetConnections() const;

        //! Provides Connection objects which supports given protocol
        virtual ConnectionVector GetConnections(const QString &protocol) const;

        //! Handle events
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        virtual Foundation::Framework* GetFramework() { return framework_; };

    protected:
        ConnectionProviderVector GetConnectionProviders(const QString &protocol) const;

        Foundation::Framework* framework_;
        static CommunicationService* instance_;
        ConnectionProviderVector connection_providers_;
        ConnectionVector connections_;
        QMap<QString, ConnectionVector> connections_per_protocol_;
        QStringList supported_protocols_;

        //! category id for incoming messages
        event_category_id_t event_category_networkinin_;
        event_category_id_t event_category_networkstate_;
    protected slots:
        virtual void OnProtocolListUpdated(const QStringList& protocols);
        virtual void OnConnectionOpened(Communication::ConnectionInterface*);
        virtual void OnConnectionClosed(Communication::ConnectionInterface*);
        virtual void OnFriendRequestReceived(Communication::FriendRequestInterface& request);
    };
} // end of namespace: Communication

#endif // incl_Comm_CommunicationService_h
