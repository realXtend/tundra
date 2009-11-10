#include <TelepathyQt4/ReferencedHandles>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingChannel>
#include "VoiceSession.h"

namespace TelepathyIM
{
	
	VoiceSession::VoiceSession(Tp::StreamedMediaChannelPtr tp_channel): state_(STATE_INITIALIZING), tp_channel_(tp_channel)
	{
	    connect(tp_channel->becomeReady(Tp::StreamedMediaChannel::FeatureStreams),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnIncomingChannelReady(Tp::PendingOperation*) ));
	}

	VoiceSession::VoiceSession(Tp::ContactPtr tp_contact): state_(STATE_INITIALIZING)
	{
	    QVariantMap request;
		request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA);
		request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), (uint) Tp::HandleTypeContact);
		request.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"), tp_contact->handle().at(0));

		Tp::ConnectionPtr tp_connection = tp_contact->manager()->connection();
		connect(tp_connection->ensureChannel(request),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnOutgoingChannelCreated(Tp::PendingOperation*) ));
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
		//Tp::PendingOperation* op = tp_channel_->requestClose();
		//! @todo IMPLEMENT
	}

	void VoiceSession::OnOutgoingChannelCreated(Tp::PendingOperation *op)
	{
		//! @todo IMPLEMENT
	    if (op->isError())
		{
			return;
		}

		Tp::PendingChannel *pending_channel = qobject_cast<Tp::PendingChannel *>(op);
		tp_channel_ = Tp::StreamedMediaChannel::create(pending_channel->connection(),pending_channel->objectPath(), pending_channel->immutableProperties());
	    connect(tp_channel_->becomeReady(Tp::StreamedMediaChannel::FeatureStreams),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnOutgoingChannelReady(Tp::PendingOperation*) ));
	}

	void VoiceSession::OnIncomingChannelReady(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			return;
		}

		Tp::ContactPtr contact = tp_channel_->initiatorContact();
		//! @todo IMPLEMENT

		// tp_channel_->acceptCall();
		// tp_channel_->requestClose();
	}

	void VoiceSession::OnOutgoingChannelReady(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			return;
		}
		//! @todo IMPLEMENT
		Tp::ContactManager *cm = tp_channel_->connection()->contactManager();
		Tp::ContactPtr contact = cm->lookupContactByHandle(tp_channel_->targetHandle());
		Q_ASSERT(contact);


		Tp::MediaStreams streams = tp_channel_->streams();
		for(Tp::MediaStreams::iterator i = streams.begin(); i != streams.end(); ++i)
		{
			Tp::MediaStreamType type = (*i)->type();
			switch(type)
			{
			case Tp::MediaStreamTypeAudio: break;
			case Tp::MediaStreamTypeVideo: break;
			}
		}
	}

} // end of namespace: TelepathyIM
