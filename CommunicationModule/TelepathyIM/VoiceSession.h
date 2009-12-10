#ifndef incl_Communication_TelepathyIM_VoiceSession_h
#define incl_Communication_TelepathyIM_VoiceSession_h

//#include <TelepathyQt4/Types>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/PendingReady>

#include "interface.h"
#include "Contact.h"
#include "FarsightChannel.h"
#include "VoiceSessionParticipant.h"

namespace TelepathyIM
{
	/**
	 *  Uses Tp::StreamedMediaChannel for communicating with dbus       
	 */
	class VoiceSession : public Communication::VoiceSessionInterface
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	public:
		VoiceSession(const Tp::StreamedMediaChannelPtr &tp_channel);
		VoiceSession(Tp::ContactPtr tp_contact);
        virtual ~VoiceSession();

		//! @return State of the session
		virtual Communication::VoiceSessionInterface::State GetState() const;

		//! @return all known participants of the chat session inlcuding the user
		virtual Communication::VoiceSessionParticipantVector GetParticipants() const;

		virtual void Close();

        virtual void Accept();

        virtual void Reject();

        //virtual void EnableAudioSending(bool value);
        //virtual void EnableVideoSending(bool value);

	protected:
        void UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send);
        void CreateAudioStream();
        void CreateVideoStream();
        QString GetReadon() { return reason_; };
        Tp::MediaStreamPtr GetAudioMediaStream();
        Tp::MediaStreamPtr GetVideoMediaStream(); // video
        

		State state_;
		Tp::StreamedMediaChannelPtr tp_channel_;
        FarsightChannel* farsight_channel_;

        Tp::PendingMediaStreams *pending_audio_streams_;
//        Tp::MediaStreamPtr audio_stream_;
        Tp::ContactPtr tp_contact_;
        QString reason_;      
        VoiceSessionParticipantVector participants_;

	protected slots:
        void OnChannelInvalidated(Tp::DBusProxy *proxy, const QString &error, const QString &message);
        void OnFarsightChannelStatusChanged(TelepathyIM::FarsightChannel::Status status);
		void OnOutgoingChannelCreated(Tp::PendingOperation *op);
		void OnIncomingChannelReady(Tp::PendingOperation *op);
		void OnOutgoingChannelReady(Tp::PendingOperation *op);
        void OnStreamFeatureReady(Tp::PendingOperation* op);
        void OnStreamAdded(const Tp::MediaStreamPtr &media_stream);
        void OnStreamRemoved(const Tp::MediaStreamPtr &media_stream);
        void OnStreamDirectionChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamDirection direction, Tp::MediaStreamPendingSend ps);
        void OnStreamStateChanged(const Tp::MediaStreamPtr &stream, Tp::MediaStreamState state);
        void OnAudioStreamCreated(Tp::PendingOperation *op);
    signals:
		void Ready(VoiceSession* session);
    };

    typedef std::vector<VoiceSession*> VoiceSessionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VoiceSession_h
