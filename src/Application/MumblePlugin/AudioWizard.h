// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleFwd.h"
#include "FrameworkFwd.h"
#include "MumbleDefines.h"

#include "ui_AudioWizard.h"
#include <QPixmap>

/// @cond PRIVATE
namespace Mumble
{
    class AudioBar;
}

namespace MumbleAudio
{
    class AudioWizard : public QWidget, private Ui::AudioWizardWidget
    {    
    Q_OBJECT

    public:
        AudioWizard(Framework *framework, AudioSettings settings);
        ~AudioWizard();

        void SetLevels(float level, bool isSpeech);

    signals:
        void SettingsChanged(MumbleAudio::AudioSettings settings, bool saveConfig);

    private slots:
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

        void OnAdvancedToggle();
        void OnProcessingHelpToggle();
        void OnTransmissionHelpToggle();
        void OnPositionalHelpToggle();

        void OnOKPressed();
        void OnCancelPressed();
        void OnApplyPressed();
        void OnSliderReleased();

    private:
        AudioSettings currentSettings;
        AudioSettings originalSettings;

        Mumble::AudioBar *audioBar;

        int peakTicks;
        int peakMax;

        QPixmap activePixmap;
        QPixmap inactivePixmap;
    };
}
/// @endcond
