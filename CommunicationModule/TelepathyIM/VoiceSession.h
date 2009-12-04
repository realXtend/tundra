#ifndef incl_Communication_TelepathyIM_VoiceSession_h
#define incl_Communication_TelepathyIM_VoiceSession_h

//#include <TelepathyQt4/Types>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/PendingReady>

#include "../interface.h"
#include "Contact.h"
#include "FarsightChannel.h"

namespace TelepathyIM
{
	/**
	 *       
	 */
	class VoiceSession : public Communication::VoiceSessionInterface
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	public:
		VoiceSession(Tp::StreamedMediaChannelPtr tp_channel);
		VoiceSession(Tp::ContactPtr tp_contact);

		//! @return State of the session
		virtual Communication::VoiceSessionInterface::State GetState() const;

		//! @return all known participants of the chat session inlcuding the user
		virtual Communication::VoiceSessionParticipantVector GetParticipants() const;

		virtual void Close();
	protected:
		State state_;
		Tp::StreamedMediaChannelPtr tp_channel_;
        FarsightChannel* farsight_channel_;

	protected slots:
		void OnOutgoingChannelCreated(Tp::PendingOperation *op);
		void OnIncomingChannelReady(Tp::PendingOperation *op);
		void OnOutgoingChannelReady(Tp::PendingOperation *op);
        void OnStreamFeatureReady(Tp::PendingOperation* op);
        void OnStreamAdded(const Tp::MediaStreamPtr &media_stream);
        void OnStreamRemoved(const Tp::MediaStreamPtr &media_stream);
        void OnStreamDirectionChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamDirection direction, Tp::MediaStreamPendingSend ps);
        void OnStreamStateChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamState state);

    };

    typedef std::vector<VoiceSession*> VoiceSessionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VoiceSession_h
