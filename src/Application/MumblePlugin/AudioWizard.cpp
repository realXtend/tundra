// For conditions of distribution and use, see copyright notice in LICENSE

#include "AudioWizard.h"
#include "mumble/AudioStats.h"

namespace MumbleAudio
{
    AudioWizard::AudioWizard(AudioSettings settings) : 
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

        // Hide help labels by default
        helpProcessing->hide();
        helpTransmission->hide();
        helpPositional->hide();

        // Signaling
        connect(radioButtonLow, SIGNAL(clicked()), SLOT(OnQualityChanged()));
        connect(radioButtonBalanced, SIGNAL(clicked()), SLOT(OnQualityChanged()));
        connect(radioButtonHigh, SIGNAL(clicked()), SLOT(OnQualityChanged()));

        connect(sliderSuppression, SIGNAL(valueChanged(int)), SLOT(OnSuppressChanged(int)));
        connect(sliderAmplification, SIGNAL(valueChanged(int)), SLOT(OnAmplificationChanged(int)));

        connect(sliderSilence, SIGNAL(valueChanged(int)), SLOT(OnMinVADChanged(int)));
        connect(sliderSpeech, SIGNAL(valueChanged(int)), SLOT(OnMaxVADChanged(int)));

        connect(buttonOK, SIGNAL(clicked()), SLOT(OnOKPressed()));
        connect(buttonCancel, SIGNAL(clicked()), SLOT(OnCancelPressed()));
        connect(buttonApply, SIGNAL(clicked()), SLOT(OnApplyPressed()));

        connect(comboBoxTransmitMode, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnTransmitModeChanged(const QString&)));

        connect(pushButtonProcessingHelp, SIGNAL(clicked()), SLOT(OnProcessingHelpToggle()));
        connect(pushButtonTransmissionHelp, SIGNAL(clicked()), SLOT(OnTransmissionHelpToggle()));
        connect(pushButtonPositionalHelp, SIGNAL(clicked()), SLOT(OnPositionalHelpToggle()));

        connect(innerRangeSpinBox, SIGNAL(valueChanged(int)), SLOT(OnInnerRangeChanged(int)));
        connect(outerRangeSpinBox, SIGNAL(valueChanged(int)), SLOT(OnOuterRangeChanged(int)));

        connect(checkBoxPositionalSend, SIGNAL(clicked()), SLOT(OnAllowSendingPositionalChanged()));
        connect(checkBoxPositionalReceive, SIGNAL(clicked()), SLOT(OnAllowReceivingPositionalChanged()));

        buttonApply->setDisabled(true);

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

        buttonApply->setEnabled(true);
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

    void AudioWizard::OnProcessingHelpToggle()
    {
        helpProcessing->setVisible(!helpProcessing->isVisible());
        
        QString text = pushButtonProcessingHelp->text();
        pushButtonProcessingHelp->setText(!helpProcessing->isVisible() ? text.replace("Hide", "Show") : text.replace("Show", "Hide"));
        
        resize(1,1);
    }

    void AudioWizard::OnTransmissionHelpToggle()
    {
        helpTransmission->setVisible(!helpTransmission->isVisible());

        QString text = pushButtonTransmissionHelp->text();
        pushButtonTransmissionHelp->setText(!helpTransmission->isVisible() ? text.replace("Hide", "Show") : text.replace("Show", "Hide"));

        resize(1,1);
    }

    void AudioWizard::OnPositionalHelpToggle()
    {
        helpPositional->setVisible(!helpPositional->isVisible());

        QString text = pushButtonPositionalHelp->text();
        pushButtonPositionalHelp->setText(!helpPositional->isVisible() ? text.replace("Hide", "Show") : text.replace("Show", "Hide"));

        resize(1,1);
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
}