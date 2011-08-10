// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "Math/float3.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDateTime>

namespace MumbleVoip
{
    // IParticipant

    class IParticipant : public QObject
    {
        
    Q_OBJECT

    public:
        virtual ~IParticipant() {};

    public slots:
        virtual QString Name() const = 0;
        virtual bool IsSpeaking() const = 0;
        virtual void Mute(bool mute) = 0;
        virtual bool IsMuted() const = 0;
        virtual float3 Position() const = 0;
        virtual double VoiceActivity() const = 0;
    
    signals:
        void StartSpeaking(); 
        void StopSpeaking();
        void StateChanged();
        void VoiceActivityChanged();
        void Left(); //! @todo remove

    };

    typedef QList<MumbleVoip::IParticipant*> ParticipantList;

    // IParticipant

    class ISession;

    class IProvider : public QObject
    {

    Q_OBJECT
    
    public:
        virtual ~IProvider() {};

    public slots:
        virtual MumbleVoip::ISession* Session() = 0;
        virtual bool HasSession() = 0;
        virtual QString& Description() = 0;
    
    signals:
        void SessionAvailable();
        void SessionUnavailable();
    };

    // ISession

    class ISession : public QObject
    {
        
    Q_OBJECT
    Q_ENUMS(State)

    public:
        enum State 
        {
            STATE_CLOSED, 
            STATE_INITIALIZING, 
            STATE_OPEN, 
            STATE_ERROR
        };

        virtual ~ISession() {};

    public slots:

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
        virtual int GetAverageBandwithIn() const = 0;
        virtual int GetAverageBandwithOut() const = 0;
        virtual QString GetActiveChannel() const = 0;
        virtual void SetActiveChannel(QString channel) = 0;
        virtual QStringList GetChannels() = 0;
        virtual ParticipantList Participants() const = 0;
        virtual QStringList GetParticipantsNames() const = 0;
        virtual void MuteParticipantByName(QString, bool) const = 0;

    signals:
        void StateChanged(MumbleVoip::ISession::State state);
        void ParticipantJoined(MumbleVoip::IParticipant* participant);
        void ParticipantLeft(MumbleVoip::IParticipant* participant);
        void StartSendingAudio();
        void StopSendingAudio();
        void StartReceivingAudio();
        void StopReceivingAudio();
        void SpeakerVoiceActivityChanged(double volume);
        void ActiceChannelChanged(QString new_channel);
        void ChannelListChanged(QStringList list);
    };
}
