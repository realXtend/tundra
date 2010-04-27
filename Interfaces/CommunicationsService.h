// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_CommunicationsService_h
#define incl_Interfaces_CommunicationsService_h

namespace Communications
{
    class VoiceCallInterface;       // \todo: Define       telepathy
    class VideoCallInterface;       // \todo: Define       telepathy
    class PublicChatSessionInterface;  // \todo: Define       telepathy
    class PrivateChatSessionInterface; // \todo: Define       telepathy

    class ContactInterface;         // \todo: Define       telepathy, opensim
    class ContactListInterface;     // \todo: Define       telepathy, opensim
    class InWorldVoiceInterface;    //                     mumble
    class InWorldTextChat;          //                     opensim


    //! A simple text message
    //!
    class TextMessageInterface
    {
    public:
        // virtual ContactPtr Author() const();
        virtual QString Text() const = 0;
        virtual QDateTime TimeStamo() const 0;
    };

    namespace InWorldChat
    {
        class ParticipantInterface
        {
        private:

        };

        class SessionInterface
        {
            Q_OBJECT
        public:
            virtual void SendTextMessage(const QString &text);

        signals:
            TextMessageReceived(TextMessageInterface message);
        };

    } // InWorldChat

    namespace InWorldVoice
    {
        class ParticipantInterface
        {
            virtual QString AvatarUUID() const = 0; // do we get this always ?
            virtual bool IsSpeaking() const = 0;
            virtual void Mute(bool mute) = 0;
            virtual void Position(vector3df &pos)= 0;

            //! \return true if participant has left 
            virtual bool IsLeft() const = 0;
        signals:
            void StartSpkeaking(); 
            void StopSpeaking();
            void Left();
        };
        typedef shared_ptr<Participant> ParticipantPtr;

        //! In-world voice session for virtual reality environments
        class SessionInterface
        {
        public:
            virtual QString Description() = 0;
            virtual bool IsSendingAudio() = 0;
            virtual bool IsReceivingAudio() = 0;

            virtual void EnableAudioSending() = 0;
            virtual void DisableAudioSending() = 0;
            virtual bool IsAudioSendingEnabled() = 0;
            virtual void EnableAudioReceiving() = 0;
            virtual void DisableAudioReceiving() = 0;
            virtual bool IsAudioReceivingEnabled() = 0;

            virtual QList<ParticipantInterface*> Participants() const = 0;
        signals:
            void ParticipantJoined(Participant* participant);
            void ParticipantLeft(Participant* participant);
            void StartSendingAudio();
            void StopSendingAudio();
            void StartReceivingAudio();
            void StopReceivingAudio();
        };

    } // InWorldVoice

    class ServiceInterface
    {
    public:
        static Service* Instance();

        InWorldVoice::Session* InWorldVoiceSession();
        InWorldChat::Session* InWorldChatSession();

    signals:
        void InWorldVoiceSessionRequest(InWorldVoice::SessionInterface* session);
        void InWorldTextChatRequst(InWorldChat::SessionInterface* session);
        //void PrivateChatRequest(PrivateChat::Session session);
    };

} // Communications

#endif incl_Interfaces_CommunicationsService_h
