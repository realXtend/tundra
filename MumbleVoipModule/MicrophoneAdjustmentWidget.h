// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_MicrophoneAdjustmentWidget_h
#define incl_MumbleVoipModule_MicrophoneAdjustmentWidget_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_MicrophoneAdjustment.h"
#include "SoundServiceInterface.h"
#include <QTimer>

namespace Foundation
{
    class Framework;
}

namespace MumbleVoip
{
    class Settings;
    class PCMAudioFrame;

    /// Read audio data from SoundService adjust volume and playback locally.
    /// Shows graphical view for audio level.
    class MicrophoneAdjustmentWidget : public QWidget, private Ui::MicrophoneAdjustmentWidget
    {
        Q_OBJECT
    public:
        MicrophoneAdjustmentWidget(Foundation::Framework* framework, Settings* settings);
        virtual ~MicrophoneAdjustmentWidget();

    private slots:
        virtual void HandleRecordedAudio();
        virtual void PlaybackAudioFrame(PCMAudioFrame* frame);
        virtual void ApplyMicrophoneLevel(PCMAudioFrame* frame);
        virtual void UpdateUI();
        virtual void SaveSettings();
        virtual void ApplySliderValue();
        virtual void StartAudioRecording();
        virtual void close();
    private:
        static const int AUDIO_RECORDING_BUFFER_MS_ = 500;

        Settings* settings_;
        QTimer read_audio_timer_;
        Foundation::Framework* framework_;
        sound_id_t sound_id_;
        double voice_activity_level_;
        double microphone_level_;
    signals:
        void Closed(bool save_settings);
    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_MicrophoneAdjustmentWidget_h
