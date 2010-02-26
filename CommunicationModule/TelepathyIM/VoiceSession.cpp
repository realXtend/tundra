// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <TelepathyQt4/ReferencedHandles>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingChannel>
#include "VoiceSession.h"
#include <CommunicationService.h>
#include "ServiceManager.h"
#include "VoiceSessionParticipant.h"
#include "SoundServiceInterface.h"
#include "Contact.h"
#include "CoreException.h"

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    
    VoiceSession::VoiceSession(const Tp::StreamedMediaChannelPtr &tp_channel)
        : state_(STATE_INITIALIZING), 
        tp_channel_(tp_channel), 
        pending_audio_streams_(0), 
        farsight_channel_(0), 
        pending_video_streams_(0), 
        audio_playback_channel_(0),
        positional_voice_enabled_(false),
        audio_playback_position_( Vector3df(0.0f, 0.0f, 0.0f)),
        spatial_audio_playback_(false)

    {
        connect(tp_channel_->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(OnIncomingChannelReady(Tp::PendingOperation*)));
    }

    VoiceSession::VoiceSession(Tp::ContactPtr tp_contact):
        state_(STATE_INITIALIZING),
        tp_channel_(0),
        pending_audio_streams_(0),
        farsight_channel_(0),
        pending_video_streams_(0),
        audio_playback_channel_(0),
        positional_voice_enabled_(false),
        audio_playback_position_( Vector3df(0.0f, 0.0f, 0.0f)),
        spatial_audio_playback_(false)
    {
        tp_contact_ = tp_contact;
        QVariantMap request;
        request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA);
        request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), (uint) Tp::HandleTypeContact);
        request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"), tp_contact->handle().at(0));

        Tp::ConnectionPtr tp_connection = tp_contact->manager()->connection();
        connect(tp_connection->ensureChannel(request),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnOutgoingChannelCreated(Tp::PendingOperation*) ));
    }

    VoiceSession::~VoiceSession()
    {
        DeleteChannels();
        for (VoiceSessionParticipantVector::iterator i =  participants_.begin(); i != participants_.end(); ++i)
        {
            VoiceSessionParticipant* p = *i;
            SAFE_DELETE(p);
        }
        participants_.clear();
    }

    void VoiceSession::DeleteChannels()
    {
        // todo: check stream closes
        //if (pending_audio_streams_)
        //    disconnect(
        pending_audio_streams_ = 0;
        //if (pending_video_streams_)
        //    disconnect(
        pending_video_streams_ = 0;

        Tp::MediaStreamPtr audio_stream = GetAudioMediaStream();
        if (audio_stream)
            audio_stream->requestDirection(false, false);
        Tp::MediaStreamPtr video_stream = GetVideoMediaStream();
        if (video_stream)
            audio_stream->requestDirection(false, false);
        
        if (tp_channel_)
        {
            tp_channel_->requestClose();
        }

        if (farsight_channel_)
        {
            farsight_channel_->StopPipeline();
            SAFE_DELETE(farsight_channel_);
        }
    }

    Communication::VoiceSessionInterface::State VoiceSession::GetState() const
    {
        return state_;
    }

    Communication::VoiceSessionParticipantVector VoiceSession::GetParticipants() const
    {
        Communication::VoiceSessionParticipantVector participants;

        for (VoiceSessionParticipantVector::const_iterator i = participants_.begin(); i != participants_.end(); i++)
        {
            VoiceSessionParticipant* participant = *i;
            participants.push_back( participant );
        }
        
        //! @todo IMPLEMENT
        return participants;
    }

    void VoiceSession::Close()
    {
        if (audio_playback_channel_)
            ClosePlaybackChannel();

        state_ = STATE_CLOSED;
        DeleteChannels();
        emit StateChanged(state_);

    }

    void VoiceSession::ClosePlaybackChannel()
    {
        Foundation::Framework* framework = ((Communication::CommunicationService*)(Communication::CommunicationService::GetInstance()))->GetFramework();
        if (!framework)
            return;
        Foundation::ServiceManagerPtr service_manager = framework->GetServiceManager();
        if (!service_manager.get())
            return;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;   

        soundsystem->StopSound(audio_playback_channel_);
        audio_playback_channel_ = 0;
    }

    void VoiceSession::Accept()
    {
        if (state_ != STATE_RINGING_LOCAL || tp_channel_.isNull())
        {
            LogError("Voice session state doesn't allow accept command");
            return;
        }

        tp_channel_->acceptCall();
        connect(tp_channel_.data(),
            SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
            SLOT(OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &)));

        CreateFarsightChannel();
    }

    void VoiceSession::Reject()
    {
        if (state_ != STATE_RINGING_LOCAL || tp_channel_.isNull())
        {
            LogError("Voice session state doesn't allow accept command");
            return;
        }

        tp_channel_->requestClose(); // reject
        state_ = STATE_CLOSED;
        reason_ = "User rejected incoming call.";
        emit StateChanged(state_);
    }

    void VoiceSession::OnOutgoingChannelCreated(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            reason_ = QString("Cannot create connection").append(op->errorMessage());
            LogError(reason_.toStdString());
            emit StateChanged(state_);
            return;
        }

        Tp::PendingChannel *pending_channel = qobject_cast<Tp::PendingChannel *>(op);
        tp_channel_ = Tp::StreamedMediaChannel::create(pending_channel->connection(),pending_channel->objectPath(), pending_channel->immutableProperties());
        connect(tp_channel_->becomeReady(),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnOutgoingChannelReady(Tp::PendingOperation*) ));
    }

    void VoiceSession::OnIncomingChannelReady(Tp::PendingOperation *op)
    {
        Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);

        tp_channel_ = Tp::StreamedMediaChannelPtr(qobject_cast<Tp::StreamedMediaChannel *>(pr->object()));

        if (op->isError())
        {
            state_ = STATE_ERROR;
            QString message = QString("Incoming streamed media channel cannot become ready: ").append(op->errorMessage());
            LogError(message.toStdString());
            reason_ = message;
            emit StateChanged(state_);
            return;
        }
        connect(tp_channel_.data(), SIGNAL( invalidated(Tp::DBusProxy *, const QString &, const QString &) ), SLOT( OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &) ));

        tp_contact_ = tp_channel_->initiatorContact();

        Contact* contact = new Contact(tp_contact_);   // HACK: The contact should be get from Connection object (now we don't delete it!!!)
        VoiceSessionParticipant* participant = new VoiceSessionParticipant(contact);
        participants_.push_back(participant);
        QString id = tp_contact_->id();

        state_ = STATE_RINGING_LOCAL;
        emit StateChanged(state_);

        emit Ready(this);
    }

    void VoiceSession::OnOutgoingChannelReady(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            emit StateChanged(state_);
            return;
        }
        connect(tp_channel_.data(), SIGNAL( invalidated(Tp::DBusProxy *, const QString &, const QString &) ), SLOT( OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &) ));
        
        Tp::ContactManager *cm = tp_channel_->connection()->contactManager();
        tp_contact_ = cm->lookupContactByHandle(tp_channel_->targetHandle());
        QString id = tp_contact_->id();

        CreateFarsightChannel();

        if (state_ == STATE_INITIALIZING)
        {
            state_ = STATE_RINGING_REMOTE;
            emit StateChanged(state_);
        }
    }

    void VoiceSession::CreateFarsightChannel()
    {
        try
        {
            // todo: for linux use "autoaudiosrc" for audio_src_name
            //       CURRENT IMPLEMENTATION WORKS ONLY ON WINDOWS
            farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "autovideosrc", "autovideosink");
            if ( !farsight_channel_->IsAudioSupported() )
            {
                SAFE_DELETE(farsight_channel_);
                QString message = QString("Cannot initialize audio features.");
                reason_ = message;
                LogError(message.toStdString());
                state_ = STATE_ERROR;
                emit StateChanged(state_);
                return;
            }
        }
        catch(Exception &e) 
        {
            QString message = QString("Cannot create FarsightChannel object - ").append(e.what());
            reason_ = message;
            LogError(message.toStdString());
            state_ = STATE_ERROR;
            emit StateChanged(state_);
            return;
        }

        connect( farsight_channel_, SIGNAL(AudioDataAvailable(int)), SLOT( OnFarsightAudioDataAvailable(int ) ), Qt::QueuedConnection );
        connect( farsight_channel_, SIGNAL(AudioBufferOverflow(int)), SLOT( OnFarsightAudioBufferOverflow(int ) ), Qt::QueuedConnection );

        connect(tp_channel_->becomeReady(Tp::StreamedMediaChannel::FeatureStreams),
             SIGNAL( finished(Tp::PendingOperation*) ),
             SLOT( OnStreamFeatureReady(Tp::PendingOperation*) ));

        connect(farsight_channel_,
            SIGNAL(StatusChanged(TelepathyIM::FarsightChannel::Status)),
            SLOT(OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status)),  Qt::QueuedConnection);

        connect(farsight_channel_,
            SIGNAL( AudioStreamReceived() ),
            SLOT( OnFarsightChannelAudioStreamReceived() ),  Qt::QueuedConnection);

        connect(farsight_channel_,
            SIGNAL( VideoStreamReceived() ),
            SLOT( OnFarsightChannelVideoStreamReceived() ), Qt::QueuedConnection);
    }

    void VoiceSession::OnStreamFeatureReady(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            QString error_message = "Stream feature cannot become ready!";
            reason_ = error_message;
            LogError(error_message.toStdString());
            emit StateChanged(state_);
            return;
        }
        state_ = STATE_OPEN;

        connect(tp_channel_.data(), SIGNAL( streamAdded(const Tp::MediaStreamPtr &) ), SLOT( OnStreamAdded(const Tp::MediaStreamPtr &) ));
        connect(tp_channel_.data(), SIGNAL( streamRemoved(const Tp::MediaStreamPtr &) ), SLOT( OnStreamRemoved(const Tp::MediaStreamPtr &) ));
        connect(tp_channel_.data(), SIGNAL( streamDirectionChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamDirection, Tp::MediaStreamPendingSend) ), SLOT( OnStreamDirectionChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamDirection, Tp::MediaStreamPendingSend) ));
        connect(tp_channel_.data(), SIGNAL( streamStateChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamState) ), SLOT( OnStreamStateChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamState) ));

        Tp::MediaStreams streams = tp_channel_->streams();
        for(Tp::MediaStreams::iterator i = streams.begin(); i != streams.end(); ++i)
        {
            Tp::MediaStreamPtr stream = *i;

            QString type = "UNKNOWN";
            switch(stream->type())
            {
            case Tp::MediaStreamTypeAudio:
                type = "AUDIO";
                break;
            case Tp::MediaStreamTypeVideo:
                type = "VIDEO";
                break;
            }

            QString direction = "unknown";
            
            switch(stream->direction())
            {
            case Tp::MediaStreamDirectionNone:
                direction = "None";
                break;
            case Tp::MediaStreamDirectionSend:
                direction = "Send";
                break;
            case Tp::MediaStreamDirectionReceive:
                direction = "Receive";
                break;
            case Tp::MediaStreamDirectionBidirectional:
                direction = "Bidirectional";
                break;
            }
            
            QString log_message = type.append(" stream is ready: direction is ").append(direction);
            LogDebug(log_message.toStdString());

            OnStreamDirectionChanged(stream, stream->direction(), stream->pendingSend());
            OnStreamStateChanged(stream, stream->state());
        }

        Tp::MediaStreamPtr audio_stream = GetAudioMediaStream();
        Tp::MediaStreamPtr video_stream = GetVideoMediaStream();

        // Create automatically audio stream and start sending audio data
        if ( pending_audio_streams_ == 0 && audio_stream.isNull() )
            CreateAudioStream();
        else
        {
            if (!audio_stream.isNull())
                UpdateStreamDirection(audio_stream, true);
        }

        emit StateChanged(state_);
    }

    void VoiceSession::OnChannelInvalidated(Tp::DBusProxy *proxy, const QString &error, const QString &message)
    {
        QString log_message = QString(" VoiceSession::OnChannelInvalidated - ").append(error).append(" - ").append(message);
        LogInfo(log_message.toStdString());
        state_ = STATE_CLOSED;
        reason_ = message;
        emit StateChanged(state_);
    }

    void VoiceSession::CreateAudioStream() 
    {
        if (pending_audio_streams_ )
            return;

        if (!tp_channel_)
        {
            LogError("Cannot create audio stream.");
            return;
        }
        pending_audio_streams_ = tp_channel_->requestStream(tp_contact_, Tp::MediaStreamTypeAudio);
        connect(pending_audio_streams_, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnAudioStreamCreated(Tp::PendingOperation*) ));
    }

    void VoiceSession::CreateVideoStream() 
    {
        if (pending_video_streams_ )
            return;

        if (!tp_channel_)
        {
            LogError("Cannot create audio stream.");
            return;
        }
        pending_video_streams_ = tp_channel_->requestStream(tp_contact_, Tp::MediaStreamTypeVideo);
        connect(pending_video_streams_, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnVideoStreamCreated(Tp::PendingOperation*) ));
    }

    void VoiceSession::OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status status)
    {
        emit ReceivingAudioData(IsReceivingAudioData());
        emit ReceivingVideoData(IsReceivingVideoData());
        emit SendingAudioData(IsReceivingAudioData());
        emit SendingVideoData(IsReceivingVideoData());
 
        switch (status)
        {
        case FarsightChannel::StatusConnecting:
            LogInfo("VoiceSession: FarsightChannel status = Connecting...");
            break;
        case FarsightChannel::StatusConnected:
            LogInfo("VoiceSession: FarsightChannel status = Connected.");
            //state_ = STATE_OPEN;
            //reason_ = "";
            //emit StateChanged(state_);
            break;
        case FarsightChannel::StatusDisconnected:
            LogInfo("VoiceSession: Call FarsightChannel status = terminated.");
            state_ = STATE_CLOSED;
            tp_channel_->requestClose();
            emit StateChanged(state_);
            break;
        }
    }

    void VoiceSession::UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send)
    {
        QString type = "UNKNOWN";
        if (stream->type() == Tp::MediaStreamTypeAudio)
            type = "AUDIO";
        if (stream->type() == Tp::MediaStreamTypeVideo)
            type = "VIDEO";

        if (send)
        {
            if (!(stream->direction() & Tp::MediaStreamDirectionSend))
            {
                QString log_message = QString("Change ").append(type).append("stream direction to: DirectionSend");
                LogDebug(log_message.toStdString());
                int dir = stream->direction() | Tp::MediaStreamDirectionSend;
                stream->requestDirection((Tp::MediaStreamDirection) dir);
            }
        }
        else
        {
            if (stream->direction() & Tp::MediaStreamDirectionSend) 
            {
                QString log_message = QString("Change ").append(type).append("stream direction to: ~DirectionSend");
                LogDebug(log_message.toStdString());
                int dir = stream->direction() & ~Tp::MediaStreamDirectionSend;
                stream->requestDirection((Tp::MediaStreamDirection) dir);
            }
        }
    }

    void VoiceSession::OnStreamAdded(const Tp::MediaStreamPtr &stream)
    {
        if (stream->type() == Tp::MediaStreamTypeAudio)
        {
            LogDebug("Added AUDIO STREAM");
        }

        if (stream->type() == Tp::MediaStreamTypeVideo)
        {
            LogDebug("Added VIDEO STREAM");
        }

        if (stream->type() == Tp::MediaStreamTypeAudio)
            UpdateStreamDirection(stream, true);
        else
            UpdateStreamDirection(stream, stream->direction());

        OnStreamDirectionChanged(stream, stream->direction(), stream->pendingSend());
        OnStreamStateChanged(stream, stream->state());

        //state_ = STATE_OPEN;
        //emit StateChanged(state_);


        // TODO: Shoud we handle these ?
        //stream->localSendingRequested();
        //remoteSendingRequested
    }

    void VoiceSession::OnStreamRemoved(const Tp::MediaStreamPtr &stream)
    {
        if (stream->type() == Tp::MediaStreamTypeAudio)
        {
            LogDebug("Removed AUDIO STREAM");
            emit AudioStreamStateChanged(SS_DISCONNECTED);
        }

        if (stream->type() == Tp::MediaStreamTypeVideo)
        {
            LogDebug("Removed VIDEO STREAM");
            emit VideoStreamStateChanged(SS_DISCONNECTED);
        }
    }

    void VoiceSession::OnStreamDirectionChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamDirection direction, Tp::MediaStreamPendingSend ps)
    {
        // Note we do not use value of ps. Instead we use MediaStream interface to obtain that information.

        switch(stream->type())
        {
        case Tp::MediaStreamTypeAudio:
            emit SendingAudioData(IsSendingAudioData());
            emit ReceivingAudioData(IsReceivingAudioData());
            if (stream->localSendingRequested())
            {
                LogDebug("Audio send requested.");
                // todo send audio 
            }
            break;
        case Tp::MediaStreamTypeVideo:
            emit SendingVideoData(IsSendingVideoData());
            emit ReceivingVideoData(IsReceivingVideoData());
            if (stream->localSendingRequested())
            {
                LogDebug("Video send requested.");
                // todo send audio 
            }
            break;
        }

        QString type = "UNKNOWN";
        if (stream->type() == Tp::MediaStreamTypeAudio)
            type="AUDIO";
        if (stream->type() == Tp::MediaStreamTypeVideo)
            type="VIDEO";

        //if (ps.testFlag(Tp::MediaStreamPendingLocalSend)) 
        //{
        //    // todo: implementation
        //}

        //if (ps.testFlag(Tp::MediaStreamPendingRemoteSend)) 
        //{
        //    // todo: implementation
        //}

        //QString log_message = type;
        //bool new_state = false;
        //if (direction == Tp::MediaStreamDirectionSend)
        //{
        //    audio_in_enabled_ = false;
        //    audio_out_enabled_ = true;
        //    log_message.append(" stream direction changed to: SEND");

        //} else if (direction == Tp::MediaStreamDirectionReceive)
        //{
        //    audio_in_enabled_ = true;
        //    audio_out_enabled_ = false;
        //    log_message.append(" stream direction changed to: RECEIVE");

        //} else if (direction == (Tp::MediaStreamDirectionSend | Tp::MediaStreamDirectionReceive))
        //{
        //    audio_in_enabled_ = true;
        //    audio_out_enabled_ = true;
        //    log_message.append(" stream direction changed to: SEND & RECEIVE");
        //} else
        //{
        //    audio_in_enabled_ = false;
        //    audio_out_enabled_ = false;
        //    log_message.append(" stream direction changed to: NONE");
        //}
        //LogDebug(log_message.toStdString());

        //if (stream->type() == Tp::MediaStreamTypeAudio)
        //    emit ReceivingAudioData(new_state);
        //if (stream->type() == Tp::MediaStreamTypeVideo)
        //    emit ReceivingVideoData(new_state);
    }

    void VoiceSession::OnStreamStateChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamState state)
    {
        QString type = "UNKNOWN";
        if (stream->type() == Tp::MediaStreamTypeAudio)
            type="AUDIO";
        if (stream->type() == Tp::MediaStreamTypeVideo)
            type="VIDEO";

        QString log_message = type;

        switch(state)
        {
        case Tp::MediaStreamStateDisconnected:
            if (stream->type() == Tp::MediaStreamTypeAudio)
                emit AudioStreamStateChanged(SS_DISCONNECTED);
            if (stream->type() == Tp::MediaStreamTypeVideo)
                emit VideoStreamStateChanged(SS_DISCONNECTED);
            log_message.append(" stream state changed to: DISCONNECT");
            break;
        case Tp::MediaStreamStateConnecting:
            if (stream->type() == Tp::MediaStreamTypeAudio)
                emit AudioStreamStateChanged(SS_CONNECTING);
            if (stream->type() == Tp::MediaStreamTypeVideo)
                emit VideoStreamStateChanged(SS_CONNECTING);
            log_message.append(" stream state changed to: CONNECTING...");
            break;
        case Tp::MediaStreamStateConnected:
            if (stream->type() == Tp::MediaStreamTypeAudio)
                emit AudioStreamStateChanged(SS_CONNECTED);
            if (stream->type() == Tp::MediaStreamTypeVideo)
                emit VideoStreamStateChanged(SS_CONNECTED);
            log_message.append(" stream state changed to: CONNECTED");
            break;
        }
        LogDebug(log_message.toStdString());
    }

    void VoiceSession::OnAudioStreamCreated(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            QString error = "Cannot create AUDIO stream: ";
            error.append( op->errorMessage() );
            LogError(error.toStdString());
            return;
        }
        pending_audio_streams_ = 0;
        LogDebug("AUDIO stream created.");

        // do nothing as OnStreamAdded signal will be emitted
    }

    void VoiceSession::OnVideoStreamCreated(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            QString error = "Cannot create VIDEO stream: ";
            error.append( op->errorMessage() );
            LogError(error.toStdString());
            return;
        }
        pending_video_streams_ = 0;
        LogDebug("VIDEO stream created.");

        // do nothing as OnStreamAdded signal will be emitted
    }

    Tp::MediaStreamPtr VoiceSession::GetAudioMediaStream() const
    {
        if (!tp_channel_)
            return Tp::MediaStreamPtr();
        Tp::MediaStreams streams = tp_channel_->streams();
        foreach (const Tp::MediaStreamPtr &stream, streams) {
            if (stream->type() == Tp::MediaStreamTypeAudio)
                return stream;
        }
        return Tp::MediaStreamPtr();
    }

    Tp::MediaStreamPtr VoiceSession::GetVideoMediaStream() const
    {
        if (!tp_channel_)
            return Tp::MediaStreamPtr();
        Tp::MediaStreams streams = tp_channel_->streams();
        foreach (const Tp::MediaStreamPtr &stream, streams) {
            if (stream->type() == Tp::MediaStreamTypeVideo)
                return stream;
        }
        return Tp::MediaStreamPtr();
    }

    Communication::VideoPlaybackWidgetInterface* VoiceSession::GetReceivedVideo()
    {
        if (farsight_channel_)
            return farsight_channel_->GetReceivedVideoWidget();
        return 0;
    }

    Communication::VideoPlaybackWidgetInterface* VoiceSession::GetLocallyCapturedVideo()
    {
        if (farsight_channel_)
        {
            return farsight_channel_->GetLocallyCapturedVideoWidget();
        }
        return 0;
    }

    void VoiceSession::OnFarsightAudioDataAvailable(int count)
    {       
        if (count < AUDIO_BUFFER_PLAYBACK_MIN_SIZE)
            return;

        Foundation::Framework* framework = ((Communication::CommunicationService*)(Communication::CommunicationService::GetInstance()))->GetFramework();
        if (!framework)
            return;
        Foundation::ServiceManagerPtr service_manager = framework->GetServiceManager();
        if (!service_manager.get())
            return;
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;     
        if (!farsight_channel_)
            return;

        int size = farsight_channel_->GetAudioData(audio_buffer, AUDIO_BUFFER_SIZE);   

        bool stereo = false;
        int channel_count = farsight_channel_->GetChannelCount();
        if (channel_count == 2)
            stereo = true;
        int sample_width = farsight_channel_->GetSampleWidth();
        int sample_rate = farsight_channel_->GetSampleRate();

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        sound_buffer.data_ = audio_buffer;
        sound_buffer.frequency_ = sample_rate;
        if (sample_width == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        sound_buffer.size_ = size;
        sound_buffer.stereo_ = stereo;
        if (size > 0 && sample_rate != -1 && sample_width != -1 && (channel_count == 1 || channel_count == 2) )
        {
            if (spatial_audio_playback_)
                audio_playback_channel_ = soundsystem->PlaySoundBuffer3D(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_position_, audio_playback_channel_);
            else
                audio_playback_channel_ = soundsystem->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channel_);
        }
    }

    void VoiceSession::OnFarsightAudioBufferOverflow(int count)
    {
        QString message = QString("Farsight audio buffer overflow. %1 bytes lost.").arg(QString::number(count));
        LogDebug(message.toStdString());
    }

    Communication::VoiceSessionInterface::StreamState VoiceSession::GetAudioStreamState() const
    {
        Tp::MediaStreamPtr stream = GetAudioMediaStream();
        if (!stream)
            return Communication::VoiceSessionInterface::SS_DISCONNECTED;
        switch( stream->state() )
        {
        case Tp::MediaStreamStateDisconnected:
            return Communication::VoiceSessionInterface::SS_DISCONNECTED;
        case Tp::MediaStreamStateConnecting:
            return Communication::VoiceSessionInterface::SS_CONNECTING;
        case Tp::MediaStreamStateConnected:
            return Communication::VoiceSessionInterface::SS_CONNECTED;
        }
        return Communication::VoiceSessionInterface::SS_DISCONNECTED;
    }

    Communication::VoiceSessionInterface::StreamState VoiceSession::GetVideoStreamState() const
    {
        Tp::MediaStreamPtr stream = GetVideoMediaStream();
        if (!stream)
            return Communication::VoiceSessionInterface::SS_DISCONNECTED;
        switch( stream->state() )
        {
        case Tp::MediaStreamStateDisconnected:
            return Communication::VoiceSessionInterface::SS_DISCONNECTED;
        case Tp::MediaStreamStateConnecting:
            return Communication::VoiceSessionInterface::SS_CONNECTING;
        case Tp::MediaStreamStateConnected:
            return Communication::VoiceSessionInterface::SS_CONNECTED;
        }
        return Communication::VoiceSessionInterface::SS_DISCONNECTED;
    }

    bool VoiceSession::IsSendingAudioData() const
    {
        if (!farsight_channel_)
            return false;

        if (farsight_channel_->GetStatus() != FarsightChannel::StatusConnected)
            return false;

        Tp::MediaStreamPtr stream = GetAudioMediaStream();
        if (!stream)
            return false;

        return stream->sending();
    }

    bool VoiceSession::IsSendingVideoData() const
    {
        if (!farsight_channel_)
            return false;

        if (farsight_channel_->GetStatus() != FarsightChannel::StatusConnected)
            return false;

        Tp::MediaStreamPtr stream = GetVideoMediaStream();
        if (!stream)
            return false;

        return stream->sending();
    }

    bool VoiceSession::IsReceivingAudioData() const
    {
        if (!farsight_channel_)
            return false;

        if (farsight_channel_->GetStatus() != FarsightChannel::StatusConnected)
            return false;

        if (GetState() != STATE_OPEN)
            return false;

        Tp::MediaStreamPtr stream = GetAudioMediaStream();
        if (!stream)
            return false;

        if (!farsight_channel_->IncomingAudioStreamConnected())
            return false;

        return stream->receiving();
    }

    bool VoiceSession::IsReceivingVideoData() const
    {
        if (!farsight_channel_)
            return false;

        if (farsight_channel_->GetStatus() != FarsightChannel::StatusConnected)
            return false;

        if (GetState() != STATE_OPEN)
            return false;

        Tp::MediaStreamPtr stream = GetVideoMediaStream();
        if (!stream)
            return false;

        if (!farsight_channel_->IncomingVideoStreamConnected())
            return false;

        return stream->receiving();
    }

    void VoiceSession::SendAudioData(bool send)
    {
        Tp::MediaStreamPtr audio_stream = GetAudioMediaStream();
        if (audio_stream)
            UpdateStreamDirection(audio_stream, send);
        else
            if (send)
                CreateAudioStream();
    }

    void VoiceSession::SendVideoData(bool send)
    {
        Tp::MediaStreamPtr video_stream = GetVideoMediaStream();
        if (video_stream)
            UpdateStreamDirection(video_stream, send);
        else
            if (send)
                CreateVideoStream();
    }

    void VoiceSession::UpdateAudioSourcePosition(Vector3df position)
    {
        if (!spatial_audio_playback_)
        {
            ClosePlaybackChannel();
            spatial_audio_playback_ = true;
        }
        audio_playback_position_ = position;
    }

    void VoiceSession::TrackingAvatar(bool enabled)
    {
        if (enabled)
            qDebug() << "Voice Session >> Spatial tracking enabled";
        else
            qDebug() << "Voice Session >> Spatial tracking disabled";
        positional_voice_enabled_ = enabled;
    }

    void VoiceSession::OnFarsightChannelAudioStreamReceived()
    {
        emit ReceivingAudioData(true);
    }

    void VoiceSession::OnFarsightChannelVideoStreamReceived()
    {
        emit ReceivingVideoData(true);
    }

} // end of namespace: TelepathyIM
