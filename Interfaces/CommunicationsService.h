// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_CommunicationsService_h
#define incl_Interfaces_CommunicationsService_h

#include "ServiceInterface.h"
// todo: Make these forward declarations if possible
#include <QObject>
#include <Vector3D.h>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QDateTime>
//class QString;
//class QDateTime;

namespace Communications
{
    //! Naali Communications Framework 
    //! ==============================
    //!
    //! - Communications module implements CommunicationsService interface.
    //! - Modules can implement and registe their communication providers to CommunicationsService
    //! - Modules can request communications features from CommunicationsService
    //!
    //! Communication types
    //! - Private chat ..... Text message based communication between 2..N people
    //! - Public chat ...... Text message based communication on public channel
    //! - Voice call ....... Voice call between 2..N people
    //! - Video call ....... Video and audio call between 2..N people
    //! - In-world voice ... Spatial Voice communication in virtual reality 
    //! - In-world chat .... Spatial text message based communication
    //!  
    //! IM Services
    //! Connections to IM services can be opened and closed with credentials. Open connections
    //! can offer contact list with presence support. User can open only one connection per
    //! IM type such as irc and jabber.
    //! 
    //! - Contact list ..... List of contacts from certain IM service. 
    //!                      Support presence features (contact presences and self presence)
    //! - Contact .......... IM contact
    //! - Credendials ...... Used for login to IM services
    //!
    //!
    //!

    class TextMessageInterface;
    // namespace: IM
    class VoiceCallInterface;           // \todo: Define       telepathy
    class VideoCallInterface;           // \todo: Define       telepathy
    class PublicChatSessionInterface;   // \todo: Define       telepathy
    class PrivateChatSessionInterface;  // \todo: Define       telepathy
    class ContactInterface;             // \todo: Define       telepathy, opensim
    class ContactListInterface;         // \todo: Define       telepathy, opensim
    // namespace: InWorldVoice
  //  class InWorldVoiceInterface;        //                     mumble
    // namespace: InWorldChat
//    class InWorldTextChat;              //                     opensim

    //!
    class TextMessageInterface
    {
    public:
        virtual const QString& Author() const = 0;
        virtual const QString& Text() const = 0;
        virtual const QDateTime& TimeStamp() const = 0;
        virtual bool IsOwnMessage() const = 0;
//        virtual QList<QByteArray> Attachments() const = 0;
    };

    namespace IM
    {
    //    class CredentialInterface
    //    {
    //
    //
    //    };
    //
    //    class FriendRequestInterface
    //    {
    //    public:
    //        virtual QString From() const = 0;
    //        virtual QString Message() const = 0;
    //        virtual QDateTime TimeStamp() const = 0;
    //    };

    //    class ContactInterface
    //    {
    //    public:
    //        virtual QString FullName() const = 0;
    //        virtual QString ShortName() const = 0;
    //        virtual QString Alias() const = 0;
    //        virtual void SetAlias(QString alias) const = 0;
    //        virtual void Remove() = 0;
    //    signals:
    //        void PresenceChanged();
    //        void FriendshipTerminated();
    //    };

    //    class ContactListInterface : QObject
    //    {
    //        Q_OBJECT
    //    public:
    //        virtual ~ContactListInterface() {};
    //        virtual ContactInterface& SelfContact() = 0;
    //        virtual void SetPresenceStatus(QString status) = 0;
    //        virtual void SetPresenceText(QString status) = 0;
    //        virtual QString PresenceStatus() const = 0;
    //        virtual QString PresenceText() const = 0;
    //        virtual void SendFriendRequest(QString address, QString message) = 0;
    //    signals:
    //        void FriendRequest(FriendRequestInterface& request);
    //    };

        //! (TEST IDEA...)
        //! 'Connection'
        //class ServiceInterface : public QObject
        //{
        //    Q_OBJECT
        //public:
        //    virtual QString Description() const = 0;
        //    virtual void Close() = 0;
        //    //virtual ContactList ContactList() = 0;
        //signals:
        //    void StateChanged();
        //    void BecameUnavailable();
        //};

    } // IM

    //! In-world chat in virtual reality systems
    //!
    //! There can be up to one in-world chat session intance per virtual world session.
    //! This chat session can be obtained by calling InWorldChatSession method from 
    //! CommunicationsService interface.
    //!
    //! \todo Define way to inform about session becames inavailable
    //!
    namespace InWorldChat
    {
        //! Participant of in-world chat session. Participant list must be request from
        //! session object 
        //!
        class ParticipantInterface
        {
        public:
            virtual ~ParticipantInterface() {};
            virtual const QString &Name() const = 0;
            virtual QString AvatarUUID() const = 0;
            virtual Vector3df Position() const = 0; // needed ???
        };
        typedef QList<ParticipantInterface*> ParticipantList;

        class TextMessageInterface : public Communications::TextMessageInterface
        {
        public:
//            virtual const ParticipantInterface& AuthorParticipant() const = 0;
        };


        /// @todo : ??? enum ChatEvent { TextMessageReceived, TextMessageSent, ParticipantLeft, ParticipantJoined, ... }
        class SessionInterface : public QObject
        {
            Q_OBJECT
        public:
            /// Default destructor
            virtual ~SessionInterface() {};

            /// Send given text to in-world chat
            virtual void SendTextMessage(const QString &text) = 0;

            /// @return List of pointers to text messages received in this session.
            ///         The list items do not change only new messages are appended during
            ///         the session.
            virtual QList<TextMessageInterface*> MessageHistory() const = 0;

            virtual bool IsClosed() const = 0;

//            virtual ParticipantList Participants() const = 0;
        signals:
            /// When text message is received from in-world chat. The origin of the message 
            /// can be self or other participant
            void TextMessageReceived(const Communications::InWorldChat::TextMessageInterface &message);

            /// When user closes world connections
            void Closed();
            //void ParticipantJoined(ParticipantInterface* participant);
            //void ParticipantLeft(ParticipantInterface* participant);
        };

        /// Provider in-world chat sessions
        class ProviderInterface : public QObject
        {
            Q_OBJECT
        public:
            /// destructor
            virtual ~ProviderInterface() {};

            /// @return session object, return 0 if the session doesn't exist
            virtual Communications::InWorldChat::SessionInterface* Session() const = 0;

            /// Provider description
            virtual QString Description() const = 0;
        signals:
            void SessionAvailable();
            void SessionUnavailable();
        };

    } // InWorldChat

    namespace InWorldVoice
    {
        class ParticipantInterface : public QObject
        {
            Q_OBJECT
        public:
            virtual ~ParticipantInterface() {};
            virtual QString Name() const = 0;
            virtual QString AvatarUUID() const = 0;
            virtual bool IsSpeaking() const = 0;
            virtual void Mute(bool mute) = 0;
            virtual bool IsMuted() const = 0;
            virtual Vector3df Position() const = 0;
//            virtual bool IsLeft() const = 0;

            //! \return true if participant has left 
//            virtual bool IsLeft() const = 0; // \todo better name for method
        signals:
            void StartSpeaking(); 
            void StopSpeaking();
            void Left(); //! @todo remove
            void StateChanged();
//            void PositionUpdated();
        };
        typedef QList<ParticipantInterface*> ParticipantList;
//        typedef shared_ptr<ParticipantInterface> ParticipantPtr;

        //! In-world voice session for virtual reality environments
        //!
        //! \todo Design channel system ???
        //!       - List channels (names, positional audio, listen/speak)
        class SessionInterface : public QObject
        {
            Q_OBJECT
        public:
            enum State {STATE_CLOSED, STATE_INITIALIZING, STATE_OPEN, STATE_ERROR};

            virtual ~SessionInterface() {};
            virtual QString Description() const = 0;

            virtual void Close() = 0;
            virtual State GetState() const = 0;
            virtual QString Reason() const = 0;
            virtual bool IsSendingAudio() const = 0;
            virtual bool IsReceivingAudio() const = 0;
            virtual void EnableAudioSending() = 0;
            virtual void DisableAudioSending() = 0;
            virtual bool IsAudioSendingEnabled() const = 0;
            virtual void EnableAudioReceiving() = 0;
            virtual void DisableAudioReceiving() = 0;
            virtual bool IsAudioReceivingEnabled() const = 0;
            virtual double SpeakerVoiceActivity() const = 0;

            //virtual void SetSelfPosition(const vector3df& pos) = 0;

            //! \todo: Give weak_ptr instead
            virtual QList<Communications::InWorldVoice::ParticipantInterface*> Participants() const = 0;

        signals:
            void StateChanged(Communications::InWorldVoice::SessionInterface::State state);
            void ParticipantJoined(Communications::InWorldVoice::ParticipantInterface* participant);
            void ParticipantLeft(Communications::InWorldVoice::ParticipantInterface* participant);
            void StartSendingAudio();
            void StopSendingAudio();
            void StartReceivingAudio();
            void StopReceivingAudio();
            void SpeakerVoiceActivityChanged(double volume);
//            void ReceivedVoiceActivity(double volume);
        };

        //! Provider in-world voice sessions
        //!
        class ProviderInterface : public QObject
        {
            Q_OBJECT
        public:
            virtual ~ProviderInterface() {};
            virtual Communications::InWorldVoice::SessionInterface* Session() = 0;
            virtual QString& Description() = 0;
        signals:
            void SessionAvailable();
            void SessionUnavailable();
        };

    } // InWorldVoice

    //! Provides all communication methods to rest of the viewer. 
    //!
    //! Communication feature implementations must register/unregister the functionality they provide.
    //! 
    //! Consumers should connect to *Available signals and then request services when they came available.
    //! The services are considired to be available until viewer shutdown. (???)
    //!
    //! \todo Define how to signal about service becomes unavailable.
    class ServiceInterface : public QObject, public Foundation::ServiceInterface 
    {
        Q_OBJECT
    public:
        
        virtual ~ServiceInterface() {};
        static ServiceInterface* Instance();

        ///! \todo use WeakPtr instead ?
        virtual InWorldVoice::SessionInterface* InWorldVoiceSession() const = 0;

        ///! \todo use WeakPtr instead ?
        virtual InWorldChat::SessionInterface* InWorldChatSession() const = 0;
//        virtual IM::ContactList* ContactList(QString type) const = 0;

        //! Registrationd methods for communication providers
        virtual void Register(InWorldVoice::ProviderInterface& provider) = 0;
        //virtual void Unregister(InWorldVoice::ProviderInterface& provider) = 0;
        virtual void Register(InWorldChat::ProviderInterface& provider) = 0;
        //virtual void Unregister(InWorldChat::ProviderInterface& provider) = 0;

        //virtual void LoginToIMService(IM::Credentials credentials) = 0;
        //virtual void LogoutFromIMService(IM::Credentials credentials) = 0;
        //virtual IM:ServiceList IMServices() const = 0;

    signals:
        void InWorldVoiceAvailable();
        void InWorldVoiceUnvailable();
        void InWorldChatAvailable();
        void InWorldChatUnavailable();
        //void PrivateChatRequest(PrivateChat::Session session);
        //void VoiceCallRequest(VoiceCall call);
        //void VideoCallRqeust(VideoCall call);
    };
    typedef boost::shared_ptr<ServiceInterface> ServicePtr;

} // Communications

#endif // incl_Interfaces_CommunicationsService_h
