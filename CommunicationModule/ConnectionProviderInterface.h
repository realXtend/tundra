// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ConnectionProviderInterface_h
#define incl_Comm_ConnectionProviderInterface_h

#include <QObject>
#include <QStringList>
#include <QString>

#include "CommunicationModuleFwd.h"

namespace Communication
{
    /**
     *  This class is only used by CommunicationService object. Do not use this
     *  directly!
     *
     * Provides connections to IM servers eg. to jabber server.
     * ConnectionProvide object can support multiple protocols same time.
     * 
     * This class is only used by ConnectionModule and ConnectionService objects.
     */
    class ConnectionProviderInterface : public QObject
    {
        Q_OBJECT
    public:
        virtual ~ConnectionProviderInterface() {};

        //! Provides list of supported protocols by this provider
        virtual QStringList GetSupportedProtocols() const = 0;

        //! Open a new connection to IM server woth given credentials
        virtual ConnectionInterface* OpenConnection(const CredentialsInterface& credentials) = 0;

        //! Provides all Connections objects created by this provider
        virtual ConnectionVector GetConnections() const = 0;

        //! @return true if connection provoder does support given protocol.
        virtual bool SupportProtocol(QString &protocol) const = 0;

    signals:
        void ProtocolListUpdated(const QStringList &protocols);
        void ConnectionOpened(Communication::ConnectionInterface* connection);
        void ConnectionClosed(Communication::ConnectionInterface* connection);
    };

}

#endif

