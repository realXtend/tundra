// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_User_h
#define incl_MumbleVoipModule_User_h

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>
#include <Core.h>

namespace MumbleClient
{
    class User;
}

namespace MumbleVoip
{
    class Channel;
    class PCMAudioFrame;

    //! Wrapper over libmumbleclient library's User class
    //! Present mumble client intance on MurMur server
    class User : public QObject, public QMutex
    {
        Q_OBJECT
    public:
        //! Default constructor
        //! @param user
        //! @param channel The channel where the user are located
        User(const MumbleClient::User& user, MumbleVoip::Channel* channel);

        //! Destructor
        virtual ~User();

        //! @return name of the user
        virtual QString Name() const;

        //!@return textual comment about user
        virtual QString Comment() const;

        //! @return session id of the user
        virtual int Session() const;

        //! @return Mumble specific id of the user
        virtual int Id() const;

        //! @return channel of where user is
        virtual Channel* Channel() const;

        //! @return true is user is speaking
        virtual bool IsSpeaking() const;

        //! @return true if the position of the user is known return false if not
        virtual bool PositionKnown() const {return position_known_;}

        //! @return position od the user
        virtual Vector3df Position() const;

        //! @return length of playback buffer is ms for this user 
        virtual int PlaybackBufferLengthMs() const ;

        //! @return oldest audio frame available for playback 
        //! @note caller must delete audio frame object after usage
        virtual PCMAudioFrame* GetAudioFrame();

        //! Set user status to be left
        virtual void SetLeft() { left_ = true; emit Left(); }

        //! @return true if the user has left the channel
        virtual bool IsLeft() const { return left_; }

    public slots:
        //! Put audio frame to end of playback buffer 
        //! If playback buffer is full it is cleared first.
        //! @param farme Audio data frame received from network and ment to be for playback locally
        void AddToPlaybackBuffer(PCMAudioFrame* frame);

        //! Updatedes user last known position
        //! Also set position_known_ flag up
        //! @param pos the curren position of this user
        void UpdatePosition(Vector3df pos);

    private slots:
        void OnSpeakingTimeout();

    private:
        const MumbleClient::User& user_;
        bool speaking_;
        Vector3df position_;
        bool position_known_;
        static const int SPEAKING_TIMEOUT_MS = 1000;
        static const int PLAYBACK_BUFFER_MAX_LENGTH_MS_= 200;
        QList<PCMAudioFrame*> playback_queue_;
        bool left_;
        MumbleVoip::Channel* channel_;

    signals:
        //! Emited when user has left from server
        void Left();

        //! Emited when user starts to speak
        void StartSpeaking();

        //! Emited when user stops speaking
        void StopSpeaking();

        void PositionUpdated();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_User_h
