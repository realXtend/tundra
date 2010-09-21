// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Connection_h
#define incl_MumbleVoipModule_Connection_h

#include <QObject>
#include <QList>
#include <QMutex>
#include <QMap>
#include <QPair>
#include <QTimer>
#include <QReadWriteLock>
#include "Core.h"
#include "MumbleDefines.h"
#include "StatisticsHandler.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace MumbleClient
{
    class MumbleClient;
    class Channel;
    class User;
}

namespace MumbleVoip
{
    class ServerInfo;
    class PCMAudioFrame;
}

struct CELTMode;
struct CELTEncoder;
struct CELTDecoder;

namespace MumbleLib
{
    class Channel;
    class User;

    typedef QPair<User*, MumbleVoip::PCMAudioFrame*> AudioPacket;

    //! Connection to a single mumble server.
    //!
    //! Do not use this class directly. Only ConnectionManager class is supposed
    //! to use this class.
    //!
    //! This is basically a wrapper over Client class of mumbleclient library.
    //  Mumbleclient library has a main loop whitch calls callback functions in this class
    //! so thread safaty have to be dealed within this class.
    //! 
    //! Connections has Channel and User objects.
    class Connection : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(int playback_buffer_max_length_ms READ GetPlaybackBufferMaxLengthMs WRITE SetPlaybackBufferMaxLengthMs ) 
        Q_PROPERTY(double encoding_quality READ GetEncodingQuality WRITE SetEncodingQuality)
        Q_PROPERTY(bool sending_audio) // \todo implement
        Q_PROPERTY(bool receiving_audio) // \todo implement
        Q_PROPERTY(bool sending_position) // \todo implement
        
    public:
        enum State { STATE_CONNECTING, STATE_AUTHENTICATING, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

        //! Default constructor
        Connection(MumbleVoip::ServerInfo &info, int playback_buffer_length_ms);

        //! Default deconstructor
        virtual ~Connection();

        //! Closes connection to Mumble server
        virtual void Close();

        //! Joins to given channel if channels exist
        //! If authtorization is not completed yet the join request is queuesd
        //! and executed again after successfullu authorization
        //! @todo HANDLE REJOIN
        virtual void Join(QString channel);

        //! Joins to given channel
        //! @todo HANDLE REJOIN
        virtual void Join(const Channel* channel);

        //! @return first <user,audio frame> pair from playback queue
        //!         return <0,0> if playback queue is empty
        //! The caller must delete audio frame object after usage
        virtual AudioPacket GetAudioPacket();

        //! Encode and send given frame to Mumble server
        //! Frame object is NOT deleted by this method 
        virtual void SendAudioFrame(MumbleVoip::PCMAudioFrame* frame, Vector3df users_position);

        //! @return list of channels available
        //! @todo CONSIDER TO USE boost::weak_ptr HERE
        virtual QList<Channel*> ChannelList();

        //! @return channel by id. Return 0 if channel cannot be found
        virtual MumbleLib::Channel* ChannelById(int id);

        //! @param name The full name of the channel e.g "Root/mychannel"
        //! @return channel by name. Return 0 if channel cannot be found
        virtual MumbleLib::Channel* ChannelByName(QString name);

        //! Set audio sending true/false 
        //! @param send true if audio want to be sent to mumble server, otherwise false
        //! @todo rename to EnableAudioSending(bool enable)
        virtual void SendAudio(bool send);

        //! @return true if connection is sending audio, return false otherwise
        virtual bool SendingAudio() const;

        //! Set audio sending true/false 
        //! @param receive true if received audio packets should be handled, false if 
        //!                received audio packets should be ignoered
        //! @todo rename to EnableAudioReceiving(bool enable)
        virtual void ReceiveAudio(bool receive);

        //! \param quality [0.0 .. 1.0] where:
        //!        0.0 means lowest bitrate and worst quality
        //!        1.0 means highest bitrate and best quality.
        virtual void SetEncodingQuality(double quality);

        //! Set position sending on/off
        //! @param send true if position information should be sent with audio data, false otherwise
        //! @see UpdatePosition to set current position to be sent.
        virtual void SendPosition(bool send) { send_position_ = send; }

        //! @return true if position is sent to server with audio data, otherwise false
        virtual bool IsSendingPosition() const { return send_position_; }

        //! @return current state of the connection
        //! @see GetState() to get reason for the state
        virtual State GetState() const;

        //! @return textual description for the reason for current state
        //! @see GetState() to get state
        virtual QString GetReason() const;

    public slots:

        void SetAuthenticated();

        //! @param text Text message received from Mumble server
        void HandleIncomingTextMessage(QString text);

        //! @param length the length of packet in bytes
        //! @param the packet data
        void HandleIncomingRawUdpTunnelPacket(int length, void* buffer);

//        void OnRelayTunnel(std::string &s);

        //! Create a new Channel object if channels doesn't already exist
        void AddChannel(const MumbleClient::Channel& channel);

        //! Remove channel if it already exist
        void RemoveChannel(const MumbleClient::Channel& channel);

        //! Add user if it doesn't already exit
        void CreateUserObject(const MumbleClient::User& user);

        //! Remove user from user list if it exist
        void MarkUserLeft(const MumbleClient::User& user);


        int GetPlaybackBufferMaxLengthMs() { return encoding_quality_; }
        
        //! Set the playback buffer max length for all user object.
        void SetPlaybackBufferMaxLengthMs(int length); // {playback_buffer_length_ms_ = length; }
        
        double GetEncodingQuality() {return encoding_quality_;}

        virtual int GetAverageBandwithIn() const;
        virtual int GetAverageBandwithOut() const;

    private slots:
        void AddToUserList(User* user);
        void HandleIncomingCELTFrame(int session, unsigned char* data, int size);
        void UpdateUserStates();

    private:
        static const int MUMBLE_DEFAULT_PORT_ = 64738;
        static const int AUDIO_BITRATE_MAX_ = 90000; 
        static const int AUDIO_BITRATE_MIN_ = 32000; 
        static const int ENCODE_BUFFER_SIZE_ = 4000;
        static const int USER_STATE_CHECK_TIME_MS = 1000;
        static const int FRAME_BUFFER_SIZE = 256;

        char encoded_frame_data_[MumbleVoip::FRAMES_PER_PACKET][FRAME_BUFFER_SIZE];
        int encoded_frame_length_[MumbleVoip::FRAMES_PER_PACKET];

        void InitializeCELT();
        void UninitializeCELT();
        CELTDecoder* CreateCELTDecoder();
        int BitrateForDecoder();

        State state_;
        QString reason_;
        MumbleClient::MumbleClient* client_;
        QString join_request_; // queued request to join a channel @todo IMPLEMENT BETTER
        QList<MumbleVoip::PCMAudioFrame*> encode_queue_;
        QList<Channel*> channels_; // @todo Use shared ptr
        QMap<int, User*> users_; // maps: session id <-> User object

        CELTMode* celt_mode_;
        CELTEncoder* celt_encoder_;
        CELTDecoder* celt_decoder_;
        MumbleVoip::StatisticsHandler statistics_;

        unsigned char encode_buffer_[ENCODE_BUFFER_SIZE_];
        bool authenticated_;
        bool sending_audio_;
        bool receiving_audio_;
        bool send_position_;
        double encoding_quality_;
        int frame_sequence_;
        QTimer user_update_timer_;
        int playback_buffer_length_ms_;
        
        QMutex mutex_channels_;
        QMutex mutex_authentication_;
        QMutex mutex_encode_queue_;
        QMutex mutex_encoding_quality_;
        QMutex mutex_raw_udp_tunnel_;
        QMutex mutex_client_;
        QMutex mutex_encoder_;
        QReadWriteLock lock_state_;
        QReadWriteLock lock_users_;
        
    signals:
        void StateChanged(MumbleLib::Connection::State state); // \todo register meta data or use int type..
        void TextMessageReceived(QString &text); 
        void AudioDataAvailable(short* data, int size);

        /// emited when user left from server
        void UserLeftFromServer(MumbleLib::User* user);
        /// emited when user join to server
        void UserJoinedToServer(MumbleLib::User* user);

//        void UserJoinedToChannel(User* user);

        void ChannelAdded(Channel* channel); 
        void ChannelRemoved(Channel* channel);

        // private
        void UserObjectCreated(User*);
        void CELTFrameReceived(int session, unsigned char*data, int size);
    };

} // namespace MumbleLib

//Q_DECLARE_METATYPE(MumbleClient::User) // not needed

#endif // incl_MumbleVoipModule_Connection_h
