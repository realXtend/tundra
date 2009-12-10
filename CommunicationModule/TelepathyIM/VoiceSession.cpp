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
	
	VoiceSession::VoiceSession(const Tp::StreamedMediaChannelPtr &tp_channel): state_(STATE_INITIALIZING), tp_channel_(tp_channel), pending_audio_streams_(0), farsight_channel_(0)
	{
        state_ = STATE_RINGING_LOCAL;

        connect(tp_channel_->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(OnIncomingChannelReady(Tp::PendingOperation*)));
	}

	VoiceSession::VoiceSession(Tp::ContactPtr tp_contact): state_(STATE_INITIALIZING), tp_channel_(0), pending_audio_streams_(0), farsight_channel_(0)
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

    void VoiceSession::Accept()
    {
        if (state_ != STATE_RINGING_LOCAL || tp_channel_.isNull())
        {
            LogError("Voice session state doesn't allow accept command");
            return;
        }

        tp_channel_->acceptCall();

                                                              
        //farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "directsoundsink", "autovideosrc");  // AUTO VIDEO
        try
        {
            farsight_channel_ = new FarsightChannel(tp_channel_, "dshowaudiosrc", "directsoundsink", "videotestsrc");     // TEST VIDEO
        }
        catch(...) // todo: find out exception types
        {
            LogError("Cannot create FarsightChannel object!");
            state_ = STATE_ERROR;

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

    void VoiceSession::CreateAudioStream()
    {
        if (pending_audio_streams_ )
            return;

        pending_audio_streams_ = tp_channel_->requestStream(tp_contact_, Tp::MediaStreamTypeAudio);
        connect(pending_audio_streams_, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnAudioStreamCreated(Tp::PendingOperation*) ));
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
            
			Tp::MediaStreamType type = (*i)->type();

            // DEBUG...
			switch(type)
			{
			case Tp::MediaStreamTypeAudio:
                LogDebug("Received AUDIO CHANNEL");
                break;
			case Tp::MediaStreamTypeVideo:
                LogDebug("Received VIDEO CHANNEL");
                break;
			}

            OnStreamDirectionChanged(stream, stream->direction(), stream->pendingSend());
            OnStreamStateChanged(stream, stream->state());
		}

        Tp::MediaStreamPtr audio_stream = GetAudioMediaStream();

        if ( pending_audio_streams_ == 0 && audio_stream.isNull() )
            CreateAudioStream();
    }

    void VoiceSession::OnChannelInvalidated(Tp::DBusProxy *proxy, const QString &error, const QString &message)
    {
        LogInfo(" VoiceSession::OnChannelInvalidated(");
        state_ = STATE_ERROR;
        reason_ = message;
        emit Closed(this);
    }

    void VoiceSession::OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status status)
    {
        switch (status) {
        case FarsightChannel::StatusConnecting:
            LogInfo("VoiceSession: Connecting...");
            break;
        case FarsightChannel::StatusConnected:
            LogInfo("VoiceSession: Connected.");
            break;
        case FarsightChannel::StatusDisconnected:
            tp_channel_->requestClose();
            // TODO: emit signal
            LogInfo("VoiceSession: Call terminated.");
            break;
        }
    }

    void VoiceSession::UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send)
    {
        if (send)
        {
            if (!(stream->direction() & Tp::MediaStreamDirectionSend))
            {
                LogDebug("Change stream direction to: DirectionSend");
                int dir = stream->direction() | Tp::MediaStreamDirectionSend;
                stream->requestDirection((Tp::MediaStreamDirection) dir);
                LogInfo("Start sending");
            }
        }
        else
        {
            if (stream->direction() & Tp::MediaStreamDirectionSend) 
            {
                LogDebug("Change stream direction to: ~DirectionSend");
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

        // todo: IMPLEMENT
    }

    void VoiceSession::OnStreamDirectionChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamDirection direction, Tp::MediaStreamPendingSend ps)
    {
        // todo: IMPLEMENT
        if (direction == Tp::MediaStreamDirectionSend)
        {
            LogDebug("Stream direction changed to: SEND");

        } else if (direction == Tp::MediaStreamDirectionReceive)
        {
            LogDebug("Stream direction changed to: RECEIVE");

        } else if (direction == (Tp::MediaStreamDirectionSend | Tp::MediaStreamDirectionReceive))
        {
            LogDebug("Stream direction changed to: SEND & RECEIVE");
        } else
        {
            LogDebug("Stream direction changed to: NONE");
        }
    }

    void VoiceSession::OnStreamStateChanged(const Tp::MediaStreamPtr &strean, Tp::MediaStreamState state)
    {
        // todo: IMPLEMENT
        switch(state)
        {
        case Tp::MediaStreamStateDisconnected:
            LogDebug("Stream state changed to: DISCONNECT");
            break;
        case Tp::MediaStreamStateConnecting:
            LogDebug("Stream state changed to: CONNECTING...");
            break;
        case Tp::MediaStreamStateConnected:
            LogDebug("Stream state changed to: CONNECTED");
            break;
        }
    }

    void VoiceSession::OnAudioStreamCreated(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            QString error = "Cannot create audio stream: ";
            error.append( op->errorMessage() );
            LogError(error.toStdString());
            return;
        }
        pending_audio_streams_ = 0;

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

} // end of namespace: TelepathyIM
