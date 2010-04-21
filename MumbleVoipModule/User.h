// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_User_h
#define incl_MumbleVoipModule_User_h

#include <QObject>
#include <QString>
#include <QList>
#include <Core.h>
#include <QMutex>

namespace MumbleClient
{
    class User;
}

namespace MumbleVoip
{
    class Channel;
    class PCMAudioFrame;

    //! Wrapper over libmumbleclient library's User class
    class User : public QObject, public QMutex
    {
        Q_OBJECT
    public:
        User(const MumbleClient::User& user);
        virtual ~User();
        virtual QString Name();
        virtual QString Comment();
        virtual QString Hash(); // ???
        virtual int Session();
        virtual int Id();
        virtual Channel* Channel();
        virtual bool IsSpeaking();
        virtual bool PositionKnown() {return position_known_;}
        virtual Vector3df Position();
        virtual double PlaybackBufferAvailableMs();
        virtual PCMAudioFrame* GetAudioFrame();

    public slots:
        void OnAudioFrameReceived(PCMAudioFrame* frame);
        void SpeakingTimeout();
        void UpdatePosition(Vector3df pos);

    private:
        const MumbleClient::User& user_;
        bool speaking_;
        Vector3df position_;
        bool position_known_;
        static const int SPEAKING_TIMEOUT_MS = 1000;
        static const int PLAYBACK_BUFFER_MS_= 200;
        QList<PCMAudioFrame*> playback_queue_;

    signals:
        void Left();
        void StartSpeaking();
        void StopSpeaking();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_User_h
