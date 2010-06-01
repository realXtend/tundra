// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/StreamedMediaChannel>
#include "Connection.h"
#include "ChatSession.h"
#include "FriendRequest.h"
#include "OutgoingFriendRequest.h"
#include "VoiceSession.h"
#include "Contact.h"
#include "CredentialsInterface.h"
#include "CoreDefines.h"
#include "CoreException.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    Connection::Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials) : tp_connection_manager_(tp_connection_manager), name_("Gabble"), protocol_("jabber"), state_(STATE_INITIALIZING), friend_list_("friend list"), self_contact_(0), tp_pending_connection_(0)
    {
        CreateTpConnection(credentials);
    }

    Connection::~Connection()
    {
        Close();
    }

    void Connection::DeleteContacts()
    {
        for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
        {
            SAFE_DELETE(*i);
        }
        contacts_.clear();
    }

    void Connection::DeleteFriendRequests()
    {
        for (FriendRequestVector::iterator i = received_friend_requests_.begin(); i != received_friend_requests_.end(); ++i)
        {
            SAFE_DELETE(*i);
        }
        received_friend_requests_.clear();

        for (OutgoingFriendRequestVector::iterator i = sent_friend_requests_.begin(); i != sent_friend_requests_.end(); ++i)
        {
            SAFE_DELETE(*i);
        }
        sent_friend_requests_.clear();
    }

    void Connection::CloseSessions()
    {
        // todo: disconnect any signals left
        for (ChatSessionVector::iterator i = private_chat_sessions_.begin(); i != private_chat_sessions_.end(); ++i)
        {
            (*i)->Close();
            SAFE_DELETE(*i);
        }
        private_chat_sessions_.clear();

        for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
        {
            (*i)->Close();
            SAFE_DELETE(*i);
        }
        public_chat_sessions_.clear();

        for (VoiceSessionVector::iterator i = voice_sessions_.begin(); i != voice_sessions_.end(); ++i)
        {
            VoiceSession* session = *i;
            SAFE_DELETE(session);
        }
        voice_sessions_.clear();
    }

    void Connection::CreateTpConnection(const Communication::CredentialsInterface &credentials)
    {
        QVariantMap params;

        params.insert("account", credentials.GetUserID());
        params.insert("password", credentials.GetPassword());
        params.insert("server", credentials.GetServer());
        params.insert("port", QVariant( (unsigned int)credentials.GetPort() ));
        
        Tp::PendingConnection *pending_connection = tp_connection_manager_->requestConnection(credentials.GetProtocol(), params);
        QObject::connect(pending_connection, SIGNAL( finished(Tp::PendingOperation *) ), SLOT( OnConnectionCreated(Tp::PendingOperation *) ));

        server_ = credentials.GetServer();
        user_id_ = credentials.GetUserID();
    }

    
    QString Connection::GetName() const
    {
        return name_;
    }

    QString Connection::GetProtocol() const
    {
        return protocol_;
    }
    
    Communication::ConnectionInterface::State Connection::GetState() const
    {
        return state_;
    }

    QString Connection::GetServer() const
    {
        return server_;
    }

    QString Connection::GetUserID() const
    {
        return user_id_;
    }

    QString Connection::GetReason() const
    {
        return reason_;
    }

    Communication::ContactGroupInterface& Connection::GetContacts()
    {
        return friend_list_;
    }

    QStringList Connection::GetPresenceStatusOptionsForContact() const
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");

        QStringList options;
        Tp::SimpleStatusSpecMap map = tp_connection_->allowedPresenceStatuses();
        for (Tp::SimpleStatusSpecMap::iterator i = map.begin(); i != map.end(); ++i)
        {
            QString o = i.key();
            options.append(o);
        }
        return options;
    }

    QStringList Connection::GetPresenceStatusOptionsForUser() const
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");

        QStringList options;
        Tp::SimpleStatusSpecMap map = tp_connection_->allowedPresenceStatuses();
        for (Tp::SimpleStatusSpecMap::iterator i = map.begin(); i != map.end(); ++i)
        {
            QString o = i.key();
            if ( o.compare("offline") == 0 || o.compare("unknown") == 0 || o.compare("error") == 0 )
                continue; // HACK: Gabble crash if presence status is set to 'offline', 'unknown' or 'error'
            options.append(o);
        }
        return options;
    }

    Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");

        //! todo check if there is already open chat session with given contact
        //! and return that

        ChatSession* session = new ChatSession(*self_contact_, (Contact&)contact, tp_connection_);
        private_chat_sessions_.push_back(session);
        return session;
    }

    Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString& user_id)
    {
        //! @todo IMPLEMENT
        //! - Request Tp::Contact object from Tp::Connection
        //! - Create ChatSession object and return it
        throw Exception("NOT IMPLEMENTED");
    }

    Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel_id)
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");

        ChatSession* session = new ChatSession(channel_id, tp_connection_);
        public_chat_sessions_.push_back(session);
        return session;
    }

    Communication::VoiceSessionInterface* Connection::OpenVoiceSession(const Communication::ContactInterface &contact)
    {
        VoiceSession* session = new VoiceSession(dynamic_cast<Contact const*>(&contact)->GetTpContact());
        voice_sessions_.push_back(session);
        return session;
    }

    void Connection::RemoveContact(const Communication::ContactInterface &contact)
    {
        const Contact* c =  dynamic_cast<const Contact*>(&contact);
        friend_list_.RemoveContact(c);
        c->GetTpContact()->removePresencePublication();
        c->GetTpContact()->removePresenceSubscription();
        emit ContactRemoved(contact);
    }

    void Connection::SendFriendRequest(const QString &target, const QString &message)
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");
        OutgoingFriendRequest* request = new OutgoingFriendRequest(target, message, tp_connection_);
        sent_friend_requests_.push_back(request);
        connect(request, SIGNAL( Error(OutgoingFriendRequest*) ), SLOT( OnSendingFriendRequestError(OutgoingFriendRequest*) ));

        connect(request, SIGNAL( Accepted(OutgoingFriendRequest*) ), SLOT( OnSendingFriendAccepted(OutgoingFriendRequest*) ));
        connect(request, SIGNAL( Rejected(OutgoingFriendRequest*) ), SLOT( OnSendingFriendRejected(OutgoingFriendRequest*) ));
    }

    Communication::FriendRequestVector Connection::GetFriendRequests() const
    {
        if (state_ != STATE_OPEN)
            throw Exception("Connection is not open.");

        Communication::FriendRequestVector requests;
        for (FriendRequestVector::const_iterator i = received_friend_requests_.begin(); i != received_friend_requests_.end(); ++i)
        {
            requests.push_back(*i);
        }
        return requests;
    }

    void Connection::SetPresenceStatus(const QString &status)
    {
        if (state_ != STATE_OPEN )
            throw Exception("Connection is not open.");

        presence_status_ = status;
        Tp::PendingOperation* op = tp_connection_->setSelfPresence(presence_status_,presence_message_);
        //! @todo Check success
    }

    void Connection::SetPresenceMessage(const QString &message)
    {
        if (state_ != STATE_OPEN )
            throw Exception("Connection is not open.");

        presence_message_ = message;
        Tp::PendingOperation* op = tp_connection_->setSelfPresence(presence_status_,presence_message_);
        //! @todo Check success
    }

    void Connection::Close()
    {
        if ( tp_connection_.isNull() )
            return; // nothing to close

        CloseSessions();
        DeleteFriendRequests();
        DeleteContacts();
        DisconnectSignals();

        Tp::PendingOperation* op = tp_connection_->requestDisconnect();
        tp_connection_.reset();
    }

    void Connection::DisconnectSignals()
    {
        // todo: Check that all signals are disconnected
        disconnect(tp_connection_->requestConnect(),
                         SIGNAL(finished(Tp::PendingOperation *)), this,
                         SLOT(OnConnectionConnected(Tp::PendingOperation *)));

        disconnect(tp_connection_.data(),
                         SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)), this,
                         SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));

        if (tp_pending_connection_)
            disconnect(tp_pending_connection_, SIGNAL(finished(Tp::PendingOperation *)), this, SLOT(OnConnectionReady(Tp::PendingOperation *)));

        disconnect(tp_connection_.data(), SIGNAL( statusChanged(uint , uint) ), this, SLOT( OnConnectionStatusChanged(uint , uint ) ) );
        disconnect(tp_connection_.data(), SIGNAL( selfHandleChanged(uint) ), this, SLOT( OnSelfHandleChanged(uint) ));
    }

    void Connection::OnConnectionCreated(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            QString error_name = op->errorName(); // == Tp::ConnectionStatusReasonAuthenticationFailed)
            state_ = STATE_ERROR;
            reason_ = op->errorMessage();
            emit( ConnectionError(*this) );
            return;
        }
        
        Tp::PendingConnection *c = qobject_cast<Tp::PendingConnection *>(op);
        tp_connection_ = c->connection();

        QObject::connect(tp_connection_->requestConnect(),
                         SIGNAL(finished(Tp::PendingOperation *)),
                         SLOT(OnConnectionConnected(Tp::PendingOperation *)));

        QObject::connect(tp_connection_.data(),
                         SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
                         SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));
    }

    void Connection::OnConnectionConnected(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            reason_ = op->errorMessage();
            LogError(QString("Cannot established connection to IM server: ").append(reason_).toStdString());
            emit( ConnectionError(*this) );
            return;
        }

        LogDebug("Connection to IM server established successfully.");

        QStringList interfaces = tp_connection_->interfaces();
        QString message = "Interfaces (tp_connection): ";
        if (interfaces.size() == 0)
        {
            message.append("  None");
        }
        else
        {
            for(QStringList::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
            {
                message.append(" ");
                message.append(*i);
            }
        }
        LogDebug(message.toStdString());

        Tp::Features features;
        features.insert(Tp::Connection::FeatureCore);
        features.insert(Tp::Connection::FeatureSelfContact);
        features.insert(Tp::Connection::FeatureRoster);
          features.insert(Tp::Connection::FeatureSimplePresence);
        if ( !tp_connection_->isReady(features) )
        {
            QString message = "Establishing connection to IM server. Waiting for these features: ";
            if ( !tp_connection_->isReady(Tp::Connection::FeatureSimplePresence) )
                message.append("SimplePresence");
            if ( !tp_connection_->isReady(Tp::Connection::FeatureRoster) )
                message.append(", Roster");
            if ( !tp_connection_->isReady(Tp::Connection::FeatureSelfContact) )
                message.append(", SelfContact");
            if ( !tp_connection_->isReady(Tp::Connection::FeatureCore) )
                message.append(", Core");
            LogDebug(message.toStdString());
        }   

        if (tp_connection_->interfaces().contains(TELEPATHY_INTERFACE_CONNECTION_INTERFACE_CAPABILITIES))
        {
            Tp::CapabilityPair capability = {
                TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA,
                Tp::ChannelMediaCapabilityAudio |
                Tp::ChannelMediaCapabilityVideo |
                Tp::ChannelMediaCapabilityNATTraversalSTUN |
//                Tp::ChannelMediaCapabilityNATTraversalICEUDP | // added 15.12.2009 mattiku
                Tp::ChannelMediaCapabilityNATTraversalGTalkP2P
            };
        
            tp_connection_->capabilitiesInterface()->AdvertiseCapabilities( Tp::CapabilityPairList() << capability, QStringList());
            LogDebug("Advertising capabilities.");
        }
        else
        {
            LogWarning("Capabilities interface is not supported!");
        }

        if( tp_connection_->interfaces().contains(TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS) )
            QObject::connect(tp_connection_->requestsInterface(), SIGNAL( NewChannels(const Tp::ChannelDetailsList&) ), SLOT( OnNewChannels(const Tp::ChannelDetailsList&) ));
        else
            LogWarning("Requests interface is not supported!");

        tp_pending_connection_ = tp_connection_->becomeReady(features);
        connect(tp_pending_connection_, SIGNAL( finished(Tp::PendingOperation *) ),  SLOT( OnConnectionReady(Tp::PendingOperation *) ));
        connect(tp_connection_.data(), SIGNAL( statusChanged(uint , uint) ), SLOT( OnConnectionStatusChanged(uint , uint ) ));
        connect(tp_connection_.data(), SIGNAL( selfHandleChanged(uint) ), SLOT( OnSelfHandleChanged(uint) ));
    }

    void Connection::OnConnectionReady(Tp::PendingOperation *op)
    {
        tp_pending_connection_ = 0;
        if (state_ == STATE_CLOSED)
            return;

        if (op->isError())
        {
            state_ = STATE_ERROR;
            reason_ = op->errorMessage();
            LogError(QString("Connection establision failed: ").append(reason_).toStdString());
            emit( ConnectionError(*this) );
            return;
        }
        
        if (!tp_connection_->isReady( tp_connection_->requestedFeatures() ))
        {
            state_ = STATE_ERROR;
            
            reason_ = "Cannot get all requested connection features.";
            LogDebug(QString("Login to IM server failed: ").append(reason_).toStdString());
            emit ConnectionError(*this);
            return;
        }
        else
            LogDebug("Login to IM server successed.");

        self_contact_ = new Contact(tp_connection_->selfContact());

        connect(tp_connection_.data(), SIGNAL( statusChanged(uint, uint) ), SLOT( OnTpConnectionStatusChanged(uint, uint) ));
        connect(tp_connection_->contactManager(), SIGNAL( presencePublicationRequested(const Tp::Contacts &) ), SLOT( OnPresencePublicationRequested(const Tp::Contacts &) ));
        connect(tp_connection_->contactManager(), SIGNAL( groupMembersChanged(const QString &, const Tp::Contacts &, const Tp::Contacts &) ), SLOT( OnGroupMembersChanged(const QString &, const Tp::Contacts &, const Tp::Contacts &) ));

        ContactVector new_contacts = HandleAllKnownTpContacts();
        for (ContactVector::iterator i = new_contacts.begin(); i != new_contacts.end(); ++i)
        {
            Contact* c = *i;
            contacts_.push_back(c);
            friend_list_.AddContact(c);
        }

        if (tp_connection_.isNull() || tp_connection_->selfContact().isNull())
        {
            LogError("Selfcontact is NULL");
            presence_status_ = "";
            presence_message_ = "";

            state_ = STATE_ERROR;
            reason_ = "Cannot get all self contact object.";
            LogDebug(QString("Login to IM server failed: ").append(reason_).toStdString());
            emit ConnectionError(*this);
            return;
        }
        else
        {
            presence_status_ = tp_connection_->selfContact()->presenceStatus();
            presence_message_ = tp_connection_->selfContact()->presenceMessage();
        }

        state_ = STATE_OPEN;
        emit( ConnectionReady(*this) );
    }

    void Connection::OnTpConnectionStatusChanged(uint new_status, uint reason)
    {
        // todo: IMPLEMENT
        switch (new_status)
        {
        case Tp::Connection::StatusConnected:
            break;
        case Tp::Connection::StatusUnknown:
            break;
        case Tp::Connection::StatusDisconnected:
            break;
        case Tp::Connection::StatusConnecting:
            break;
        }
    }

    ContactVector Connection::HandleAllKnownTpContacts()
    {
        ContactVector new_contacts;

        //! Check every known telepathy contact and determinate their nature by state of subscribtionState and publistState values
        //! Combinations are:
        //!                                  subscription:   publish:
        //! - Normal contact                 YES             YES
        //! - friend request (sended)        
        //! - friend request (received)      ASK             YES
        //! - banned contact                 NO              *
        //! - unknow                         (all the other combinations)
        
        foreach (const Tp::ContactPtr &tp_contact, tp_connection_->contactManager()->allKnownContacts())
        {
            switch ( tp_contact->subscriptionState() )
            {
            case Tp::Contact::PresenceStateNo:
                // The user doesn't subscribe presence status of this contact

                      switch ( tp_contact->publishState() )
                    {
                    case Tp::Contact::PresenceStateAsk:
                        //! Open situation, we do nothing
                        {
                        FriendRequest* request = new FriendRequest(tp_contact);
                        received_friend_requests_.push_back(request);
                        pending_friend_requests_.append(request);
                        emit FriendRequestReceived(*request);
                        }
                        break;
                    case Tp::Contact::PresenceStateYes:
                        //! Contact have published the presence status 
                        break;
                    case Tp::Contact::PresenceStateNo:
                        //! Contact have denied to publish presence status
                        break;
                    }

                break;

            case Tp::Contact::PresenceStateYes:
                {
                    // A friend list item
                    switch ( tp_contact->publishState() )
                    {
                    case Tp::Contact::PresenceStateNo:
                        {
                        //! We have subsribed presence status of this contact
                        //! but we have not published our own presence!
                        //! -> We unsubsribe this contact
                        tp_contact->removePresenceSubscription();
                        }
                        break;

                    case Tp::Contact::PresenceStateYes:
                        {
                        //! This is a normal friendship state, both the user and the contact have subsribed each others
                        //! presence statuses.
                        Contact* contact = GetContact(tp_contact);
                        if (contact == 0)
                        {
                            //QSet<Tp::Contact::Feature> features;
                            //features << Tp::Contact::FeatureAlias;
                            //features << Tp::Contact::FeatureAvatarToken;
                            //features << Tp::Contact::FeatureSimplePresence;
                            

                            contact = new Contact(tp_contact);
                            new_contacts.push_back(contact);
                        }
                        break;
                        }

                    case Tp::Contact::PresenceStateAsk:
                        //! We have subscribed presence of this contact
                        //! but we don't publish our?
                        //! Publicity level should be same to the both directions
                        Tp::PendingOperation* op = tp_contact->authorizePresencePublication("");
                        connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( PresencePublicationFinished(Tp::PendingOperation*) ));
                        //Contact* contact = GetContact(tp_contact);
                        //if (contact == 0)
                        //{
                        //    contact = new Contact(tp_contact);
                        //    new_contacts.push_back(contact);
                        //}
                        //new_contacts.push_back(&contact);
                        break;
                    }
                }
                break;
                           
            case Tp::Contact::PresenceStateAsk:
                //! The user have not yet decided if (s)he wants to subscribe presence of this contact
                //! 
                {
                      switch ( tp_contact->publishState() )
                    {
                    case Tp::Contact::PresenceStateAsk:
                        //! Open situation, we do nothing
                        break;
                    case Tp::Contact::PresenceStateYes:
                        //! Contact have published the presence status 
                        break;
                    case Tp::Contact::PresenceStateNo:
                        //! Contact have denied to publish presence status
                        break;
                    }

                    // User have not yet made the decision to accept or reject presence subscription 
                    // So we create a FriendRequest obeject
                    FriendRequest* request = new FriendRequest(tp_contact);
                    received_friend_requests_.push_back(request);
                    emit FriendRequestReceived(*request);
                }
                break;
            }
        }

        return new_contacts;
    }

    void Connection::OnNewChannels(const Tp::ChannelDetailsList& channels)
    {
        if (state_ == STATE_CLOSED)
            return;

        foreach (const Tp::ChannelDetails &details, channels) 
        {
            QString channelType = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType")).toString();
            bool requested = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".Requested")).toBool();

            if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT && !requested)
            {
                //! todo check if there is already open chat session with given contact
                //! and return that

                LogDebug("Text chat request received.");
                Tp::TextChannelPtr tp_text_channel = Tp::TextChannel::create(tp_connection_, details.channel.path(), details.properties);
                LogDebug("Text channel object created.");
                
                ChatSession* session = new ChatSession(*self_contact_, tp_text_channel);
                private_chat_sessions_.push_back(session);
                connect(session, SIGNAL( Ready(ChatSession*) ), SLOT( IncomingChatSessionReady(ChatSession*) ));
            }

            if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_CONTACT_LIST && !requested)
            {
                LogDebug("Contact list channel");
                //TextChannelPtr tp_text_channel = Tp::TextChannel::create(tp_connection_, details.channel.path(), details.properties);
            }

            if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA && !requested)
            {
                LogDebug("Voice chat request received.");
                Tp::StreamedMediaChannelPtr tp_streamed_media_channel = Tp::StreamedMediaChannel::create(tp_connection_, details.channel.path(), details.properties);
                
                if ( !tp_streamed_media_channel->initiatorContact().isNull() )
                {
                    Contact* initiator = GetContact(tp_streamed_media_channel->initiatorContact());
                    //! @todo get the actual contact object
                    VoiceSession* session = new VoiceSession(tp_streamed_media_channel);
                    voice_sessions_.push_back(session);
                    //emit( VoiceSessionReceived(*session) );
                    connect(session, SIGNAL( Ready(ChatSession*) ), SLOT( IncomingVoiceSessionReady(VoiceSession*) ));
                }
                else
                {
                    Contact* null_contact = new Contact(tp_streamed_media_channel->initiatorContact()); // we don't have the contact!

                    VoiceSession* session = new VoiceSession(tp_streamed_media_channel);
                    connect(session, SIGNAL( Ready(VoiceSession*) ), SLOT( IncomingVoiceSessionReady(VoiceSession*) ));

                    voice_sessions_.push_back(session);
                    
                    //emit( VoiceSessionReceived(*session) );
                }
            }
        }
    }

    void Connection::OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
    {
        //! We don't have to report about error here.
        //! OnConnectionReady() will be called after this with error information

        state_ = STATE_ERROR;
        reason_ = errorMessage;
        emit( ConnectionError(*this) );
    }

    void Connection::OnConnectionClosed(Tp::PendingOperation *op)
    {
        state_ = STATE_CLOSED;
        emit( ConnectionClosed(*this) );
    }

    void Connection::OnPresencePublicationRequested(const Tp::Contacts &contacts)
    {
        for (Tp::Contacts::const_iterator i = contacts.begin(); i != contacts.end(); ++i)
        {
            Tp::ContactPtr c = *i;
            LogDebug(QString("Presence publication request from ").append(c->id()).toStdString());

            if ( c->subscriptionState() == Tp::Contact::PresenceStateNo )
            {
                FriendRequest* friend_request = new FriendRequest(c);
                connect(friend_request, SIGNAL( Accepted(FriendRequest*) ), SLOT( IncomingFriendRequestAccepted(FriendRequest*) ));
                received_friend_requests_.push_back(friend_request);
                emit FriendRequestReceived(*friend_request);
                return;
            }

            if ( c->subscriptionState() == Tp::Contact::PresenceStateYes && c->publishState() == Tp::Contact::PresenceStateAsk )
            {
                c->authorizePresencePublication();
                Contact* contact = new Contact(c);
                contacts_.push_back(contact);
                friend_list_.AddContact(contact);
                emit FriendRequestAccepted(contact->GetID());
                emit NewContact(*contact);
                return;        
            }

             if ( c->subscriptionState() == Tp::Contact::PresenceStateYes && c->publishState() == Tp::Contact::PresenceStateNo )
             {
                 LogDebug(QString("User have already rejected this contact so we ignore this request.").toStdString());
                 return;
             }
             Tp::Contact::PresenceState state1 = c->subscriptionState();
             Tp::Contact::PresenceState state2 = c->publishState();
        }
    }

    Contact* Connection::GetContact(Tp::ContactPtr tp_contact)
    {
        for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
        {
            Contact* c = *i;
            if (c->GetID().compare(tp_contact->id()) == 0)
                return c;
        }
        return 0;
    }

    void Connection::OnSendingFriendRequestError(OutgoingFriendRequest* request)
    {
        QString message = "Cannot send a friend request to ";
        LogError(message.toStdString());
    }

    void Connection::IncomingChatSessionReady(ChatSession* session)
    {
        emit( ChatSessionReceived(*session) );
    }

    void Connection::IncomingFriendRequestAccepted(FriendRequest *request)
    {
        ContactVector new_contacts = HandleAllKnownTpContacts();
        for (ContactVector::iterator i = new_contacts.begin(); i != new_contacts.end(); ++i)
        {
            Contact* c = *i;
            contacts_.push_back(c);
            friend_list_.AddContact(c);
            emit NewContact(*c);
        }
    }

    void Connection::PresencePublicationFinished(Tp::PendingOperation* op)
    {
        ContactVector new_contacts = HandleAllKnownTpContacts();
        for (ContactVector::iterator i = new_contacts.begin(); i != new_contacts.end(); ++i)
        {
            Contact* c = *i;
            contacts_.push_back(c);
            friend_list_.AddContact(c);
            emit NewContact(*c);
        }
    }

    void Connection::AddContact(Contact* contact)
    {
        contacts_.push_back(contact);
        friend_list_.AddContact(contact);
        connect(contact, SIGNAL( PresenceSubscriptionCanceled(Contact*) ), SLOT( OnPresenceSubscriptionCanceled(Contact* ) ));
    }

    void Connection::OnPresenceSubscriptionCanceled(Contact* contact)
    {
        //! We remove the contact just from friend list 
        //! Actual contact object is deleted at deconstructor of connection object
        friend_list_.RemoveContact(contact);
        emit ContactRemoved(*contact);
    }

    void Connection::IncomingVoiceSessionReady(VoiceSession *session)
    {
        emit( VoiceSessionReceived(*session) );
    }

    void Connection::OnConnectionStatusChanged(uint status, uint reason)
    {
        // TODO: IMPLEMENT
    }

    void Connection::OnSelfHandleChanged(uint handle) 
    {
        // TODO: IMPLEMENT
    }

    void Connection::OnSendingFriendAccepted(OutgoingFriendRequest* request)
    {   
        TelepathyIM::Contact* c = request->GetContact();
        Contact* contact = new Contact(c->GetTpContact());
        for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
        {
            Contact* vc = *i;
            if (vc->GetID().compare(contact->GetID()) == 0)
            {
                return;
            }
        }
        contacts_.push_back(contact);
        emit NewContact(*contact);
    }

    void Connection::OnSendingFriendRejected(OutgoingFriendRequest* request)
    {
        // TODO: IMPLEMENT
    }

    void Connection::OnGroupMembersChanged(const QString &group,
                                           const Tp::Contacts &groupMembersAdded,
                                           const Tp::Contacts &groupMembersRemoved)
    {
        // TODO: IMPLEMENT
    }

    void Connection::CheckPendingFriendRequests()
    {
        foreach (FriendRequest *request, pending_friend_requests_)
            emit FriendRequestReceived(*request);
        pending_friend_requests_.clear();
    }

} // end of namespace: TelepathyIM
