#include "StableHeaders.h"
#include "User.h"
#include "MumbleVoipModule.h"
#include "stdint.h"

#include <QTimer>

//#define BUILDING_DLL // for dll import/export declarations
//#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/user.h>
//#undef BUILDING_DLL // for dll import/export declarations

namespace MumbleVoip
{
    User::User(const MumbleClient::User& user) : user_(user), speaking_(false), position_known_(false), position_(0,0,0)
    {
    }

    QString User::Name()
    {
        return QString(user_.name.c_str());
    }
    
    QString User::Hash()
    {
        return QString(user_.hash.c_str());
    }
    
    int User::Session()
    {
        return user_.session;
    }
    
    int User::Id()
    {
        return user_.user_id;
    }
    
    Channel* User::Channel()
    {
        return 0; // \todo: implement
    }
    
    QString User::Comment()
    {
        return QString(user_.comment.c_str());
    }

    bool User::Speaking()
    {
        return speaking_;
    }

    void User::OnAudioFrameReceived()
    {
        if (!speaking_)
            emit StartSpeaking();
        speaking_ = true;

        QTimer::singleShot(SPEAKING_TIMEOUT_MS, this, SLOT(SpeakingTimeout()));
    }

    void User::SpeakingTimeout()
    {
        if (speaking_)
            emit StopSpeaking();
        speaking_ = false;
    }

    void User::UpdatePosition(Vector3df position)
    {
        position_ = position;
    }

    Vector3df User::Position()
    {
        return position_;
    }

} // namespace MumbleVoip
