#include "StableHeaders.h"
#include "Connection.h"
#include "MumbleVoipModule.h"

//#include "celt.h"
#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client.h>
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL // for dll import/export declarations
#include <mumbleclient/settings.h>
//#include <boost/make_shared.hpp>

#include "SoundServiceInterface.h"
#include "Channel.h"
#include <mumbleclient/PacketDataStream.h>
#include <libcelt/celt.h> 

namespace MumbleVoip {



PCMAudioFrame::PCMAudioFrame(int sample_rate, int sample_width, int channels, char* data, int data_size):
        channels_(channels),
        sample_rate_(sample_rate),
        sample_width_(sample_width),
        data_(data),
        data_size_(data_size)
{
    data_ = new char[data_size];
    memcpy(data_, data, data_size);
}

PCMAudioFrame::~PCMAudioFrame()
{
    SAFE_DELETE_ARRAY(data_);
}
    
char* PCMAudioFrame::Data()
{
    return data_;
}

int PCMAudioFrame::Channels()
{
    return channels_;
}
    
int PCMAudioFrame::SampleRate()
{
    return sample_rate_;
}

int PCMAudioFrame::SampleWidth()
{
    return sample_width_;
}
    
int PCMAudioFrame::Samples()
{
    return data_size_ / sample_width_;
}

int PCMAudioFrame::GetLengthMs()
{
    return 1000 * Samples() / sample_rate_;
}

int PCMAudioFrame::GetLengthBytes()
{
    return data_size_;
}

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
    connection->OnTextMessage(QString(message.c_str()));
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
    int frames = scanPacket((char*)buffer, length);
    
    PacketDataStream data_stream = PacketDataStream((char*)buffer, length);
    bool valid = data_stream.isValid();




    MumbleClient::UdpMessageType::MessageType type = static_cast<MumbleClient::UdpMessageType::MessageType>( (data_stream.next() >> 5) & 0x07 );
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
    //int session << data_stream.next();
    //int seq >> data_stream.next();

    bool last_frame = true;
    do {
		int header = static_cast<unsigned char>(data_stream.next());
        int frame_size = header & 0x7f;
        last_frame = !(header & 0x80);
        const char* frame_data = data_stream.charPtr();
        data_stream.skip(frame_size);
        connection->OnRawUdpTunnel((char*)frame_data, frame_size);

	} while (!last_frame);


    ////int seg = data_stream.next8();
    ////seg = data_stream.next8();
    ////seg = data_stream.next8();
    ////seg = data_stream.next8();
    //int seg = 1;
    //for (int i = 0; i < frames; ++i)
    //{
    //    uint8_t data_size = data_stream.next8();
    //    const char* data = data_stream.charPtr();

    //    connection->OnRawUdpTunnel((char*)data, data_size);
    //}
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
    connection->OnAuthenticated();
	std::cout << "I'm authenticated" << std::endl;
}

void ChannelAddCallback(const MumbleClient::Channel& channel, Connection* connection)
{
    connection->OnChannelAddCallback(channel);
}

void ChannelRemoveCallback(const MumbleClient::Channel& channel, Connection* connection)
{
	std::cout << "Channel removed" << std::endl;
}

Connection::Connection(ServerInfo &info) :
        client_(0),
        authenticated_(false),
        celt_mode_(0),
        celt_encoder_(0),
        celt_decoder_(0)
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
}

void Connection::Close()
{

}

void Connection::InitializeCELT()
{
    int error = 0;
    int channels = 1;
    int framesize = SAMPLE_RATE_ / 100;
    celt_mode_ = celt_mode_create(SAMPLE_RATE_, channels, framesize, &error );
    if (error != 0)
    {
        MumbleVoipModule::LogDebug("CELT initialized.");
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

void Connection::OnAuthenticated()
{
    authenticated_ = true;
    if (join_request_.length() > 0)
    {
        QString channel = join_request_;
        join_request_ = "";
        Join(channel);
    }
}

void Connection::OnTextMessage(QString text)
{
    emit (TextMessage(text));
}

void Connection::OnRawUdpTunnel(char* data, int size)
{
    // @todo: lock 

    // CELT decode

    //celt_mode_info(celt_mode_, ???, sample_rate);
    int sample_count = 480;
    celt_int16_t *pcm_data = new celt_int16_t[sample_count];
    int ret = celt_decode(celt_decoder_, (unsigned char*)data, size, pcm_data);
    switch (ret)
    {
    case CELT_OK:
        {
            if (playback_queue_.size() < 100)
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
    SAFE_DELETE_ARRAY(pcm_data);
   
}

PCMAudioFrame* Connection::GetAudioFrame()
{
    if (playback_queue_.size() == 0)
        return 0;
    PCMAudioFrame* frame = playback_queue_.takeFirst();
    return frame;
}

void Connection::OnPlayAudioData(char* data, int size)
{
	char *buffer = new char[size];
    memcpy(buffer, data, size);


	int frames = scanPacket(buffer, size);
    if (frames == -1)
    {
        // invalid packet
        return;
    }

	buffer[0] = MumbleClient::UdpMessageType::UDPVoiceCELTAlpha | 0;
	memcpy(&buffer[1], &buffer[2], size - 1);

    // @todo: encode CELT
#define TCP 1
#if TCP
	client_->SendRawUdpTunnel(buffer, size - 1);
#else
	client_->SendUdpMessage(buffer, size - 1);
#endif
	delete []buffer;
}

void Connection::OnChannelAddCallback(const MumbleClient::Channel& channel)
{
    Channel* c = new Channel(channel);
    channels_.append(c);
    QString message = QString("Channel '%1' added").arg(c->Name());
    MumbleVoipModule::LogDebug(message.toStdString());
}

QList<QString> Connection::Channels()
{
    QList<QString> channels;
    foreach(Channel* c, channels_)
    {
        channels.append(c->Name());
    }
    return channels;
}

} // namespace MumbleVoip 
