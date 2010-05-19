// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Session_h
#define incl_MumbleVoipModule_Session_h

#include "CommunicationsService.h"
#include "SoundServiceInterface.h"
#include <QMap>

namespace Foundation
{
    class Framework;
}

namespace MumbleClient
{
    class MumbleClientLib;
}

namespace MumbleVoip
{
    class ServerInfo;
    class User;
    class PCMAudioFrame;
    class Connection;

    namespace InWorldVoice
    {
        class Participant;
        typedef QList<Participant*> ParticipantList;

        class Session : public Communications::InWorldVoice::SessionInterface
        {
            Q_OBJECT
        public:
            Session(Foundation::Framework* framework, const ServerInfo &server_info);
            virtual ~Session();

            virtual void Close();
            virtual State GetState() const;
            virtual QString Reason() const;

            virtual QString Description() const;
            virtual bool IsSendingAudio() const;
            virtual bool IsReceivingAudio() const;

            virtual void EnableAudioSending();
            virtual void DisableAudioSending();
            virtual bool IsAudioSendingEnabled() const;
            virtual void EnableAudioReceiving();
            virtual void DisableAudioReceiving();
            virtual bool IsAudioReceivingEnabled() const;
            virtual double SpeakerVoiceActivity() const;

            virtual QList<Communications::InWorldVoice::ParticipantInterface*> Participants() const;

            virtual void Update(f64 frametime);

        private:
            static const int AUDIO_RECORDING_BUFFER_MS = 200;
            static double AUDIO_QUALITY_; // 0 .. 1.0

            virtual void OpenConnection(ServerInfo info);
            bool GetOwnAvatarPosition(Vector3df& position, Vector3df& direction);
            QString OwnAvatarId();
            QString GetAvatarFullName(QString uuid) const;
            void SendRecordedAudio();
            void PlaybackReceivedAudio();
            void PlaybackAudioFrame(User* user, PCMAudioFrame* frame);
            boost::shared_ptr<Foundation::SoundServiceInterface> SoundService();
    
            Foundation::Framework* framework_;
            State state_;
            QString reason_;
            QString description_;
            bool sending_audio_;
            bool receiving_audio_;
            bool audio_sending_enabled_;
            bool audio_receiving_enabled_;
            ParticipantList participants_;
            ParticipantList left_participants_;
            QList<User*> other_channel_users_;
            Connection* connection_; // // In future session could have multiple connections
            double speaker_voice_activity_;
            const ServerInfo &server_info_;
            User* self_user_;
            QString channel_name_;
            QMap<int, sound_id_t> audio_playback_channels_;
            std::string recording_device_;

        private slots:
            void CreateNewParticipant(User*);
            void UpdateParticipantList();
            void OnUserStartSpeaking(); // rename to: UpdateReceivingAudioStatus
            void OnUserStopSpeaking(); // rename to: UpdateReceivingAudioStatus
            void UpdateSpeakerActivity(PCMAudioFrame*);
            void CheckChannel(User*);
        };

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Session_h
