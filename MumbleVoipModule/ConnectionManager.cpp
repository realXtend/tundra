#include "StableHeaders.h"
#include "ConnectionManager.h"
#include "MumbleVoipModule.h"
#include "SoundServiceInterface.h"

//#include "celt.h"
//#include <mumbleclient/client.h>

#define BUILDING_DLL
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL

#include <QDesktopServices>
#include "Connection.h"

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

    ConnectionManager::ConnectionManager(Foundation::Framework* framework) : mumble_lib(0), framework_(framework)
    {
        StartMumbleLibrary();
    }

    ConnectionManager::~ConnectionManager()
    {
        StopMumbleLibrary();
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
        // todo: IMPLEMENT

        QProcess kill_mumble;
        MumbleVoipModule::LogDebug("Try to kill mumble.exe process.");
        kill_mumble.start("taskkill /F /FI \"IMAGENAME eq mumble.exe");
    }

    void ConnectionManager::StartMumbleLibrary()
    {
        mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
                
        lib_thread_.start();
        //mumble_lib->Run();
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

        int sample_rate = 16000; // test
        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = data;
        sound_buffer.frequency_ = sample_rate;
        int sample_width = 16; // test
        int channel_count = 2; // test
        bool stereo = true; // test
        int spatial_audio_playback_ = true; // test

        if (sample_width == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = size;
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

