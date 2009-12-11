// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <TelepathyQt4/ReferencedHandles>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingChannel>
#include "VoiceSession.h"


namespace TelepathyIM
{
	
	VoiceSession::VoiceSession(const Tp::StreamedMediaChannelPtr &tp_channel): state_(STATE_INITIALIZING), tp_channel_(tp_channel), pending_audio_streams_(0), farsight_channel_(0), pending_video_streams_(0)
	{
        state_ = STATE_RINGING_LOCAL;

        connect(tp_channel_->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(OnIncomingChannelReady(Tp::PendingOperation*)));
	}

	VoiceSession::VoiceSession(Tp::ContactPtr tp_contact): state_(STATE_INITIALIZING), tp_channel_(0), pending_audio_streams_(0), farsight_channel_(0), pending_video_streams_(0)
	{
        state_ = STATE_RINGING_REMOTE;
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
        if (tp_channel_)
        {
            tp_channel_->requestClose();
        }
    }

	Communication::VoiceSessionInterface::State VoiceSession::GetState() const
	{
		return state_;
	}

	Communication::VoiceSessionParticipantVector VoiceSession::GetParticipants() const
	{
		Communication::VoiceSessionParticipantVector empty_vector;
        
		//! @todo IMPLEMENT
		return empty_vector;
	}

	void VoiceSession::Close()
	{
        state_ = STATE_CLOSED;
        if ( !tp_channel_.isNull())
		    Tp::PendingOperation* op = tp_channel_->requestClose();
	}

    //void VoiceSession::Accept()
    //{
    //    emit AcceptSignal();
    //}

    void VoiceSession::Accept()
    {
        if (state_ != STATE_RINGING_LOCAL || tp_channel_.isNull())
        {
            LogError("Voice session state doesn't allow accept command");
            return;
        }

        tp_channel_->acceptCall();

        try
        {
            farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "directsoundsink", "autovideosrc");  // AUTO VIDEO
            //farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "directsoundsink", "videotestsrc");     // TEST VIDEO
        }
        catch(Core::Exception &e) 
        {
            QString message = QString("Cannot create FarsightChannel object - ").append(e.what());
            LogError(message.toStdString());
            state_ = STATE_ERROR;
            emit StateChanged(state_);
            return;
        }

	    connect(tp_channel_->becomeReady(Tp::StreamedMediaChannel::FeatureStreams),
             SIGNAL( finished(Tp::PendingOperation*) ),
             SLOT( OnStreamFeatureReady(Tp::PendingOperation*) ));

        connect(tp_channel_.data(),
            SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
            SLOT(OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &)));

        connect(farsight_channel_,
            SIGNAL(statusChanged(TelepathyIM::FarsightChannel::Status)),
            SLOT(OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status)));
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
    }

	void VoiceSession::OnOutgoingChannelCreated(Tp::PendingOperation *op)
	{
	    if (op->isError())
		{
            state_ = STATE_ERROR;
            reason_ = QString("Cannot create connection").append(op->errorMessage());
            LogError(reason_.toStdString());
            emit (Closed(this));
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
            emit (Closed(this));
			return;
		}

		tp_contact_ = tp_channel_->initiatorContact();
        QString id = tp_contact_->id();

		emit Ready(this);
	}

	void VoiceSession::OnOutgoingChannelReady(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			return;
		}
		
		Tp::ContactManager *cm = tp_channel_->connection()->contactManager();
		tp_contact_ = cm->lookupContactByHandle(tp_channel_->targetHandle());
        QString id = tp_contact_->id();

        farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "directsoundsink", "autovideosrc");

	    connect(tp_channel_->becomeReady(Tp::StreamedMediaChannel::FeatureStreams),
             SIGNAL( finished(Tp::PendingOperation*) ),
             SLOT( OnStreamFeatureReady(Tp::PendingOperation*) ));

        connect(tp_channel_.data(),
            SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
            SLOT(OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &)));

        connect(farsight_channel_,
            SIGNAL(statusChanged(FarsightChannel::Status)),
            SLOT(OnFarsightChannelStatusChanged(FarsightChannel::Status)));
	}

    void VoiceSession::OnStreamFeatureReady(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            LogError("Stream feature cannot become ready!");
            return;
        }
        state_ = STATE_OPEN;
        connect(tp_channel_.data(), SIGNAL( streamAdded(const Tp::MediaStreamPtr &) ), SLOT( OnStreamAdded(const Tp::MediaStreamPtr &) ));
        connect(tp_channel_.data(), SIGNAL( streamRemoved(const Tp::MediaStreamPtr &) ), SLOT( OnStreamRemoved(const Tp::MediaStreamPtr &) ));
        connect(tp_channel_.data(), SIGNAL( streamDirectionChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamDirection, Tp::MediaStreamPendingSend) ),
                                    SLOT( OnStreamDirectionChanged(const Tp::MediaStreamPtr &, Tp::MediaStreamDirection, Tp::MediaStreamPendingSend) ));
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

        if ( pending_audio_streams_ == 0 && audio_stream.isNull() )
            CreateAudioStream();
        else
            UpdateStreamDirection(audio_stream, true);

        //if ( pending_video_streams_ == 0 && video_stream.isNull() )
        //    CreateVideoStream();
        //else
        //    UpdateStreamDirection(video_stream, true);

        emit ( Opened(this) );
    }

    void VoiceSession::OnChannelInvalidated(Tp::DBusProxy *proxy, const QString &error, const QString &message)
    {
        QString log_message = QString(" VoiceSession::OnChannelInvalidated - ").append(error).append(" - ").append(message);
        LogInfo(log_message.toStdString());
        state_ = STATE_ERROR;
        reason_ = message;
        emit Closed(this);
    }

    void VoiceSession::CreateAudioStream()
    {
        if (pending_audio_streams_ )
            return;

        pending_audio_streams_ = tp_channel_->requestStream(tp_contact_, Tp::MediaStreamTypeAudio);
        connect(pending_audio_streams_, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnAudioStreamCreated(Tp::PendingOperation*) ));
    }

    void VoiceSession::CreateVideoStream()
    {
        if (pending_video_streams_ )
            return;

        pending_video_streams_ = tp_channel_->requestStream(tp_contact_, Tp::MediaStreamTypeVideo);
        connect(pending_video_streams_, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnVideoStreamCreated(Tp::PendingOperation*) ));
    }

    void VoiceSession::OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status status)
    {
        switch (status) {
        case FarsightChannel::StatusConnecting:
            LogInfo("VoiceSession: FarsightChannel status = Connecting...");
            break;
        case FarsightChannel::StatusConnected:
            LogInfo("VoiceSession: FarsightChannel status = Connected.");
            break;
        case FarsightChannel::StatusDisconnected:
            LogInfo("VoiceSession: Call FarsightChannel status = terminated.");
            state_ = STATE_CLOSED;
            tp_channel_->requestClose();
            emit Closed(this);
            break;
        }
    }

    void VoiceSession::UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send)
    {
        QString type = "UNKNOWN";
        if (stream->type() == Tp::MediaStreamTypeAudio);
            type = "AUDIO";
        if (stream->type() == Tp::MediaStreamTypeVideo);
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

        UpdateStreamDirection(stream, true);
        OnStreamDirectionChanged(stream, stream->direction(), stream->pendingSend());
        OnStreamStateChanged(stream, stream->state());
    }

    void VoiceSession::OnStreamRemoved(const Tp::MediaStreamPtr &stream)
    {
        if (stream->type() == Tp::MediaStreamTypeAudio)
        {
            LogDebug("Removed AUDIO STREAM");
        }

        if (stream->type() == Tp::MediaStreamTypeVideo)
        {
            LogDebug("Removed VIDEO STREAM");
        }
    }

    void VoiceSession::OnStreamDirectionChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamDirection direction, Tp::MediaStreamPendingSend ps)
    {
        QString type = "UNKNOWN";
        if (stream->type() == Tp::MediaStreamTypeAudio)
            type="AUDIO";
        if (stream->type() == Tp::MediaStreamTypeVideo)
            type="VIDEO";

        QString log_message = type;
        if (direction == Tp::MediaStreamDirectionSend)
        {
            log_message.append(" stream direction changed to: SEND");

        } else if (direction == Tp::MediaStreamDirectionReceive)
        {
            log_message.append(" stream direction changed to: RECEIVE");

        } else if (direction == (Tp::MediaStreamDirectionSend | Tp::MediaStreamDirectionReceive))
        {
            log_message.append(" stream direction changed to: SEND & RECEIVE");
        } else
        {
            log_message.append(" stream direction changed to: NONE");
        }
        LogDebug(log_message.toStdString());
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
            log_message.append(" stream state changed to: DISCONNECT");
            break;
        case Tp::MediaStreamStateConnecting:
            log_message.append(" stream state changed to: CONNECTING...");
            break;
        case Tp::MediaStreamStateConnected:
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

        // do nothing as OnStreamAdded signal will be emitted
    }

    Tp::MediaStreamPtr VoiceSession::GetAudioMediaStream()
    {
        Tp::MediaStreams streams = tp_channel_->streams();
        foreach (const Tp::MediaStreamPtr &stream, streams) {
            if (stream->type() == Tp::MediaStreamTypeAudio)
                return stream;
        }
        return Tp::MediaStreamPtr();
    }

    Tp::MediaStreamPtr VoiceSession::GetVideoMediaStream()
    {
        Tp::MediaStreams streams = tp_channel_->streams();
        foreach (const Tp::MediaStreamPtr &stream, streams) {
            if (stream->type() == Tp::MediaStreamTypeVideo)
                return stream;
        }
        return Tp::MediaStreamPtr();
    }

    Communication::VideoWidgetInterface* VoiceSession::GetRemoteVideo()
    {
        if (farsight_channel_)
            return farsight_channel_->GetPreviewVideo();
        return 0;
    }

    Communication::VideoWidgetInterface* VoiceSession::GetOwnVideo()
    {
        if (farsight_channel_)
            return farsight_channel_->GetRemoteOutputVideo();
        return 0;
    }

} // end of namespace: TelepathyIM
