#include "StableHeaders.h"
#include "Connection.h"
#include "MumbleVoipModule.h"

//#include "celt.h"
#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client.h>
#include <mumbleclient/client_lib.h>
#include <mumbleclient/channel.h>
#undef BUILDING_DLL // for dll import/export declarations
#include <mumbleclient/settings.h>
//#include <boost/make_shared.hpp>

#include "SoundServiceInterface.h"
#include "Channel.h"
#include "PCMAudioFrame.h"
#include <mumbleclient/PacketDataStream.h>
#include <libcelt/celt_types.h> 
#include <libcelt/celt.h> 

namespace MumbleVoip {


//struct RelayMessage {
//	MumbleClient::MumbleClient* mc;
//	const std::string message;
//	RelayMessage(MumbleClient::MumbleClient* mc_, const std::string& message_) : mc(mc_), message(message_) { }
//};

//static boost::mutex mut;
//static std::deque< boost::shared_ptr<RelayMessage> > relay_queue;
//static boost::condition_variable cond;

//
//

    // \todo Move these static callback functions to separeate file...

//static inline int32_t pds_int_len(char* x)
//{
//	if ((x[0] & 0x80) == 0x00) {
//		return 1;
//	} else if ((x[0] & 0xC0) == 0x80) {
//		return 2;
//	} else if ((x[0] & 0xF0) == 0xF0) {
//		switch (x[0] & 0xFC) {
//			case 0xF0:
//				return 5;
//			case 0xF4:
//				return 9;
//			case 0xF8:
//				return pds_int_len(&x[1]) + 1;
//			case 0xFC:
//				return 1;
//			default:
//				return 1;
//		}
//	} else if ((x[0] & 0xF0) == 0xE0) {
//		return 3;
//	} else if ((x[0] & 0xE0) == 0xC0) {
//		return 3;
//	}
//
//	return 0;
//}

//int scanPacket(char* data, int len) {
//	int header = 0;
//	int frames = 0;
//	// skip flags
//	int pos = 1;
//
//	// skip session & seqnr
//	pos += pds_int_len(&data[pos]);
//	pos += pds_int_len(&data[pos]);
//
//	bool valid = true;
//	do {
//		header = static_cast<unsigned char>(data[pos]);
//		++pos;
//		++frames;
//		pos += (header & 0x7f);
//
//		if (pos > len)
//			valid = false;
//	} while ((header & 0x80) && valid);
//
//	if (valid) {
//		return frames;
//	} else {
//		return -1;
//	}
//}

void TextMessageCallback(const std::string& message, Connection* connection)
{
    connection->OnTextMessageCallback(QString(message.c_str()));
}

//void RelayTunnelCallback(int32_t length, void* buffer, MumbleClient::MumbleClient* mc)
//{
//	std::string s(static_cast<char *>(buffer), length);
//	s.erase(1, pds_int_len(&static_cast<char *>(buffer)[1]));
//	boost::shared_ptr<RelayMessage> r = boost::make_shared<RelayMessage>(mc, s);
//	{
//		boost::lock_guard<boost::mutex> lock(mut);
//		relay_queue.push_back(r);
//	}
//	cond.notify_all();
//}

void RawUdpTunnelCallback(int32_t length, void* buffer, Connection* connection)
{
    connection->OnRawUdpTunnelCallback(length, buffer);
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
    connection->OnAuthCallback();
}

void ChannelAddCallback(const MumbleClient::Channel& channel, Connection* connection)
{
    connection->OnChannelAddCallback(channel);
}

void ChannelRemoveCallback(const MumbleClient::Channel& channel, Connection* connection)
{
    connection->OnChannelRemoveCallback(channel);
}



Connection::Connection(ServerInfo &info) :
        client_(0),
        authenticated_(false),
        celt_mode_(0),
        celt_encoder_(0),
        celt_decoder_(0),
        sending_audio_(false),
        frame_sequence_(0)
{
    InitializeCELT();

    MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
    client_ = mumble_lib->NewClient();

    QString port = "64738"; // default port name
    // \todo Handle connection error
    
	client_->SetRawUdpTunnelCallback( boost::bind(&RawUdpTunnelCallback, _1, _2, this));
    client_->SetChannelAddCallback(boost::bind(&ChannelAddCallback, _1, this));
    client_->SetChannelRemoveCallback(boost::bind(&ChannelRemoveCallback, _1, this));
    client_->SetTextMessageCallback(boost::bind(&TextMessageCallback, _1, this));
    client_->SetAuthCallback(boost::bind(&AuthCallback, this));
    client_->Connect(MumbleClient::Settings(info.server.toStdString(), port.toStdString(), info.user_name.toStdString(), info.password.toStdString()));
}

Connection::~Connection()
{
    UninitializeCELT();
    SAFE_DELETE(client_);
    // \todo clean playback queue
    // \todo clean send queue
    // \todo clean channel list
}

void Connection::Close()
{
    client_->Disconnect();
}

void Connection::InitializeCELT()
{
    int error = 0;
    
    int framesize = SAMPLE_RATE_ / 100;
    celt_mode_ = celt_mode_create(SAMPLE_RATE_, framesize, &error );
    if (error != 0)
    {
        QString message = QString("CELT initialization failed, error code = %1").arg(error);
        MumbleVoipModule::LogDebug(message.toStdString());
    }

    celt_encoder_ = celt_encoder_create(celt_mode_,CHANNELS, NULL );
    celt_encoder_ctl(celt_encoder_, CELT_SET_PREDICTION(0));
	celt_encoder_ctl(celt_encoder_, CELT_SET_VBR_RATE(AUDIO_QUALITY_));

    MumbleVoipModule::LogDebug("CELT initialized.");
}

void Connection::UninitializeCELT()
{
    celt_decoder_destroy(celt_decoder_);
    celt_encoder_destroy(celt_encoder_);
    celt_mode_destroy(celt_mode_);
    MumbleVoipModule::LogDebug("CELT uninitialized.");
}

CELTDecoder* Connection::CreateCELTDecoder()
{
    CELTDecoder* decoder = celt_decoder_create(celt_mode_,CHANNELS, NULL);
    return decoder;
}

void Connection::Join(QString channel_name)
{
    QMutexLocker locker1(&mutex_authentication_);
    QMutexLocker locker2(&mutex_channels_);

    if (!authenticated_)
    {
        join_request_ = channel_name;
        return; // @todo: Throw exception
    }

    foreach(Channel* c, channels_)
    {
        if (c->Name() == channel_name)
        {
            client_->JoinChannel(c->Id());
        }
    }
}

void Connection::OnAuthCallback()
{
    mutex_authentication_.lock();
    authenticated_ = true;
    mutex_authentication_.unlock();

    if (join_request_.length() > 0)
    {
        QString channel = join_request_;
        join_request_ = "";
        Join(channel);
    }
}

void Connection::OnTextMessageCallback(QString text)
{
    emit (TextMessage(text));
}

PCMAudioFrame* Connection::GetAudioFrame()
{
    QMutexLocker locker(&mutex_playback_queue_);

    if (playback_queue_.size() == 0)
        return 0;

    PCMAudioFrame* frame = playback_queue_.takeFirst();
    return frame;
}

void Connection::SendAudioFrame(PCMAudioFrame* frame)
{
    QMutexLocker locker(&mutex_encode_audio_);
    
    if (encode_queue_.size() < FRAMES_PER_PACKET_)
        return;

    std::deque<std::string> packet_list;

    for (int i = 0; i < FRAMES_PER_PACKET_; ++i)
    {
        PCMAudioFrame* audio_frame = encode_queue_.first();
        encode_queue_.pop_front();

        int32_t len = celt_encode(celt_encoder_, reinterpret_cast<short *>(audio_frame->DataPtr()), NULL, encode_buffer_, std::min(AUDIO_QUALITY_ / (100 * 8), 127));
        packet_list.push_back(std::string(reinterpret_cast<char *>(encode_buffer_), len));
        assert(len < ENCODE_BUFFER_SIZE_);

        delete audio_frame;
    }
    int session = 0;
	char data[1024];
	int flags = 0; // target = 0
	flags |= (MumbleClient::UdpMessageType::UDPVoiceCELTAlpha << 5);
	data[0] = static_cast<unsigned char>(flags);
    PacketDataStream data_stream(data + 1, 1023);
    data_stream << frame_sequence_;

	for (int i = 0; i < FRAMES_PER_PACKET_; ++i)
    {
		if (packet_list.empty())
            break;

		const std::string& s = packet_list.front();

		unsigned char head = s.size();
		// Add 0x80 to all but the last frame
		if (i < FRAMES_PER_PACKET_ - 1)
			head |= 0x80;

		data_stream.append(head);
		data_stream.append(s);

		packet_list.pop_front();
        frame_sequence_++;
	}
    client_->SendRawUdpTunnel(data, data_stream.size() + 1 );
}

void Connection::OnChannelAddCallback(const MumbleClient::Channel& channel)
{
    QMutexLocker locker(&mutex_channels_);

    Channel* c = new Channel(channel);
    channels_.append(c);
    QString message = QString("Channel '%1' added").arg(c->Name());
    MumbleVoipModule::LogDebug(message.toStdString());
}

void Connection::OnChannelRemoveCallback(const MumbleClient::Channel& channel)
{
    QMutexLocker locker(&mutex_channels_);

    int i = 0;
    for (int i = 0; i < channels_.size(); ++i)
    {
        if (channels_.at(i)->Id() == channel.id)
        {
            channels_.removeAt(i);
            return;
        }
    }
}

void Connection::OnRawUdpTunnelCallback(int32_t length, void* buffer)
{
    //return; // test
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
        MumbleVoipModule::LogDebug("MUMBLE-UDP: PING");
        return;
        break;
    case MumbleClient::UdpMessageType::UDPVoiceSpeex:
        MumbleVoipModule::LogDebug("MUMBLE-UDP: Speex");
        return;
        break;
    case MumbleClient::UdpMessageType::UDPVoiceCELTBeta:
        MumbleVoipModule::LogDebug("MUMBLE-UDP: CELT B");
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
            HandleIncomingCELTFrame(session, (unsigned char*)frame_data, frame_size);
	}
    while (!last_frame && data_stream.isValid());
    if (!data_stream.isValid())
    {
        MumbleVoipModule::LogWarning("Syntax error in RawUdpTunnel packet.");
    }

    int bytes_left = data_stream.left();
    if (bytes_left)
    {
        float position[3];
        data_stream >> position[0];
        data_stream >> position[1];
        data_stream >> position[2];
        // \todo store position data
    }
}

QList<QString> Connection::Channels()
{
    QMutexLocker locker(&mutex_channels_);

    QList<QString> channels;
    foreach(Channel* c, channels_)
    {
        channels.append(c->Name());
    }
    return channels;
}

void Connection::SendAudio(bool send)
{
    sending_audio_ = send;
}

bool Connection::SendingAudio()
{
    return sending_audio_;
}

void Connection::HandleIncomingCELTFrame(int session, unsigned char* data, int size)
{
    QMutexLocker locker(&mutex_playback_queue_);

    CELTDecoder* decoder = celt_decoders_[session];
    if (!decoder)
    {
        decoder = CreateCELTDecoder();
        celt_decoders_[session] = decoder;
    }

    PCMAudioFrame* audio_frame = new PCMAudioFrame(SAMPLE_RATE_, SAMPLE_WIDTH, CHANNELS, 2*SAMPLES_IN_FRAME);

    int ret = celt_decode(decoder, data, size, (short*)audio_frame->DataPtr());

    switch (ret)
    {
    case CELT_OK:
        {
            int buffer_frames_max = SAMPLE_RATE_/SAMPLES_IN_FRAME*PLAYBACK_BUFFER_MS/1000;
            if (playback_queue_.size() < buffer_frames_max)
            {
                playback_queue_.push_back(audio_frame);
                emit AudioFramesAvailable(this);
            }
            else
            {
                delete audio_frame;
                MumbleVoipModule::LogDebug("Drop incoming Mumble audio packet");
                return;
            }
        }
        break;
    case CELT_BAD_ARG:
        break;
    case CELT_INVALID_MODE:
        break;
    case CELT_INTERNAL_ERROR:
        break;
    case CELT_CORRUPTED_DATA:
        break;
    case CELT_UNIMPLEMENTED:
        break;
    }
}

} // namespace MumbleVoip 
