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

static inline int32_t pds_int_len(char* x)
{
	if ((x[0] & 0x80) == 0x00) {
		return 1;
	} else if ((x[0] & 0xC0) == 0x80) {
		return 2;
	} else if ((x[0] & 0xF0) == 0xF0) {
		switch (x[0] & 0xFC) {
			case 0xF0:
				return 5;
			case 0xF4:
				return 9;
			case 0xF8:
				return pds_int_len(&x[1]) + 1;
			case 0xFC:
				return 1;
			default:
				return 1;
		}
	} else if ((x[0] & 0xF0) == 0xE0) {
		return 3;
	} else if ((x[0] & 0xE0) == 0xC0) {
		return 3;
	}

	return 0;
}

int scanPacket(char* data, int len) {
	int header = 0;
	int frames = 0;
	// skip flags
	int pos = 1;

	// skip session & seqnr
	pos += pds_int_len(&data[pos]);
	pos += pds_int_len(&data[pos]);

	bool valid = true;
	do {
		header = static_cast<unsigned char>(data[pos]);
		++pos;
		++frames;
		pos += (header & 0x7f);

		if (pos > len)
			valid = false;
	} while ((header & 0x80) && valid);

	if (valid) {
		return frames;
	} else {
		return -1;
	}
}

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
    client_->Connect(MumbleClient::Settings(info.server.toStdString(), port.toStdString(), info.user_name.toStdString(), info.password.toStdString()));
	client_->SetRawUdpTunnelCallback( boost::bind(&RawUdpTunnelCallback, _1, _2, this));
    client_->SetChannelAddCallback(boost::bind(&ChannelAddCallback, _1, this));
    client_->SetChannelRemoveCallback(boost::bind(&ChannelRemoveCallback, _1, this));
    client_->SetTextMessageCallback(boost::bind(&TextMessageCallback, _1, this));
    client_->SetAuthCallback(boost::bind(&AuthCallback, this));
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
    int channels = 1;
    int framesize = SAMPLE_RATE_ / 100;
    celt_mode_ = celt_mode_create(SAMPLE_RATE_, channels, framesize, &error );
    if (error != 0)
    {
        QString message = QString("CELT initialization failed, error code = %1").arg(error);
        MumbleVoipModule::LogDebug(message.toStdString());
    }
    celt_encoder_ = celt_encoder_create(celt_mode_);
    celt_decoder_ = celt_decoder_create(celt_mode_);
    MumbleVoipModule::LogDebug("CELT initialized.");
}

void Connection::UninitializeCELT()
{
    celt_decoder_destroy(celt_decoder_);
    celt_encoder_destroy(celt_encoder_);
    celt_mode_destroy(celt_mode_);
    MumbleVoipModule::LogDebug("CELT uninitialized.");
}

void Connection::Join(QString channel_name)
{
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
    authenticated_ = true;
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

void Connection::HandleIncomingCELTFrame(char* data, int size)
{
    // @todo: lock 

    // CELT decode

    //celt_mode_info(celt_mode_, ???, sample_rate);
    int sample_count = 480;
    celt_int16_t pcm_data[480];
    int ret = celt_decode(celt_decoder_, (unsigned char*)data, size, pcm_data);
    switch (ret)
    {
    case CELT_OK:
        {
            if (playback_queue_.size() < 100000)
            {
                PCMAudioFrame* audio_frame = new PCMAudioFrame(48000, 16, 1, (char*)pcm_data, 2*480);
                playback_queue_.push_back(audio_frame);
                emit AudioFramesAvailable(this);
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

PCMAudioFrame* Connection::GetAudioFrame()
{
    if (playback_queue_.size() == 0)
        return 0;

    PCMAudioFrame* frame = playback_queue_.takeFirst();
    return frame;
}

void Connection::SendAudioFrame(PCMAudioFrame* frame)
{
    //// TESTING
    //QFile file("sending.raw");
    //file.open(QIODevice::OpenModeFlag::Append);
    //file.write(frame->Data(), frame->GetLengthBytes());
    //file.close();

    std::deque<std::string> packet_list;

    int audio_quality = 60000;
//    sending_queue_.push_back(frame);

    celt_encoder_ctl(celt_encoder_, CELT_SET_LTP(0));
	//celt_encoder_ctl(celt_encode, CELT_SET_VBR_RATE(audio_quality));

    int32_t len = celt_encode(celt_encoder_, reinterpret_cast<short *>(frame->Data()), NULL, (unsigned char*)&encode_buffer_, std::min(audio_quality / (100 * 8), 127));
    packet_list.push_back(std::string(reinterpret_cast<char *>(encode_buffer_), len));

    // TESTING
    //QFile file2("sending-celt.raw");
    //file2.open(QIODevice::OpenModeFlag::Append);
    //file2.write((char*)&len,4);
    //file2.write(encode_buffer_, len);
    //file2.close();

    int32_t seq = 0;
	int frames = 1;
    int session = 0;
    while (!packet_list.empty()) 
    {
		char data[1024];
		int flags = 0; // target = 0
		flags |= (MumbleClient::UdpMessageType::UDPVoiceCELTAlpha << 5);
		data[0] = static_cast<unsigned char>(flags);
        PacketDataStream data_stream(data + 1, 1023);
        frame_sequence_++;
        data_stream << session;
        data_stream << frame_sequence_;

		for (int i = 0; i < frames; ++i)
        {
			if (packet_list.empty())
                break;

			const std::string& s = packet_list.front();

			unsigned char head = s.size();
			// Add 0x80 to all but the last frame
			if (i < frames - 1)
				head |= 0x80;

			data_stream.append(head);
			data_stream.append(s);

			packet_list.pop_front();
		}
        client_->SendRawUdpTunnel(data, data_stream.size() + 1 );
    }
    delete frame;
}

void Connection::OnChannelAddCallback(const MumbleClient::Channel& channel)
{
    // \todo THREAD SAFETY

    Channel* c = new Channel(channel);
    channels_.append(c);
    QString message = QString("Channel '%1' added").arg(c->Name());
    MumbleVoipModule::LogDebug(message.toStdString());
}

void Connection::OnChannelRemoveCallback(const MumbleClient::Channel& channel)
{
    // \todo THREAD SAFETY

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
    int frames = scanPacket((char*)buffer, length);
    
    PacketDataStream data_stream = PacketDataStream((char*)buffer, length);
    bool valid = data_stream.isValid();

    uint8_t first_byte = data_stream.next();
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

    //int skip = pds_int_len(data_stream.charPtr());
    //data_stream.skip(skip);

    //skip = pds_int_len(data_stream.charPtr());
    //data_stream.skip(skip);

    int session;
    int seq;
    data_stream >> session;
    data_stream >> seq;

    bool last_frame = true;
    do {
		int header = static_cast<unsigned char>(data_stream.next());
        int frame_size = header & 0x7f;
        last_frame = !(header & 0x80);
        const char* frame_data = data_stream.charPtr();
        data_stream.skip(frame_size);

        // TESTING
        //if (playback_queue_.size() == 0)
        //{
        //    char buffer[1024];
        //    QFile file2("sending-celt_.raw");
        //    file2.open(QIODevice::OpenModeFlag::ReadOnly);
        //    while (file2.bytesAvailable() > 0)
        //    {
        //        int len = 0;
        //        file2.read((char*)&len, 4);
        //        file2.read(buffer, len);
        //        HandleIncomingCELTFrame((char*)buffer, len);
        //    }
        //    file2.close();
        //}

        HandleIncomingCELTFrame((char*)frame_data, frame_size);
	} while (!last_frame && data_stream.isValid());

    int bytes_left = data_stream.left();
    if (bytes_left)
    {
        float position[3];
        data_stream >> position[0];
        data_stream >> position[1];
        data_stream >> position[2];
    }
}

QList<QString> Connection::Channels()
{
    // \todo THREAD SAFETY

    QList<QString> channels;
    foreach(Channel* c, channels_)
    {
        channels.append(c->Name());
    }
    return channels;
}

void Connection::SendAudioFrame()
{
    // \todo Thread safety

    if (sending_queue_.isEmpty())
        return;

    PCMAudioFrame*& frame = sending_queue_.first();
}

void Connection::SendAudio(bool send)
{
    sending_audio_ = send;
}

bool Connection::SendingAudio()
{
    return sending_audio_;
}

} // namespace MumbleVoip 
