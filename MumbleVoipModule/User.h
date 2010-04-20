// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_User_h
#define incl_MumbleVoipModule_User_h

#include <QObject>
#include <QString>

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
    private:
        const MumbleClient::User& user_;
    signals:
        void Left();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_User_h
