#ifndef incl_Communication_TelepathyIM_VoiceSession_h
#define incl_Communication_TelepathyIM_VoiceSession_h

//#include <TelepathyQt4/Types>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/PendingReady>
#include <Foundation.h>
#include <ServiceInterface.h> // needed
#include <SoundServiceInterface.h>

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
     *        * Check if audio/video capture/playback is possible and trow exections...
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

        //! Getters for captured and received video widgets
        virtual Communication::VideoWidgetInterface* GetLocallyCapturedVideo();
        virtual Communication::VideoWidgetInterface* GetReceivedVideo();

        virtual Communication::VoiceSessionInterface::StreamState GetAudioStreamState() const;
        virtual Communication::VoiceSessionInterface::StreamState GetVideoStreamState() const;
        virtual bool IsSendingAudioData() const;
        virtual bool IsSendingVideoData() const;
        virtual bool IsReceivingAudioData() const;
        virtual bool IsReceivingVideoData() const;

    public slots:
        //! Close the session
		virtual void Close();

        //! Accept incoming session
        virtual void Accept();

        //! Reject incoming session
        virtual void Reject();

        //!
        virtual void SendAudioData(bool send);

        //!
        virtual void SendVideoData(bool send);

        //! Update playback position for received audio
        virtual void UpdateAudioSourcePosition(Core::Vector3df position = Core::Vector3df(0.0f, 0.0f, 0.0f) );

	protected:
        void UpdateStreamDirection(const Tp::MediaStreamPtr &stream, bool send);
        void CreateAudioStream();
        void CreateVideoStream();
        void CreateFarsightChannel();
        void DeleteChannels();

        QString GetReason() const { return reason_; };

        Tp::MediaStreamPtr GetAudioMediaStream() const;
        Tp::MediaStreamPtr GetVideoMediaStream() const; 
        
		State state_;
		Tp::StreamedMediaChannelPtr tp_channel_;
        FarsightChannel* farsight_channel_;

        Tp::PendingMediaStreams *pending_audio_streams_;
        Tp::PendingMediaStreams *pending_video_streams_;

        Tp::ContactPtr tp_contact_;
        QString reason_;      
        VoiceSessionParticipantVector participants_;

        Core::sound_id_t audio_playback_channel_;

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
        void OnAudioPlaybackBufferReady(Core::u8* buffer, int buffer_size);

    private:
        QByteArray *stream_buffer_;
        int ms_buffer_size_;

    signals:

        // When incoming session is ready for accecpt/reject
		void Ready(VoiceSession* session);
    };

    typedef std::vector<VoiceSession*> VoiceSessionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VoiceSession_h
