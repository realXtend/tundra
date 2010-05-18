// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Connection.h"
#include "MumbleVoipModule.h"
#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client.h>
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL // for dll import/export declarations
#include <mumbleclient/settings.h>
#include <mumbleclient/PacketDataStream.h>
#include <mumbleclient/channel.h>
#include <mumbleclient/user.h>
#include "SoundServiceInterface.h"
#include "Channel.h"
#include "User.h"
#include "PCMAudioFrame.h"
#include <QUrl>
#include <celt/celt_types.h>
#include <celt/celt.h>
#include "MemoryLeakCheck.h"
#include <QMetaType>

namespace MumbleVoip
{
    // \todo Move these static callback functions to separeate file...

    void TextMessageCallback(const std::string& message, Connection* connection)
    {
        connection->HandleIncomingTextMessage(QString(message.c_str()));
    }

    void RawUdpTunnelCallback(int32_t length, void* buffer, Connection* connection)
    {
        connection->HandleIncomingRawUdpTunnelPacket(length, buffer);
    }

    void RelayTunnelCallback(int32_t length, void* buffer_, Connection* connection)
    {
     //   PacketDataStream data_stream = PacketDataStream((char*)buffer_, length);
     //   bool valid = data_stream.isValid();
     //   int size = data_stream.size();
     //   char* buffer = (char*)buffer_;
     //   short type = buffer[0] << 8 + buffer[1];
     //   int len = buffer[3] << 16 + buffer[4] << 8 + buffer[5];
	    //std::string s(static_cast<char *>(buffer), length);
	    //s.erase(1, pds_int_len(&static_cast<char *>(buffer)[1]));
    //    connection->OnRelayTunnel(s);
    }

    void AuthCallback(Connection* connection)
    {
        connection->SetAuthenticated();
    }

    void ChannelAddCallback(const MumbleClient::Channel& channel, Connection* connection)
    {
        connection->AddChannel(channel);
    }

    void ChannelRemoveCallback(const MumbleClient::Channel& channel, Connection* connection)
    {
        connection->RemoveChannel(channel);
    }

    void UserJoinedCallback(const MumbleClient::User& user, Connection* connection)
    {
        connection->CreateUserObject(user);
    }

    void UserLeftCallback(const MumbleClient::User& user, Connection* connection)
    {
        connection->MarkUserLeft(user);
    }

    Connection::Connection(ServerInfo &info) :
            client_(0),
            authenticated_(false),
            celt_mode_(0),
            celt_encoder_(0),
            celt_decoder_(0),
            sending_audio_(false),
            receiving_audio_(true),
            frame_sequence_(0),
            encoding_quality_(0),
            state_(STATE_CONNECTING),
            send_position_(false)
    {
        // BlockingQueuedConnection for cross thread signaling
        QObject::connect(this, SIGNAL(UserObjectCreated(User*)), SLOT(AddToUserList(User*)), Qt::ConnectionType::BlockingQueuedConnection);
        QObject::connect(this, SIGNAL(CELTFrameReceived(int, unsigned char*, int)), SLOT(HandleIncomingCELTFrame(int, unsigned char*, int)), Qt::ConnectionType::QueuedConnection);

        InitializeCELT();

        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        client_ = mumble_lib->NewClient();

        QUrl server_url(QString("mumble://%1").arg(info.server));

        QString port = QString::number(server_url.port(64738)); // default port name
        QString server = server_url.host();

        // \todo Handle connection error
	    client_->SetRawUdpTunnelCallback( boost::bind(&RawUdpTunnelCallback, _1, _2, this));
        client_->SetChannelAddCallback(boost::bind(&ChannelAddCallback, _1, this));
        client_->SetChannelRemoveCallback(boost::bind(&ChannelRemoveCallback, _1, this));
        client_->SetTextMessageCallback(boost::bind(&TextMessageCallback, _1, this));
        client_->SetAuthCallback(boost::bind(&AuthCallback, this));
        client_->SetUserJoinedCallback(boost::bind(&UserJoinedCallback, _1, this));
        client_->SetUserLeftCallback(boost::bind(&UserLeftCallback, _1, this));
        try
        {
            client_->Connect(MumbleClient::Settings(server.toStdString(), port.toStdString(), info.user_name.toStdString(), info.password.toStdString()));
        }
        catch(std::exception &e)
        {
            state_ = STATE_ERROR;
            reason_ = QString(e.what());
            return;
        }
        state_ = STATE_AUTHENTICATING;
        emit StateChanged(state_);
    }

    Connection::~Connection()
    {
        QMutexLocker locker1(&mutex_raw_udp_tunnel_);
        QMutexLocker locker2(&mutex_send_audio_);
        QMutexLocker locker3(&mutex_channels_);
        QMutexLocker locker4(&mutex_users_);

        this->disconnect();

        Close();
        UninitializeCELT();
        
        while (encode_queue_.size() > 0)
        {
            PCMAudioFrame* frame = encode_queue_.takeFirst();
            SAFE_DELETE(frame);
        }
        while (channels_.size() > 0)
        {
            Channel* c = channels_.takeFirst();
            SAFE_DELETE(c);
        }

        foreach(User* u, users_)
        {
            SAFE_DELETE(u);
        }
        users_.clear();
        SAFE_DELETE(client_);
    }

    Connection::State Connection::GetState() const
    {
        return state_;
    }

    QString Connection::GetReason() const
    { 
        return reason_;
    }

    void Connection::Close()
    {
        if (state_ != STATE_CLOSED)
        {
            client_->SetRawUdpTunnelCallback(0);
            client_->SetChannelAddCallback(0);
            client_->SetChannelRemoveCallback(0);
            client_->SetTextMessageCallback(0);
            client_->SetAuthCallback(0);
            client_->SetUserJoinedCallback(0);
            client_->SetUserLeftCallback(0);

            client_->Disconnect();
            state_ = STATE_CLOSED;
            emit StateChanged(state_);
        }
    }

    void Connection::InitializeCELT()
    {
        int error = 0;
        celt_mode_ = celt_mode_create(SAMPLE_RATE, SAMPLES_IN_FRAME, &error );
        if (error != 0)
        {
            QString message = QString("CELT initialization failed, error code = %1").arg(error);
            MumbleVoipModule::LogWarning(message.toStdString());
            state_ = STATE_ERROR;
            emit StateChanged(state_);
            return;
        }

        celt_encoder_ = celt_encoder_create(celt_mode_,NUMBER_OF_CHANNELS, NULL );
        if (!celt_encoder_)
        {
            QString message = QString("Cannot create CELT encoder");
            MumbleVoipModule::LogWarning(message.toStdString());
            state_ = STATE_ERROR;
            emit StateChanged(state_);
            return;
        }
        celt_encoder_ctl(celt_encoder_, CELT_SET_PREDICTION(0));
	    celt_encoder_ctl(celt_encoder_, CELT_SET_VBR_RATE(AudioQuality()));

        celt_decoder_ = CreateCELTDecoder();

        MumbleVoipModule::LogDebug("CELT initialized.");
    }

    void Connection::UninitializeCELT()
    {
        celt_encoder_destroy(celt_encoder_);
        celt_encoder_ = 0;
        celt_decoder_destroy(celt_decoder_);
        celt_mode_destroy(celt_mode_);
        MumbleVoipModule::LogDebug("CELT uninitialized.");
    }

    CELTDecoder* Connection::CreateCELTDecoder()
    {
        int error = 0;
        CELTDecoder* decoder = celt_decoder_create(celt_mode_,NUMBER_OF_CHANNELS, &error);
        switch (error)
        {
        case CELT_OK:
           return decoder;
        case CELT_BAD_ARG:
            MumbleVoipModule::LogError("Cannot create CELT decoder: CELT_BAD_ARG");
            return 0;
        case CELT_INVALID_MODE:
            MumbleVoipModule::LogError("Cannot create CELT decoder: CELT_INVALID_MODE");
            return 0;
        case CELT_INTERNAL_ERROR:
            MumbleVoipModule::LogError("Cannot create CELT decoder: CELT_INTERNAL_ERROR");
            return 0;
        case CELT_UNIMPLEMENTED:
            MumbleVoipModule::LogError("Cannot create CELT decoder: CELT_UNIMPLEMENTED");
            return 0;
        case CELT_ALLOC_FAIL:
            MumbleVoipModule::LogError("Cannot create CELT decoder: CELT_ALLOC_FAIL");
            return 0;
        default:
            MumbleVoipModule::LogError("Cannot create CELT decoder: unknow reason");
            return 0;
        }
    }

    void Connection::Join(QString channel_name)
    {
        QMutexLocker locker1(&mutex_authentication_);
        QMutexLocker locker2(&mutex_channels_);

        if (!authenticated_)
        {
            join_request_ = channel_name;
            return; 
        }

        foreach(Channel* channel, channels_)
        {
            if (channel->FullName() == channel_name)
            {
                Join(channel);
            }
        }
    }

    void Connection::Join(const Channel* channel)
    {
        client_->JoinChannel(channel->Id());
    }

    AudioPacket Connection::GetAudioPacket()
    {
        QMutexLocker userlist_locker(&mutex_users_);

        foreach(User* user, users_)
        {
            if (!user->tryLock())
                continue;

            PCMAudioFrame* frame = user->GetAudioFrame();
            if (frame)
            {
                user->unlock();
                return AudioPacket(user, frame);
            }

            user->unlock();
        }

        return AudioPacket(0,0);
    }

    void Connection::SendAudio(bool send)
    {
        sending_audio_ = send;
    }

    bool Connection::SendingAudio() const
    {
        return sending_audio_;
    }

    void Connection::ReceiveAudio(bool receive)
    {
        receiving_audio_ = receive;
    }

    void Connection::SendAudioFrame(PCMAudioFrame* frame, Vector3df users_position)
    {
        QMutexLocker locker(&mutex_send_audio_);

        if (state_ != STATE_OPEN)
            return;

        PCMAudioFrame* f = new PCMAudioFrame(frame);
        encode_queue_.push_back(f);
        
        if (encode_queue_.size() < FRAMES_PER_PACKET)
            return;

        std::deque<std::string> packet_list; //! @todo SPEED OPTIMIZATION: reuse memory

        for (int i = 0; i < FRAMES_PER_PACKET; ++i)
        {
            PCMAudioFrame* audio_frame = encode_queue_.first();
            encode_queue_.pop_front();

            int32_t len = celt_encode(celt_encoder_, reinterpret_cast<short *>(audio_frame->DataPtr()), NULL, encode_buffer_, std::min(AudioQuality() / (100 * 8), 127));
            packet_list.push_back(std::string(reinterpret_cast<char *>(encode_buffer_), len));
            assert(len < ENCODE_BUFFER_SIZE_);

            delete audio_frame;
        }
        const int PACKET_DATA_SIZE_MAX = 1024;
	    static char data[PACKET_DATA_SIZE_MAX];
	    int flags = 0; // target = 0
	    flags |= (MumbleClient::UdpMessageType::UDPVoiceCELTAlpha << 5);
	    data[0] = static_cast<unsigned char>(flags);
        PacketDataStream data_stream(data + 1, PACKET_DATA_SIZE_MAX - 1);
        data_stream << frame_sequence_;

	    for (int i = 0; i < FRAMES_PER_PACKET; ++i)
        {
		    if (packet_list.empty())
                break;

		    const std::string& s = packet_list.front();

		    unsigned char head = s.size();
		    // Add 0x80 to all but the last frame
		    if (i < FRAMES_PER_PACKET - 1)
			    head |= 0x80;

		    data_stream.append(head);
		    data_stream.append(s);

		    packet_list.pop_front();
            frame_sequence_++;
	    }
        if (send_position_)
        {
            // Coordinate conversion: Naali -> Mumble
            data_stream << static_cast<float>(users_position.y);
            data_stream << static_cast<float>(users_position.z);
            data_stream << static_cast<float>(-users_position.x);
        }

        client_->SendRawUdpTunnel(data, data_stream.size() + 1 );
    }

    void Connection::SetAuthenticated()
    {
        if (state_ != STATE_AUTHENTICATING)
        {
            QString message = QString("Authentication notification received but state = %1").arg(state_);
            MumbleVoipModule::LogWarning(message.toStdString());
            return;
        }

        mutex_authentication_.lock();
        authenticated_ = true;
        mutex_authentication_.unlock();

        if (join_request_.length() > 0)
        {
            QString channel = join_request_;
            join_request_ = "";
            Join(channel);
        }

        state_ = STATE_OPEN;
        emit StateChanged(state_);
    }

    void Connection::HandleIncomingTextMessage(QString text)
    {
        emit (TextMessageReceived(text));
    }

    void Connection::AddChannel(const MumbleClient::Channel& new_channel)
    {
        QMutexLocker locker(&mutex_channels_);

        foreach(Channel* c, channels_)
        {
            if (c->Id() == new_channel.id)
                return;
        }

        Channel* c = new Channel(&new_channel);
        channels_.append(c);
        QString message = QString("Channel '%1' added").arg(c->Name());
        MumbleVoipModule::LogDebug(message.toStdString());
    }

    void Connection::RemoveChannel(const MumbleClient::Channel& channel)
    {
        QMutexLocker locker(&mutex_channels_);

        int i = 0;
        for (int i = 0; i < channels_.size(); ++i)
        {
            if (channels_.at(i)->Id() == channel.id)
            {
                Channel* c = channels_.at(i);
                channels_.removeAt(i);
                // delete c; // @todo 
                QString message = QString("Channel '%1' removed").arg(c->Name());
                MumbleVoipModule::LogDebug(message.toStdString());
                break;
            }
        }
    }

    void Connection::HandleIncomingRawUdpTunnelPacket(int length, void* buffer)
    {
        if (!receiving_audio_)
            return;

        
        if (!mutex_raw_udp_tunnel_.tryLock(10))
            return;
        if (state_ != STATE_OPEN)
        {
            mutex_raw_udp_tunnel_.unlock();
            return;
        }
        PacketDataStream data_stream = PacketDataStream((char*)buffer, length);
        bool valid = data_stream.isValid();

        uint8_t first_byte = static_cast<unsigned char>(data_stream.next());
        MumbleClient::UdpMessageType::MessageType type = static_cast<MumbleClient::UdpMessageType::MessageType>( ( first_byte >> 5) & 0x07 );
        uint8_t flags = first_byte & 0x1f;
        switch (type)
        {
        case MumbleClient::UdpMessageType::UDPVoiceCELTAlpha:
            break;
        case MumbleClient::UdpMessageType::UDPPing:
            MumbleVoipModule::LogDebug("Unsupported packet received: MUMBLE-UDP PING");
            return;
            break;
        case MumbleClient::UdpMessageType::UDPVoiceSpeex:
            MumbleVoipModule::LogDebug("Unsupported packet received: MUMBLE-UDP Speex audio frame");
            return;
            break;
        case MumbleClient::UdpMessageType::UDPVoiceCELTBeta:
            MumbleVoipModule::LogDebug("Unsupported packet received: MUMBLE-UDP CELT B audio frame");
            return;
            break;
        }

        int session;
        int seq;
        data_stream >> session;
        data_stream >> seq;

        bool last_frame = true;
        do
        {
		    int header = static_cast<unsigned char>(data_stream.next());
            int frame_size = header & 0x7f;
            last_frame = !(header & 0x80);
            const char* frame_data = data_stream.charPtr();
            data_stream.skip(frame_size);

            if (frame_size > 0)
                emit CELTFrameReceived(session, (unsigned char*)frame_data, frame_size);
	    }
        while (!last_frame && data_stream.isValid());
        if (!data_stream.isValid())
        {
            MumbleVoipModule::LogWarning("Syntax error in RawUdpTunnel packet.");
        }

        int bytes_left = data_stream.left();
        if (bytes_left)
        {
            // Coordinate conversion: Mumble -> Naali 
            Vector3df position;

            data_stream >> position.y;
            data_stream >> position.z;
            data_stream >> position.x;
            position.x *= -1;

            QMutexLocker user_locker(&mutex_users_);
            if (mutex_users_.tryLock(10))
            {
                User* user = users_[session];
                if (user)
                {
                    if (!user->tryLock(100))
                    {
                        int ytest = 1;
                    }
                    QMutexLocker user_locker(user);
                    user->UpdatePosition(position);
                }
                mutex_users_.unlock();
            }
        }
        mutex_raw_udp_tunnel_.unlock();
    }

    void Connection::CreateUserObject(const MumbleClient::User& mumble_user)
    {
        AddChannel(*mumble_user.channel.lock().get());
        Channel* channel = ChannelById(mumble_user.channel.lock()->id);
        if (!channel)
        {
            QString message = QString("Cannot create user '%1': Channel doesn't exist.").arg(QString(mumble_user.name.c_str()));
            MumbleVoipModule::LogWarning(message.toStdString());
            return;
        }
        User* user = new User(mumble_user, channel);
        user->moveToThread(this->thread());
        
        emit UserObjectCreated(user);
    }

    void Connection::AddToUserList(User* user)
    {
        QMutexLocker locker(&mutex_users_);

        users_[user->Session()] = user;
        QString message = QString("User '%1' joined.").arg(user->Name());
        MumbleVoipModule::LogDebug(message.toStdString());
        user->StartUpdateTimer();
        emit UserJoinedToServer(user);
    }

    void Connection::MarkUserLeft(const MumbleClient::User& mumble_user)
    {
        QMutexLocker locker(&mutex_users_);

        if (!users_.contains(mumble_user.session))
        {
            QString message = QString("Unknow user '%1' Left.").arg(QString(mumble_user.name.c_str()));
            MumbleVoipModule::LogWarning(message.toStdString());
            return;
        }

        User* user = users_[mumble_user.session];
        QMutexLocker user_locker(user);

        QString message = QString("User '%1' Left.").arg(user->Name());
        MumbleVoipModule::LogDebug(message.toStdString());
        user->SetLeft();
        emit UserLeftFromServer(user);
    }

    QList<Channel*> Connection::ChannelList() 
    {
        QMutexLocker locker(&mutex_channels_);

        QList<Channel*> channels;
        foreach(Channel* c, channels_)
        {
            channels.append(c);
        }
        return channels;
    }

    MumbleVoip::Channel* Connection::ChannelById(int id) 
    {
        QMutexLocker locker(&mutex_channels_);

        foreach(Channel* c, channels_)
        {
            if (c->Id() == id)
                return c;
        }
        return 0;
    }

    MumbleVoip::Channel* Connection::ChannelByName(QString name) 
    {
        QMutexLocker locker(&mutex_channels_);

        foreach(Channel* c, channels_)
        {
            if (c->FullName() == name)
                return c;
        }
        return 0;
    }

    void Connection::HandleIncomingCELTFrame(int session, unsigned char* data, int size)
    {
        QMutexLocker locker(&mutex_users_);
        if (state_ != STATE_OPEN)
            return;
        User* user = users_[session];
        if (!user)
        {
            QString message = QString("Audio frame from unknown user: %1").arg(session);
            MumbleVoipModule::LogWarning(message.toStdString());
            return;
        }

        PCMAudioFrame* audio_frame = new PCMAudioFrame(SAMPLE_RATE, SAMPLE_WIDTH, NUMBER_OF_CHANNELS, SAMPLES_IN_FRAME*SAMPLE_WIDTH/8);
        int ret = celt_decode(celt_decoder_, data, size, (short*)audio_frame->DataPtr());

        switch (ret)
        {
        case CELT_OK:
            {
                User* user = users_[session];
                if (user)
                {
                    if (user->tryLock(5)) // 5 ms
                    {
                        user->AddToPlaybackBuffer(audio_frame);
                        user->unlock();
                        return;
                    }
                    else
                    {
//                        user->NotifyAudioPacketDroped();
                        MumbleVoipModule::LogWarning("Audio packet dropped: user locket");
                    }
                }
            }
            break;
        case CELT_BAD_ARG:
            MumbleVoipModule::LogError("CELT decoding error: CELT_BAD_ARG");
            break;
        case CELT_INVALID_MODE:
            MumbleVoipModule::LogError("CELT decoding error: CELT_INVALID_MODE");
            break;
        case CELT_INTERNAL_ERROR:
            MumbleVoipModule::LogError("CELT decoding error: CELT_INTERNAL_ERROR");
            break;
        case CELT_CORRUPTED_DATA:
            MumbleVoipModule::LogError("CELT decoding error: CELT_CORRUPTED_DATA");
            break;
        case CELT_UNIMPLEMENTED:
            MumbleVoipModule::LogError("CELT decoding error: CELT_UNIMPLEMENTED");
            break;
        }
        delete audio_frame;
    }

    void Connection::SetEncodingQuality(double quality)
    {
        QMutexLocker locker(&mutex_encoding_quality_);
        if (quality < 0)
            quality = 0;
        if (quality > 1.0)
            quality = 1.0;
        encoding_quality_ = quality;
    }
    
    int Connection::AudioQuality()
    {
        QMutexLocker locker(&mutex_encoding_quality_);
        return static_cast<int>(encoding_quality_*(AUDIO_QUALITY_MAX_ - AUDIO_QUALITY_MIN_) + AUDIO_QUALITY_MIN_);
    }

    bool Connection::CheckState(QList<State> allowed_states)
    {
        QMutexLocker locker(&mutex_state_);
        foreach(State state, allowed_states)
        {
            if (state == state_)
                return true;
        }
        return false;
    }

    void Connection::CheckChannels()
    {

    }

} // namespace MumbleVoip 
