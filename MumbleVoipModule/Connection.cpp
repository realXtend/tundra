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
#include <boost/make_shared.hpp>

#include "SoundServiceInterface.h"

namespace MumbleVoip {

//struct RelayMessage {
//	MumbleClient::MumbleClient* mc;
//	const std::string message;
//	RelayMessage(MumbleClient::MumbleClient* mc_, const std::string& message_) : mc(mc_), message(message_) { }
//};

//static boost::mutex mut;
//static std::deque< boost::shared_ptr<RelayMessage> > relay_queue;
//static boost::condition_variable cond;



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

void RelayTunnelCallback(int32_t length, void* buffer_, Connection* connection)
{
    char* buffer = (char*)buffer_;
    short type = buffer[0] << 8 + buffer[1];
    int len = buffer[3] << 16 + buffer[4] << 8 + buffer[5];
	std::string s(static_cast<char *>(buffer), length);
	s.erase(1, pds_int_len(&static_cast<char *>(buffer)[1]));
    connection->OnRelayTunnel(s);
}

void AuthCallback(Connection* connection)
{
    connection->OnAuthenticated();
	std::cout << "I'm authenticated" << std::endl;
}

void ChannelAddCallback(const MumbleClient::Channel& channel, Connection* connection)
{
	std::cout << "Channel added" << std::endl;
}

void ChannelRemoveCallback(const MumbleClient::Channel& channel, Connection* connection)
{
	std::cout << "Channel removed" << std::endl;
}

Connection::Connection(ServerInfo &info) : client_(0), authenticated_(false)
{
    MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
    client_ = mumble_lib->NewClient();

    QString port = "64738"; // default port name
    client_->Connect(MumbleClient::Settings(info.server.toStdString(), port.toStdString(), info.user_name.toStdString(), info.password.toStdString()));
	client_->SetRawUdpTunnelCallback( boost::bind(&RelayTunnelCallback, _1, _2, this));
    client_->SetChannelAddCallback(boost::bind(&ChannelAddCallback, _1, this));
    client_->SetChannelRemoveCallback(boost::bind(&ChannelRemoveCallback, _1, this));
    client_->SetTextMessageCallback(boost::bind(&TextMessageCallback, _1, this));
    client_->SetAuthCallback(boost::bind(&AuthCallback, this));
}

Connection::~Connection()
{
    SAFE_DELETE(client_);
}

void Connection::Close()
{

}

void Connection::Join(QString channel)
{
    if (!authenticated_)
    {
        join_request_ = channel;
        return; // @todo: Throw exception
    }

    int channel_id = 0;
    client_->JoinChannel(channel_id);
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

void Connection::OnRelayTunnel(std::string& s)
{
    // @todo decode CELT

    char* data = new char[s.size()];
    memcpy(data, s.c_str(), s.size());
    emit (RelayTunnelData(data,s.size()));
}

void Connection::OnPlayAudioData(char* data, int size)
{
	char *buffer = new char[size];
    memcpy(buffer, data, size);

	int frames = scanPacket(buffer, size);

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

} // namespace MumbleVoip 
