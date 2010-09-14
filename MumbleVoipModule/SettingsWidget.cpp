// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
//#include <QSettings>
#include "Settings.h"
#include "Provider.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    SettingsWidget::SettingsWidget(Provider* provider, Settings* settings) : QWidget(), provider_(provider), settings_(settings)
    {
        InitializeUI();
        UpdateUI();
    }

    SettingsWidget::~SettingsWidget()
    {
        ApplyChanges();
    }

    void SettingsWidget::InitializeUI() 
    {
        setupUi(this);

        QStringList items;
        items.append("Mute");
        items.append("Continuous Transmission");
        items.append("Push-to-Talk");
        items.append("Toggle mode");
        this->defaultVoiceMode->addItems(items);

        LoadInitialState();

        connect(this->testMicrophoneButton, SIGNAL(clicked()), this, SLOT(OpenMicrophoneAdjustmentWidget()));
        connect(this->applyPlaybackBufferSizeButton, SIGNAL(clicked()), this, SLOT(ApplyPlaybackBufferSize()));
        connect(this->applyEncodeQualityButton, SIGNAL(clicked()), this, SLOT(ApplyEncodeQuality()));
        connect(this->playbackBufferSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateUI()));
        connect(this->encodeQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateUI()));
        connect(this->defaultVoiceMode, SIGNAL(currentIndexChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->microphoneLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->positionalAudioCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ApplyChanges()));
        connect(settings_, SIGNAL(MicrophoneLevelChanged(double)), this, SLOT(UpdateMicrophoneLevel()));
    }

    void SettingsWidget::LoadInitialState()
    {
        this->playbackBufferSlider->setValue(settings_->GetPlaybackBufferSizeMs());
        this->encodeQualitySlider->setValue(settings_->GetEncodeQuality()*100);
        this->defaultVoiceMode->setCurrentIndex(static_cast<int>(settings_->GetDefaultVoiceMode()));
        this->microphoneLevelSlider->setValue(settings_->GetMicrophoneLevel()*100);
        if (settings_->GetPositionalAudioEnabled())
            this->positionalAudioCheckBox->setCheckState(Qt::Checked);
        else
            this->positionalAudioCheckBox->setCheckState(Qt::Unchecked);
    }

    void SettingsWidget::UpdateMicrophoneLevel()
    {
        this->microphoneLevelSlider->setValue(settings_->GetMicrophoneLevel()*100);
        UpdateUI();
    }

    void SettingsWidget::OpenMicrophoneAdjustmentWidget()
    {
        if (provider_)
            provider_->ShowMicrophoneAdjustmentDialog();
    }

    void SettingsWidget::ApplyChanges()
    {
        settings_->SetPlaybackBufferSizeMs( this->playbackBufferSlider->value() );
        settings_->SetEncodeQuality( this->encodeQualitySlider->value()*0.01 );
        settings_->SetDefaultVoiceMode( Settings::VoiceMode(this->defaultVoiceMode->currentIndex()) );
        settings_->SetMicrophoneLevel( this->microphoneLevelSlider->value()*0.01 );
        if (this->positionalAudioCheckBox->checkState() == Qt::Checked)
            settings_->SetPositionalAudioEnabled(true);
        else
            settings_->SetPositionalAudioEnabled(false);
        settings_->Save();

        UpdateUI();
    }

    void SettingsWidget::UpdateUI()
    {
        playbackBufferSizeLabel->setText(QString("%1 ms").arg(playbackBufferSlider->value(), 4));
        encodeQualityLabel->setText(QString("%1 %").arg(encodeQualitySlider->value(), 3));
        microphoneLevelLabel->setText(QString("%1 %").arg(microphoneLevelSlider->value(), 3));
    }
    
    void SettingsWidget::ApplyEncodeQuality()
    {
        settings_->setProperty("encode_quality", QVariant(this->encodeQualitySlider->value()*0.01));
        settings_->Save();
    }

    void SettingsWidget::ApplyPlaybackBufferSize()
    {
        settings_->setProperty("playback_buffer_size_ms", this->playbackBufferSlider->value());
        settings_->Save();
    }

    void SettingsWidget::ApplyMicrophoneLevel()
    {
        settings_->SetMicrophoneLevel( this->microphoneLevelSlider->value()*0.01 );
        settings_->Save();
    }

} // MumbleVoip
