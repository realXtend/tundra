#include "StableHeaders.h"
#include "ConnectionManager.h"
#include "MumbleVoipModule.h"
#include "SoundServiceInterface.h"
#include <QDesktopServices>

#define BUILDING_DLL
#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL
#include "Connection.h"
#include <libcelt/celt.h>

namespace MumbleVoip
{

    void LibThread::run()
    {
        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
        MumbleVoipModule::LogDebug("Mumble library mainloop started");
        mumble_lib->Run();
        MumbleVoipModule::LogDebug("Mumble library mainloop stopped");
    }

    ConnectionManager::ConnectionManager(Foundation::Framework* framework) :
        mumble_lib(0),
        framework_(framework),
        audio_playback_channel_(0),
        celt_mode_(0),
        celt_encoder_(0),
        celt_decoder_(0)
    {
        InitializeCELT();
        StartMumbleLibrary();
    }

    ConnectionManager::~ConnectionManager()
    {
        StopMumbleLibrary();
    }

    void ConnectionManager::InitializeCELT()
    {
        int error = 0;
        int channels = 1;
        int framesize = SAMPLE_RATE_ / 100;
        celt_mode_ = celt_mode_create(SAMPLE_RATE_, channels, framesize, &error );
        celt_encoder_ = celt_encoder_create(celt_mode_);
        celt_decoder_ = celt_decoder_create(celt_mode_);
        MumbleVoipModule::LogDebug("CELT initialized.");
    }

    void ConnectionManager::UninitializeCELT()
    {
        celt_decoder_destroy(celt_decoder_);
        celt_encoder_destroy(celt_encoder_);
        celt_mode_destroy(celt_mode_);
        MumbleVoipModule::LogDebug("CELT uninitialized.");
    }


    void ConnectionManager::OpenConnection(ServerInfo info)
    {
        Connection* connection = new Connection(info);
        connections_[info.server] = connection;
        connection->Join(info.channel);
        QObject::connect( connection, SIGNAL(RelayTunnelData(char*, int)), this, SLOT(OnAudioData(char*, int)) );
        StartMumbleLibrary();
    }

    void ConnectionManager::CloseConnection(ServerInfo info)
    {
        if (connections_.contains(info.server))
        {
            Connection* connection = connections_[info.server];
            connection->Close();
            connections_.remove(info.server);
        }
    }

    // static
    void ConnectionManager::StartMumbleClient(const QString& server_url)
    {
        QUrl url(server_url);
        if (!url.isValid())
        {
            QString error_message = QString("Url '%1' is invalid.").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
        
        if (! QDesktopServices::openUrl(server_url))
        {
            QString error_message = QString("Cannot find handler application for url: %1").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
    }

    //static
    void ConnectionManager::KillMumbleClient()
    {
        // Evil hack to ensure that voip connection is not remaining open when using native mumble client
        QProcess kill_mumble;
        MumbleVoipModule::LogDebug("Try to kill mumble.exe process.");
        kill_mumble.start("taskkill /F /FI \"IMAGENAME eq mumble.exe"); // Works only for Windows
    }

    void ConnectionManager::StartMumbleLibrary()
    {
        mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
                
        lib_thread_.start();
    }

    void ConnectionManager::StopMumbleLibrary()
    {
        mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }

        mumble_lib->Shutdown();
        lib_thread_.wait();
        MumbleVoipModule::LogDebug("Mumble library uninitialized.");
    }


    void ConnectionManager::OnAudioData(char* data, int size)
    {
        if (!framework_)
            return;
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            return;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;     

        celt_int16_t *pcm_data = new celt_int16_t[480];
        int ret = celt_decode(celt_decoder_, (unsigned char*)data, size, pcm_data);

        int sample_rate = SAMPLE_RATE_; // test
        int sample_width = 16; // test
        int channel_count = 1; // test
        bool stereo = false; // test
        int spatial_audio_playback_ = true; // test

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = pcm_data;
        sound_buffer.frequency_ = sample_rate;

        if (sample_width == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = SAMPLE_RATE_ / 100 * 2;
        sound_buffer.stereo_ = stereo;
        if (size > 0 && sample_rate != -1 && sample_width != -1 && (channel_count == 1 || channel_count == 2) )
        {

            //if (spatial_audio_playback_)
            //    audio_playback_channel_ = soundsystem->PlaySoundBuffer3D(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_position_, audio_playback_channel_);
            //else
                audio_playback_channel_ = soundsystem->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channel_);
        }

    }
} // end of namespace: MumbleVoip

