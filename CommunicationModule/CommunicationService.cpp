// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CommunicationService.h"
#include <QList>
#include "NetworkEvents.h"
#include "EventManager.h"
#include "ConnectionProviderInterface.h"
#include "CredentialsInterface.h"
#include "ConnectionInterface.h"
#include "FriendRequestInterface.h"
#include "Framework.h"
#include "CoreException.h"

#include "MemoryLeakCheck.h"

namespace Communication
{
    CommunicationService* CommunicationService::instance_ =  0; // static member function initialization

    CommunicationService::CommunicationService(Foundation::Framework* framework) : framework_(framework)
    {
        if (framework_ == NULL)
            throw Exception("Cannot create communication service object, framework pointer is missing."); 

        event_category_networkinin_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
    }

    CommunicationService::~CommunicationService()
    {
        for (ConnectionProviderVector::iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
        {
            Communication::ConnectionProviderInterface* provider = *i;
            SAFE_DELETE(provider);
        }
        connection_providers_.clear();
    }

    // static
    void CommunicationService::CreateInstance(Foundation::Framework* framework)
    {
        if (CommunicationService::instance_ == NULL)
            CommunicationService::instance_ = new CommunicationService(framework);
    }

    // static
    void CommunicationService::CleanUp()
    {
        SAFE_DELETE(CommunicationService::instance_);
    }

    //! static
    CommunicationServiceInterface* CommunicationService::GetInstance()
    {
        if (CommunicationService::instance_ == 0)
            throw Exception("CommunicationService is not initialized properly.");
        return CommunicationService::instance_;
    }

    void CommunicationService::RegisterConnectionProvider( ConnectionProviderInterface* const provider )
    {
        connection_providers_.push_back(provider);
        connect( provider, SIGNAL( ProtocolListUpdated(const QStringList&) ), SLOT( OnProtocolListUpdated(const QStringList&) ));
        QObject::connect( provider, SIGNAL( ConnectionOpened(Communication::ConnectionInterface*) ), this, SLOT( OnConnectionOpened(Communication::ConnectionInterface*) ));
        QObject::connect( provider, SIGNAL( ConnectionClosed(Communication::ConnectionInterface*) ), this, SLOT( OnConnectionClosed(Communication::ConnectionInterface*) ));
    }

    QStringList CommunicationService::GetSupportedProtocols() const
    {
        QStringList all_protocols;
        for (ConnectionProviderVector::const_iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
        {
            QStringList protocols = (*i)->GetSupportedProtocols();
            for (QStringList::const_iterator ii = protocols.begin(); ii != protocols.end(); ++ii)
            {
                QString protocol = (*ii);
                if ( !all_protocols.contains(protocol) )
                    all_protocols.append(protocol);
            }
        }
        return all_protocols;
    }

    Communication::ConnectionInterface* CommunicationService::OpenConnection(const CredentialsInterface &credentials)
    {
        QString protocol = credentials.GetProtocol();
        //! @todo Check if connection already exist with given credentials 
        ConnectionProviderVector providers = GetConnectionProviders(protocol);
        if (providers.size() == 0)
        {
            throw Exception("Cannot open connection, protocol is not supported");
        }
        Communication::ConnectionInterface* connection = providers[0]->OpenConnection(credentials);
        return connection;
    }
    
    ConnectionVector CommunicationService::GetConnections() const
    {
        Communication::ConnectionVector connections;
        for (ConnectionVector::const_iterator i = connections_.begin(); i != connections_.end(); ++i)
        {
            Communication::ConnectionInterface* connection = *i;
            if (connection->GetState() == Communication::ConnectionInterface::STATE_OPEN)
                connections.push_back(connection);
        }
        return connections;
    }

    ConnectionVector CommunicationService::GetConnections(const QString &protocol) const
    {
        ConnectionVector connections;
        for (ConnectionVector::const_iterator i = connections_.begin(); i != connections_.end(); ++i)
        {
            ConnectionInterface* connection = *i;
            if ( connection->GetProtocol().compare( protocol ) == 0 )
                connections.push_back( connection );
        }
        return connections;
    }

    ConnectionProviderVector CommunicationService::GetConnectionProviders(const QString &protocol) const
    {
        ConnectionProviderVector providers;
        for (ConnectionProviderVector::const_iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
        {
            ConnectionProviderInterface* connection = *i;
            QStringList protocols = connection->GetSupportedProtocols();
            if ( protocols.contains(protocol) )
                providers.push_back(connection);
        }
        return providers;
    }

    bool CommunicationService::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        event_category_networkinin_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");

        for (ConnectionProviderVector::iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
        {
            NetworkEventHandlerInterface* handler = dynamic_cast<NetworkEventHandlerInterface*>( *i );
            if ( handler )
            {
                if ( category_id == event_category_networkinin_ )
                {
                    bool ret = handler->HandleNetworkEvent( data );
                    if ( ret )
                        return true;
                }
                if ( category_id == event_category_networkstate_ )
                {
                    bool ret = handler->HandleNetworkStateEvent( event_id, data );
                    if ( ret )
                        return true;
                }
            }
            return false;
        }
        return false;
    }

    void CommunicationService::OnProtocolListUpdated(const QStringList& protocols)
    {
        for (QStringList::const_iterator i = protocols.begin(); i != protocols.end(); ++i)
        {
            QString protocol = *i;
            if ( !supported_protocols_.contains(protocol) )
            {
                supported_protocols_.append(protocol);
                emit( NewProtocolSupported(protocol) );
            }
    }

        for (QStringList::iterator i = supported_protocols_.begin(); i != supported_protocols_.end(); ++i)
        {
            QString protocol;
            bool protocol_supported = false;
               for (ConnectionProviderVector::iterator p = connection_providers_.begin(); p != connection_providers_.end(); ++p)
            {
                Communication::ConnectionProviderInterface* provider = *p;
                if (provider->SupportProtocol(protocol))
                {
                    protocol_supported = true;
                    break;
                }
            }
            if (!protocol_supported)
            {
                supported_protocols_.erase(i); // Check this!
                emit (ProtocolSupportEnded(protocol) );
            }
        }
        
        QString empty("");
        emit ( ProtocolSupportEnded(empty));
    }

    void CommunicationService::OnConnectionOpened(Communication::ConnectionInterface* connection)
    {
        connections_.push_back(connection);
        //! @todo connect to vital singals of the connection
        connect(connection, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), SLOT(OnFriendRequestReceived(Communication::FriendRequestInterface&) )); 
        emit( ConnectionOpened(connection) );
    }

    void CommunicationService::OnConnectionClosed(Communication::ConnectionInterface* connection)
    {
        emit( ConnectionClosed(connection) );
    }

    void CommunicationService::OnFriendRequestReceived(Communication::FriendRequestInterface& request)
    {
        QString message = "Friend request from ";
        message.append( request.GetOriginatorName() );
        LogDebug(message.toStdString());
    }

} // end of namespace: Communication

namespace Communications
{
    Service* Service::instance_ = 0;

    Service::Service() :
        in_world_voice_provider_(0),
        in_world_chat_provider_(0)
    {

    }

    Service::~Service()
    {

    }

    ServiceInterface* Service::Instance()
    {
        if (!Service::instance_)
            Service::instance_ = new Service();
        return 0; // test
//        return Service::instance_;
    }

    InWorldVoice::SessionInterface* Service::InWorldVoiceSession() const
    {
        if (!in_world_voice_provider_)
            return 0;
        else
            return in_world_voice_provider_->Session();
    }

    InWorldChat::SessionInterface* Service::InWorldChatSession() const
    {
        if (!in_world_chat_provider_)
            return 0;
        else
            return in_world_chat_provider_->Session();
    }

    void Service::Register(InWorldVoice::ProviderInterface& provider)
    {
        in_world_voice_provider_ = &provider;
    }

    void Service::Register(InWorldChat::ProviderInterface& provider)
    {
        in_world_chat_provider_ = &provider;
    }

} // Communications
