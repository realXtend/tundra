// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleFwd.h"
#include "FrameworkFwd.h"
#include "MumbleDefines.h"

#include "ui_AudioWizard.h"
#include "ui_AudioLevelDetectorWidget.h"

#include <QPixmap>
#include <QTimer>
#include <QDialog>

/// @cond PRIVATE
namespace Mumble
{
    class AudioBar;
}

namespace MumbleAudio
{
    class AudioLevelWidget : public QDialog, public Ui::AudioLevelDetectorWidget
    {
    
    Q_OBJECT
    
    public:
        AudioLevelWidget(QWidget *parent = 0);
        ~AudioLevelWidget();
        
        void AddLevel(float level);
        
        enum DetectionState
        {
            None = 0,
            Starting,
            Silence,
            Voice,
            Completed
        };
        
        DetectionState state;
        float voiceLevelCombined;
        float silenceLevelCombined;

    protected:
        void closeEvent(QCloseEvent *e);

    signals:
        void AudioLevelsCalculated(float silenceLevel, float voiceLevel);
        
    private slots:
        void OnSecondPassed();
        
    private:
        bool firstSample_;
        uint stepSecondsLeft_;
        QTimer tickTimer_;
    };
    
    class AudioWizard : public QWidget, private Ui::AudioWizardWidget
    {
    Q_OBJECT

    public:
        AudioWizard(Framework *framework, AudioSettings settings);
        ~AudioWizard();

    public slots:
        void StartAudioLevelDetection();
        void SetLevels(float level, bool isSpeech);

    signals:
        void SettingsChanged(MumbleAudio::AudioSettings settings, bool saveConfig);

    private slots:
        void OnAudioLevelsDetected(float silenceLevel, float voiceLevel);
        
        void OnQualityChanged();

        void OnInputDeviceChanged(const QString &deviceName);
        void OnTransmitModeChanged(const QString &mode);

        void OnSuppressChanged(int value);
        void OnAmplificationChanged(int value);

        void OnMinVADChanged(int value);
        void OnMaxVADChanged(int value);

        void OnInnerRangeChanged(int value);
        void OnOuterRangeChanged(int value);

        void OnAllowSendingPositionalChanged();
        void OnAllowReceivingPositionalChanged();
        void OnEchoCancellationChanged();

        void OnAdvancedToggle();

        void OnOKPressed();
        void OnCancelPressed();
        void OnApplyPressed();
        void OnSliderReleased();

    private:
        Framework *framework_;
        
        AudioSettings currentSettings;
        AudioSettings originalSettings;

        Mumble::AudioBar *audioBar;       
        AudioLevelWidget *detectorWidget_;

        int peakTicks;
        int peakMax;

        QPixmap activePixmap;
        QPixmap inactivePixmap;
    };
}
/// @endcond
