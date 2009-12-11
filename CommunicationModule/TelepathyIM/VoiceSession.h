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
     *
     *  TODO: audio/video in/out enabled variable setting is NOT COMPLETE YET!
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

        //! Accept incoming session
        virtual void Accept();

        //! Reject incoming session
        virtual void Reject();

        virtual Communication::VideoWidgetInterface* GetRemoteVideo();
        virtual Communication::VideoWidgetInterface* GetOwnVideo();

        virtual void SetAudioOutEnabled(bool value);
        virtual void SetVideoOutEnabled(bool value);
        virtual bool GetAudioInEnabled() { return audio_in_enabled_; };
        virtual bool GetVideoInEnabled() { return video_in_enabled_; };
        virtual bool GetAudioOutEnabled() { return audio_out_enabled_; };
        virtual bool GetVideoOutEnabled() { return video_out_enabled_; };

	protected:
        void UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send);
        void CreateAudioStream();
        void CreateVideoStream();
        QString GetReadon() { return reason_; };

        Tp::MediaStreamPtr GetAudioMediaStream(); // todo: make objects for audio and video streams
        Tp::MediaStreamPtr GetVideoMediaStream(); 
        
		State state_;
		Tp::StreamedMediaChannelPtr tp_channel_;
        FarsightChannel* farsight_channel_;

        Tp::PendingMediaStreams *pending_audio_streams_;
        Tp::PendingMediaStreams *pending_video_streams_;

        Tp::ContactPtr tp_contact_;
        QString reason_;      
        VoiceSessionParticipantVector participants_;

        bool audio_in_enabled_;
        bool video_in_enabled_;
        bool audio_out_enabled_;
        bool video_out_enabled_;

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
        void OnVideoStreamCreated(Tp::PendingOperation *op);

    signals:

        // When incoming session is ready for accecpt/reject
		void Ready(VoiceSession* session);

        void AudioInEnabledStateChanged(bool state);

        void VideoInEnabledStateChanged(bool state);

        void AudioOutEnabledStateChanged(bool state);

        void VideoOutEnabledStateChanged(bool state);
    };

    typedef std::vector<VoiceSession*> VoiceSessionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VoiceSession_h
