// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_VoiceSessionInterface_h
#define incl_Comm_VoiceSessionInterface_h

#include <QObject>
#include "CommunicationModuleFwd.h"
#include "Vector3D.h"

namespace Communication
{
    /**
     *  Voice/Video session between two users. Audio is played using OpenALAudioModule and video is 
     *  played through VideoPlaybackWidget objects.
     *
     *  To get VoiceSession object you must call ConnectionInterface::OpenVoiceSession method or get
     *  ConnectionInterface::VoiceSessionReceived signal.
     *
     *  Currently only 1-1 sessions are supported and only 1 session can be exist at the time.
     *
     * @todo Getter for originator of the chat session (gui needs id of friend to create tab and store id to local containers with that id).
     *       GUI now needs to go through ChatSessionParticipantVector and do unneccesary looping to get one QString
     */
    class VoiceSessionInterface: public QObject
    {
        Q_OBJECT
    public:
        //! State of voice session instance
        enum State { STATE_INITIALIZING, STATE_RINGING_LOCAL, STATE_RINGING_REMOTE, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

        //! State of audio or video stream in voice session instance
        enum StreamState { SS_DISCONNECTED, SS_CONNECTING, SS_CONNECTED };

        //! @return State of the session
        virtual State GetState() const = 0;

        //! @return reason of current state eg. error message
        virtual QString GetReason() const = 0;

        //! @return all known participants of the chat session inlcuding the user
        virtual VoiceSessionParticipantVector GetParticipants() const = 0;

        //! /return widget for playback incoming video stream. Return 0 if widget doesn't exist.
        virtual Communication::VideoPlaybackWidgetInterface* GetReceivedVideo() = 0;

        //! /return widget for playback local captured video stream. Return 0 if widget doesn't exist.
        virtual Communication::VideoPlaybackWidgetInterface* GetLocallyCapturedVideo() = 0;

    public slots:
                                   
        //! Closes the session and all streams associated to it.
        virtual void Close() = 0;

        //! Accpet incoming session request.
        //! If session state is not STATE_RINGING_LOCAL then does nothing
        virtual void Accept() = 0;

        //! Rejects incoming session reqeust.
        //! If session state is not STATE_RINGING_LOCAL then does nothing
        virtual void Reject() = 0;

        //! /return state of audio stream
        virtual StreamState GetAudioStreamState() const = 0;

        //! /return state of video stream
        virtual StreamState GetVideoStreamState() const = 0;

        //! /return true if session is sending audio data to audio steam return false if not.
        virtual bool IsSendingAudioData() const = 0;

        //! /return true if session is sending video data to video steam return false if not.
        virtual bool IsSendingVideoData() const = 0;

        //! /return true if session is receiving audio data from audio steam return false if not.
        virtual bool IsReceivingAudioData() const = 0;

        //! /return true if session is receiving video data from video steam return false if not.
        virtual bool IsReceivingVideoData() const = 0;

        //! Enable audio data sending. If audio stream is not created then this create it.
        virtual void SendAudioData(bool send) = 0;

        //! Enable video data sending. If video stream is not created then this create it.
        virtual void SendVideoData(bool send) = 0;

        //! Update playback position for received audio
        virtual void UpdateAudioSourcePosition(Vector3df position = Vector3df(0.0f, 0.0f, 0.0f) ) = 0;

        //! Set if we want to use spatial voice with avatar tracking
        //! /todo is this needed here?
        virtual void TrackingAvatar(bool enabled) = 0;

    signals:
        //!
        void ParticipantJoined(const VoiceSessionParticipantInterface& participant);

        //!
        void ParticipantLeft(const VoiceSessionParticipantInterface& participant);

        //!
        //void Opened(VoiceSessionInterface*);   // do we need this ?

  //      //!
        //void Closed(VoiceSessionInterface*);   // do we need this ?

        //!
        void StateChanged(Communication::VoiceSessionInterface::State state);

        //!
        void AudioStreamStateChanged(Communication::VoiceSessionInterface::StreamState state);

        //!
        void VideoStreamStateChanged(Communication::VoiceSessionInterface::StreamState state);

        //! When audio data sending toggles
        void SendingAudioData(bool sending);

        //! When video data sending toggles
        void SendingVideoData(bool sending);

        //! When audio data receiving toggles
        void ReceivingAudioData(bool sending);

        //! When video data receiving toggles
        void ReceivingVideoData(bool sending);

        // todo: Error signal?
    };

}

#endif

