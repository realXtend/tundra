// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_CommunicationServiceInterface_h
#define incl_Comm_CommunicationServiceInterface_h

#include <QObject>
#include <QString>

#include "CommunicationModuleFwd.h"

namespace Communication
{

    /**
     * Singleton class which provides connections to IM servers using registered connection providers
     * To get IM connection: 1) setup a credential object 
     *                       2) call OpenConnection
     *
     */
    class CommunicationServiceInterface : public QObject
    {
        Q_OBJECT
    public:
        virtual ~CommunicationServiceInterface() {};

        //! Static method to provide singleton CommunicationServiceInterface object 
        static CommunicationServiceInterface* GetInstance();

        //! Register a ConnectionProvider object to communication service
        //! Without any connection provider communication service cannot provide
        //! any communication protocols.
        //! Note: ConnectionProvider objects are deleted by CommunicationService object!
        //!
        virtual void RegisterConnectionProvider( ConnectionProviderInterface* const provider) = 0;

        //! Provides list of all currently supported protocols
        //! by all registered connection prividers
        virtual QStringList GetSupportedProtocols() const = 0;

        //! Create new Connection object accordingly given credentials
        virtual ConnectionInterface* OpenConnection(const CredentialsInterface &credentials) = 0;

        //! Return all Connection objects
        virtual ConnectionVector GetConnections() const = 0;

        //! Provides Connection objects which supports given protocol
        virtual ConnectionVector GetConnections(const QString &protocol) const = 0;
    signals:
        //! When a new protocol is supported
        void NewProtocolSupported(QString &protocol);

        //! When support for a protocol is ened becouse ConnectionProvider object is unregistered or have problems etc.
        void ProtocolSupportEnded(QString &protocol);

        void ProtocolListUpdated(const QStringList &protocols);

        //! When a new connection is opened
        void ConnectionOpened(Communication::ConnectionInterface* connection);

        //! When connection is closed
        void ConnectionClosed(Communication::ConnectionInterface* connection);
    };
}

#endif

