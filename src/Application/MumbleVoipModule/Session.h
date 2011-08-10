// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "MumbleFwd.h"
#include "IMumble.h"
#include "AudioAPI.h"
#include <QMap>

namespace MumbleVoip
{
    /**
       Presents a voice session with one or more real connections to mumble servers.
       Initial state is STATE_CLOSED.
     */
    class Session : public ISession
    {
        
    Q_OBJECT

    public:
        Session(Framework* framework, Settings* settings);
        virtual ~Session();

    public slots:
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
        virtual int GetAverageBandwithIn() const;
        virtual int GetAverageBandwithOut() const;

        virtual QString GetActiveChannel() const;
        virtual void SetActiveChannel(QString channel);
        virtual QStringList GetChannels();

        virtual ParticipantList Participants() const;
        virtual QStringList GetParticipantsNames() const;
        virtual void MuteParticipantByName(QString, bool) const;

        virtual void Update(f64 frametime);
        virtual QList<QString> Statistics();
        virtual QString GetServerInfo() const;
        virtual void AddChannel(QString name, const ServerInfo &server_info);
        virtual void AddChannel(QString name, QString username, QString server, QString port, QString password, QString version, QString channelIdBase);
        virtual void RemoveChannel(QString name);

        bool GetPositionalAudioEnabled() const;
        void EnablePositionalAudio(bool enable);
        void SetPosition(float3 position);

    private:
        static const int AUDIO_RECORDING_BUFFER_MS = 200;
        static const double DEFAULT_AUDIO_QUALITY_; // 0 .. 1.0

        virtual void OpenConnection(ServerInfo info);
        QString OwnAvatarId();
        QString GetAvatarFullName(QString uuid) const;
        void SendRecordedAudio();
        void PlaybackReceivedAudio();
        void PlaybackAudioFrame(MumbleLib::User* user, PCMAudioFrame* frame);
        void ApplyMicrophoneLevel(PCMAudioFrame* frame);
        //virtual void AddChannel(EC_VoiceChannel* channel);
        //virtual void RemoveChannel(EC_VoiceChannel* channel);
        void ClearParticipantList();
        void PopulateParticipantList();

        Framework* framework_;
        State state_;
        QString reason_;
        QString description_;
        bool sending_audio_;
        bool receiving_audio_;
        bool audio_sending_enabled_;
        bool audio_receiving_enabled_;
        QList<Participant*> participants_;
        QList<MumbleLib::User*> other_channel_users_;
        MumbleLib::Connection* connection_; // // In future session could have multiple connections
        double speaker_voice_activity_;
        QString current_mumble_channel_;
        QMap<int, SoundChannelPtr> audio_playback_channels_;
        std::string recording_device_;
        Settings* settings_;
        bool local_echo_mode_; // if true then audio is only played locally
        QString server_address_;
        QString active_channel_;
        QMap<QString, ServerInfo> channels_;
        float3 user_position_;
        int reconnect_timeout_; // how long to wait before trying to reconnect (msecs)

    private slots:
        void CreateNewParticipant(MumbleLib::User*);
        void UpdateParticipantList();
        void OnUserStartSpeaking(); // rename to: UpdateReceivingAudioStatus
        void OnUserStopSpeaking(); // rename to: UpdateReceivingAudioStatus
        void UpdateSpeakerActivity(PCMAudioFrame*);
        void CheckChannel(MumbleLib::User*);
        void CheckConnectionState();
        void SetPlaybackBufferSizeMs(int);
        void SetEncodeQuality(double);
        void Reconnect();
    };

    typedef boost::shared_ptr<Session> SessionPtr;
}
