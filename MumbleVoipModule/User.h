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
        //! Default constructor
        User(const MumbleClient::User& user);

        //! Default deconstructor
        virtual ~User();

        //! \return name of user
        virtual QString Name() const;

        //! \return textual comment about user
        virtual QString Comment() const;

//        virtual QString Hash(); // ???

        //! \return session id of the user
        virtual int Session() const;

        //! \return Mumble specific id of the user
        virtual int Id() const;

        //! \return channel of where user is
        virtual Channel* Channel() const;

        //! \return true is user is speaking
        virtual bool IsSpeaking() const;

        //! \return true if the position of the user is known return false if not
        virtual bool PositionKnown()  const { return position_known_;}

        //! \return position od the user
        virtual Vector3df Position() const;

        //! \return length of playback buffer is ms for this user 
        virtual int PlaybackBufferAvailableMs() const;

        //! \return oldest audio frame available for playback 
        //! \note caller must delete audio frame object after usage
        virtual PCMAudioFrame* GetAudioFrame();

        virtual void SetLeft() { left_ = true; emit Left(); }
        virtual bool IsLeft() const { return left_; }

    public slots:
        //! Put audio frame to end of playback buffer 
        //! If playback buffer is full then delete the frame
        //! \param frame Audio frame received from network
        void OnAudioFrameReceived(PCMAudioFrame* frame);

        //! Updatedes user last known position
        //! Also set position_known_ flag up
        //! \param pos the curren position of this user
        void UpdatePosition(Vector3df pos);

    private slots:
        void OnSpeakingTimeout();

    private:
        const MumbleClient::User& user_;
        bool speaking_;
        Vector3df position_;
        bool position_known_;
        static const int SPEAKING_TIMEOUT_MS = 1000;
        static const int PLAYBACK_BUFFER_MS_= 200;
        QList<PCMAudioFrame*> playback_queue_;
        bool left_;

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
