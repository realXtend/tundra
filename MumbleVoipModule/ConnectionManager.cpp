#include "StableHeaders.h"
#include "ConnectionManager.h"
#include "MumbleVoipModule.h"
#include <QDesktopServices>
#include <QPair>
#include <QMutexLocker>

#define BUILDING_DLL
#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL
#include "LibMumbleThread.h"
#include "Connection.h"
#include "PCMAudioFrame.h"
#include "User.h"

namespace MumbleVoip
{

    ConnectionManager::ConnectionManager(Foundation::Framework* framework) :
        framework_(framework),
        sending_audio_(false),
        recording_device_(""),
        lib_mumble_thread_(0),
        position_x_(0.0),
        position_y_(0.0),
        position_z_(0.0)
    {
    }

    ConnectionManager::~ConnectionManager()
    {
        foreach(Connection* c, connections_)
        {
            delete c;
        }
        connections_.clear();
        StopMumbleLibrary();
    }

    void ConnectionManager::OpenConnection(ServerInfo info)
    {
        Connection* connection = new Connection(info);
        connections_[info.server] = connection;
        connection->Join(info.channel);
        connection->SendAudio(true);
        connection->SetEncodingQuality(0.5);
        connection->SendPosition(true); 
//        QObject::connect( connection, SIGNAL(AudioFramesAvailable(Connection*)), this, SLOT(OnAudioFramesAvailable(Connection*)) );
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
        if (!lib_mumble_thread_)
            lib_mumble_thread_ = new LibMumbleThread();

        if (lib_mumble_thread_->isRunning())
            return;
                
        lib_mumble_thread_->setPriority(QThread::LowPriority);
        lib_mumble_thread_->start();
    }

    void ConnectionManager::StopMumbleLibrary()
    {
        if (!lib_mumble_thread_)
            return;

        //lib_mumble_thread_->shutdown();
        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            MumbleVoipModule::LogError("Cannot stop Mumble library: No library instance available.");
            return;
        }
        mumble_lib->Shutdown();

        lib_mumble_thread_->wait();
        SAFE_DELETE(lib_mumble_thread_);
        MumbleVoipModule::LogDebug("Mumble thread exited.library uninitialized.");
    }

    void ConnectionManager::PlaybackAudio(Connection* connection)
    {
        for(;;)
        {
            AudioPacket packet = connection->GetAudioPacket();
            if (packet.second == 0)
                break;
            PlaybackAudioPacket(packet.first, packet.second);
        }
    }

    void ConnectionManager::PlaybackAudioPacket(User* user, PCMAudioFrame* frame)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
        if (!sound_service.get())
            return;    

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = frame->DataPtr();
        sound_buffer.frequency_ = frame->SampleRate();

        if (frame->SampleWidth() == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = frame->GetLengthBytes();
        sound_buffer.stereo_ = false;
        QMutexLocker locker(user);

        if (audio_playback_channels_.contains(user->Session()))
            if (user->PositionKnown())
                sound_service->PlaySoundBuffer3D(sound_buffer, Foundation::SoundServiceInterface::Voice, user->Position(), audio_playback_channels_[user->Session()]);
            else
                sound_service->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channels_[user->Session()]);
        else
            if (user->PositionKnown())
                audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer3D(sound_buffer, Foundation::SoundServiceInterface::Voice, user->Position(), 0);
            else
                audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, 0);

        delete frame;
    }

    void ConnectionManager::SetAudioSourcePosition(double x, double y, double z)
    {
        position_x_ = x;
        position_y_ = y;
        position_z_ = z;
    }

    void ConnectionManager::SendAudio(bool send)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
        if (!sound_service.get())
            throw std::exception("SoundServiceInterface cannot be found.");

        if (!sending_audio_ && send)
        {
            sending_audio_ = true;
            int frequency = SAMPLE_RATE;
            bool sixteenbit = true;
            bool stereo = false;
            int buffer_size = SAMPLE_WIDTH/8*frequency*AUDIO_RECORDING_BUFFER_MS/1000;
            sound_service->StartRecording(recording_device_, frequency, sixteenbit, stereo, buffer_size);
        }

        if (sending_audio_ && !send)
        {
            sending_audio_ = false;
            sound_service->StopRecording();
        }
    }

    bool ConnectionManager::SendingAudio()
    {
        return sending_audio_;
    }

    void ConnectionManager::Update(f64 frametime)
    {
        if (sending_audio_)
        {
            boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = ConnectionManager::SoundService();
            if (!sound_service)
            {
                MumbleVoipModule::LogDebug("Soundservice cannot be found.");
                return;
            }

            while (sound_service->GetRecordedSoundSize() > SAMPLES_IN_FRAME*SAMPLE_WIDTH/8)
            {
                PCMAudioFrame* frame = new PCMAudioFrame(SAMPLE_RATE, SAMPLE_WIDTH, NUMBER_OF_CHANNELS, SAMPLES_IN_FRAME*SAMPLE_WIDTH/8);
                int bytes = sound_service->GetRecordedSoundData(frame->DataPtr(), SAMPLES_IN_FRAME*SAMPLE_WIDTH/8);

                for (QMap<QString, Connection*>::iterator i = connections_.begin(); i != connections_.end(); ++i)
                {
                    Connection* connection = *i;
                    if (connection->SendingAudio())
                        connection->SendAudioFrame(frame, position_x_, position_y_, position_z_);
                }
                delete frame;
            }
        }

        foreach(Connection* connection, connections_)
        {
            PlaybackAudio(connection);
        }
    }

    boost::shared_ptr<Foundation::SoundServiceInterface> ConnectionManager::SoundService()
    {
        if (!framework_)
            return boost::shared_ptr<Foundation::SoundServiceInterface>();
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        if (!service_manager.get())
            return boost::shared_ptr<Foundation::SoundServiceInterface>();
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return boost::shared_ptr<Foundation::SoundServiceInterface>();

        return soundsystem;
    }

    // GetRecordedSoundData(void* buffer, uint size)

} // end of namespace: MumbleVoip

