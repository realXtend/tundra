// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_User_h
#define incl_MumbleVoipModule_User_h

#include <QObject>
#include <QString>
#include <Core.h>

namespace MumbleClient
{
    class User;
}

namespace MumbleVoip
{
    class Channel;

    //! Wrapper over libmumbleclient library's User class
    class User : public QObject
    {
        Q_OBJECT
    public:
        User(const MumbleClient::User& user);
        QString Name();
        QString Comment();
        QString Hash(); // ???
        int Session();
        int Id();
        Channel* Channel();
        bool Speaking();
        bool PositionKnown() {return position_known_;}
        bool Position(float &x, float &y, float &z);
        Vector3df Position();

    public slots:
        void OnAudioFrameReceived();
        void SpeakingTimeout();
        void UpdatePosition(Vector3df pos);

    private:
        const MumbleClient::User& user_;
        bool speaking_;
        Vector3df position_;
        bool position_known_;
        static const int SPEAKING_TIMEOUT_MS = 1000;

    signals:
        void Left();
        void StartSpeaking();
        void StopSpeaking();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_User_h
