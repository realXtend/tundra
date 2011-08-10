// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "MumbleFwd.h"
#include "IMumble.h"

namespace MumbleVoip
{
    class Participant : public IParticipant
    {
        
    Q_OBJECT

    public:
        Participant(QString name, MumbleLib::User* user);
        virtual ~Participant();

    public slots:
        virtual QString Name() const;
        virtual void SetName(QString name);

        virtual bool IsSpeaking() const;
        virtual bool IsMuted() const;
        virtual void Mute(bool mute);

        virtual double VoiceActivity() const;
        virtual float3 Position() const;
        
        virtual void Add(MumbleLib::User* user);
        virtual MumbleLib::User* UserPtr() const;       

    private:
        bool muted_;
        bool speaking_;
        bool position_known_;

        float3 position_;
        MumbleLib::User* user_;
        QString name_;
        QString avatar_uuid_;
        double voice_activity_;

    private slots:
        void OnStartSpeaking();
        void OnStopSpeaking();
        void OnPositionUpdated();
        void OnUserLeft();
        void UserObjectDestroyed();

    };
}
