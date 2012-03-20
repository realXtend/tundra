// For conditions of distribution and use, see copyright notice in LICENSE

#include "AudioWizard.h"
#include "Framework.h"
#include "AudioAPI.h"
#include "mumble/AudioStats.h"
#include "LoggingFunctions.h"

namespace MumbleAudio
{
    AudioWizard::AudioWizard(Framework *framework, AudioSettings settings) : 
        peakTicks(0),
        peakMax(0)
    {
        originalSettings = settings;
        currentSettings = settings;

        activePixmap = QPixmap(":img/voice-active.png");
        inactivePixmap = QPixmap(":img/voice-inactive.png");

        setupUi(this);
        setAttribute(Qt::WA_DeleteOnClose, true);
        
        // Audio bar init
        audioBar = new Mumble::AudioBar(this);
        audioBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        audioBar->setMinimumHeight(28);
        audioBar->qcBelow = Qt::red;
        audioBar->qcAbove = Qt::green;
        audioBar->qcInside = Qt::yellow;
        audioBarLayout->insertWidget(1, audioBar);

        // Input devices
        QStringList inputDevices = framework->Audio()->GetRecordingDevices();
        comboBoxInputDevice->clear();
        comboBoxInputDevice->addItems(inputDevices);
        if (!settings.recordingDevice.isEmpty())
        {
            int index = comboBoxInputDevice->findText(settings.recordingDevice);
            if (index != -1)
                comboBoxInputDevice->setCurrentIndex(index);
            else
                LogWarning("[MumblePlugin]: Config input device '" + settings.recordingDevice + "' could not be found from available input devices.");
        }
        else
            comboBoxInputDevice->setCurrentIndex(0);

        // Quality
        if (currentSettings.quality == QualityLow)
            radioButtonLow->setChecked(true);
        else if (currentSettings.quality == QualityBalanced)
            radioButtonBalanced->setChecked(true);
        else if (currentSettings.quality == QualityHigh)
            radioButtonHigh->setChecked(true);

        // Transmission mode
        comboBoxTransmitMode->setCurrentIndex(currentSettings.transmitMode == TransmitVoiceActivity ? 0 : 1);
        OnTransmitModeChanged(comboBoxTransmitMode->currentText());

        // Audio processing
        int suppressValue = -currentSettings.suppression;
        sliderSuppression->setValue(suppressValue);
        OnSuppressChanged(suppressValue);

        int amplificationValue = 20000 - currentSettings.amplification;
        sliderAmplification->setValue(amplificationValue);
        OnAmplificationChanged(amplificationValue);

        // Voice activity detection
        int silenceValue = static_cast<int>(currentSettings.VADmin * 32767.0f + 0.5f);
        sliderSilence->setValue(silenceValue);
        OnMinVADChanged(silenceValue);

        int speechValue = static_cast<int>(currentSettings.VADmax * 32767.0f + 0.5f);
        sliderSpeech->setValue(speechValue);
        OnMaxVADChanged(speechValue);

        // Positional audio
        innerRangeSpinBox->setValue(settings.innerRange);
        OnInnerRangeChanged(settings.innerRange);
        outerRangeSpinBox->setMinimum(settings.innerRange + 1);
        outerRangeSpinBox->setValue(settings.outerRange);
        OnOuterRangeChanged(settings.outerRange);

        checkBoxPositionalSend->setChecked(settings.allowSendingPositional);
        OnAllowSendingPositionalChanged();
        checkBoxPositionalReceive->setChecked(settings.allowReceivingPositional);
        OnAllowReceivingPositionalChanged();

        // Setting tooltips
        QString tooltipTr;
        QString tooltipProc;
        QString tooltipPos;

        tooltipTr  = "<strong>Continuous trasmission</strong> mode sends voice out all the time. Doing this has the drawback that it spends more bandwith for other users,";
        tooltipTr += "hence it is not recommended. If you can't get voice activity detection to work well enough you can fall back to this mode.<br /><br />";
        tooltipTr += "<strong>Voice activity</strong> detection can detect when you are speaking and only then send out audio. This saves processing and bandwidth on you ";
        tooltipTr += "and other clients. Activate the mode and adjust the two sliders so that when you speak normally it detects the speech. When the meter goes to green it";
        tooltipTr += "starts sending audio and will send untill the meter goes to red for a certain period.";
        groupBoxTransmission->setToolTip(tooltipTr);

        tooltipProc  = "<strong>Noise suppression</strong> is used to remove noice from the recorded microphone audio. <br /><br />";
        tooltipProc += "<strong>Amplification</strong> can automatically increase volume when speking quietly and automatically decrese volume when loud spikes are detected.<br /><br />";
        tooltipProc += "Remember to always set the mic volume reasonably high from your operating system and then adjust additional amplification if it is needed.";
        groupBoxProcessing->setToolTip(tooltipProc);

        tooltipPos  = "Checking <strong>Send Positional Audio</strong> will send active sound listener position with the audio data. The receiving client can disable playing back the audio with the position from other users.<br /><br />";
        tooltipPos += "Checking <strong>Receive Positional Audio</strong> will play incoming voice with position if it is provided by the other client.<br /><br />";
        tooltipPos += "You can set the playback ranges on when the volume starts to fade out and where it goes completely silent.";
        groupBoxPositional->setToolTip(tooltipPos);

        // Signaling
        connect(radioButtonLow, SIGNAL(clicked()), SLOT(OnQualityChanged()));
        connect(radioButtonBalanced, SIGNAL(clicked()), SLOT(OnQualityChanged()));
        connect(radioButtonHigh, SIGNAL(clicked()), SLOT(OnQualityChanged()));

        connect(sliderSuppression, SIGNAL(valueChanged(int)), SLOT(OnSuppressChanged(int)));
        connect(sliderAmplification, SIGNAL(valueChanged(int)), SLOT(OnAmplificationChanged(int)));
        connect(sliderSilence, SIGNAL(valueChanged(int)), SLOT(OnMinVADChanged(int)));
        connect(sliderSpeech, SIGNAL(valueChanged(int)), SLOT(OnMaxVADChanged(int)));

        connect(sliderSuppression, SIGNAL(sliderReleased()), SLOT(OnSliderReleased()));
        connect(sliderAmplification, SIGNAL(sliderReleased()), SLOT(OnSliderReleased()));        
        connect(sliderSilence, SIGNAL(sliderReleased()), SLOT(OnSliderReleased()));
        connect(sliderSpeech, SIGNAL(sliderReleased()), SLOT(OnSliderReleased()));

        connect(buttonOK, SIGNAL(clicked()), SLOT(OnOKPressed()));
        connect(buttonCancel, SIGNAL(clicked()), SLOT(OnCancelPressed()));
        connect(buttonApply, SIGNAL(clicked()), SLOT(OnApplyPressed()));

        connect(comboBoxInputDevice, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnInputDeviceChanged(const QString&)));
        connect(comboBoxTransmitMode, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnTransmitModeChanged(const QString&)));

        connect(buttonAdvanced, SIGNAL(clicked()), SLOT(OnAdvancedToggle()));

        connect(innerRangeSpinBox, SIGNAL(valueChanged(int)), SLOT(OnInnerRangeChanged(int)));
        connect(outerRangeSpinBox, SIGNAL(valueChanged(int)), SLOT(OnOuterRangeChanged(int)));

        connect(checkBoxPositionalSend, SIGNAL(clicked()), SLOT(OnAllowSendingPositionalChanged()));
        connect(checkBoxPositionalReceive, SIGNAL(clicked()), SLOT(OnAllowReceivingPositionalChanged()));

        buttonApply->setDisabled(true);
        groupBoxQuality->setVisible(false);
        groupBoxProcessing->setVisible(false);

        setMinimumSize(492, 575);
        setMaximumSize(492, 824);
        show();

        resize(1,1);
    }   

    AudioWizard::~AudioWizard()
    {
    }

    void AudioWizard::SetLevels(float level, bool isSpeech)
    {
        if (currentSettings.transmitMode == TransmitVoiceActivity)
        {
            int iLevel = static_cast<int>((32767.f/96.0f) * (96.0f + level) + 0.5f);
            if (peakTicks++ >= 20)
            {
                peakTicks = 0;
                peakMax = 0;
            }
            if (iLevel > peakMax)
                peakMax = iLevel;

            audioBar->iPeak = peakMax;
            audioBar->iValue = iLevel;
            audioBar->update();
        }

        const QPixmap *currentPixmap = labelIconVAD->pixmap();
        if (isSpeech && currentPixmap != &activePixmap)
            labelIconVAD->setPixmap(activePixmap);
        else if (!isSpeech && currentPixmap != &inactivePixmap)
            labelIconVAD->setPixmap(inactivePixmap);
    }

    void AudioWizard::OnQualityChanged()
    {
        if (radioButtonLow->isChecked())
            currentSettings.quality = QualityLow;
        else if (radioButtonBalanced->isChecked())
            currentSettings.quality = QualityBalanced;
        else if (radioButtonHigh->isChecked())
            currentSettings.quality = QualityHigh;

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnInputDeviceChanged(const QString &deviceName)
    {
        currentSettings.recordingDevice = deviceName;
        
        // Automatically apply this option when changed.
        OnApplyPressed();
    }
    
    void AudioWizard::OnTransmitModeChanged(const QString &mode)
    {
        if (mode == "Voice Activity")
            currentSettings.transmitMode = TransmitVoiceActivity;
        else
            currentSettings.transmitMode = TransmitContinuous;

        bool vadEnabled = (currentSettings.transmitMode == TransmitVoiceActivity);
        sliderSilence->setEnabled(vadEnabled);
        labelValueSilence->setEnabled(vadEnabled);
        sliderSpeech->setEnabled(vadEnabled);
        labelValueSpeech->setEnabled(vadEnabled);
        audioBar->setEnabled(vadEnabled);
        
        peakTicks = 0;
        peakMax = 0;
        audioBar->iPeak = -1;
        audioBar->iValue = 0;
        audioBar->update();

        // Automatically apply this option when changed.
        OnApplyPressed();
    }

    void AudioWizard::OnSuppressChanged(int value)
    {
        if (value < 15)
        {
            labelValueSuppression->setText("Off");
            labelValueSuppression->setStyleSheet("color: red;");
            currentSettings.suppression = 0;
        }
        else
        {
            labelValueSuppression->setText(QString("-%1 dB").arg(value));
            labelValueSuppression->setStyleSheet("color: black;");
            currentSettings.suppression = -value;
        }
        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnAmplificationChanged(int value)
    {
        value = 18000 - value + 2000;
        float f = 20000.0f / static_cast<float>(value);
        labelValueAmplification->setText(QString::fromLatin1("%1").arg(f, 0, 'f', 2));

        currentSettings.amplification = value;
        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnMinVADChanged(int value)
    {
        currentSettings.VADmin = static_cast<float>(value) / 32767.0f;
        labelValueSilence->setText(QString::fromLatin1("%1").arg(currentSettings.VADmin, 0, 'f', 2));
        audioBar->iBelow = static_cast<int>(currentSettings.VADmin * 32767.0f + 0.5f);
        audioBar->update();

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnMaxVADChanged(int value)
    {
        currentSettings.VADmax = static_cast<float>(value) / 32767.0f;
        labelValueSpeech->setText(QString::fromLatin1("%1").arg(currentSettings.VADmax, 0, 'f', 2));
        audioBar->iAbove = static_cast<int>(currentSettings.VADmax * 32767.0f + 0.5f);
        audioBar->update();

        buttonApply->setEnabled(true);
    }
       
    void AudioWizard::OnInnerRangeChanged(int value)
    {
        currentSettings.innerRange = value;
        if (outerRangeSpinBox->value() < value + 1)
            outerRangeSpinBox->setValue(value + 1);
        outerRangeSpinBox->setMinimum(value + 1);

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnOuterRangeChanged(int value)
    {
        currentSettings.outerRange = value;

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnAllowSendingPositionalChanged()
    {
        currentSettings.allowSendingPositional = checkBoxPositionalSend->isChecked();

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnAllowReceivingPositionalChanged()
    {
        currentSettings.allowReceivingPositional = checkBoxPositionalReceive->isChecked();
        innerRangeSpinBox->setEnabled(currentSettings.allowReceivingPositional);
        outerRangeSpinBox->setEnabled(currentSettings.allowReceivingPositional);

        buttonApply->setEnabled(true);
    }

    void AudioWizard::OnAdvancedToggle()
    {
        bool visible = !groupBoxQuality->isVisible();
        groupBoxQuality->setVisible(visible);
        groupBoxProcessing->setVisible(visible);
        
        QString text = buttonAdvanced->text();
        buttonAdvanced->setText(!visible ? text.replace("Hide", "Show") : text.replace("Show", "Hide"));
        resize(visible ? QSize(999,999) : QSize(1,1));
    }

    void AudioWizard::OnOKPressed()
    {
        emit SettingsChanged(currentSettings, true);
        close();
    }

    void AudioWizard::OnCancelPressed()
    {
        emit SettingsChanged(originalSettings, false);
        close();
    }

    void AudioWizard::OnApplyPressed()
    {
        emit SettingsChanged(currentSettings, true);
        buttonApply->setDisabled(true);
    }
    
    void AudioWizard::OnSliderReleased()
    {
        // Automatically apply when a slider is released. We don't want to spam 
        // settings changed signal when the slider is still being moved.
        OnApplyPressed();
    }
}