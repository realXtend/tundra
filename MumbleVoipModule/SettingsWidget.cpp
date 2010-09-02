// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
#include <QSettings>
#include "Settings.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    SettingsWidget::SettingsWidget(Settings* settings) : QWidget(), settings_(settings)
    {
        settings_->Load();
        InitializeUI();
        UpdateUI();
    }

    SettingsWidget::~SettingsWidget()
    {
        ApplyChanges();
        settings_->Save();
    }

    void SettingsWidget::InitializeUI()
    {
        setupUi(this);

        QStringList items;
        items.append("Allways Off");
        items.append("Allways On");
//        items.append("Push-To-Talk"); // \todo Implement the ptt mode first
        this->defaultVoiceMode->addItems(items);

        LoadInitialState();

        connect(this->testMicrophoneButton, SIGNAL(clicked()), this, SLOT(OpenMicrophoneAdjustmentWidget()));
        connect(this->applyPlaybackBufferSizeButton, SIGNAL(clicked()), this, SLOT(ApplyPlaybackBufferSize()));
        connect(this->applyEncodeQualityButton, SIGNAL(clicked()), this, SLOT(ApplyEncodeQuality()));
        connect(this->playbackBufferSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateUI()));
        connect(this->encodeQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateUI()));
        connect(this->defaultVoiceMode, SIGNAL(currentIndexChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->microphoneLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(ApplyChanges()));
    }

    void SettingsWidget::LoadInitialState()
    {
        this->playbackBufferSlider->setValue(settings_->GetPlaybackBufferSizeMs());
        this->encodeQualitySlider->setValue(settings_->GetEncodeQuality()*100);
        this->defaultVoiceMode->setCurrentIndex(static_cast<int>(settings_->GetDefaultVoiceMode()));
        this->microphoneLevelSlider->setValue(settings_->GetMicrophoneLevel()*100);
        this->enabledCheckBox->setChecked(settings_->GetEnabled());
    }

    void SettingsWidget::OpenMicrophoneAdjustmentWidget()
    {
        // \todo Show the widget
    }

    void SettingsWidget::ApplyChanges()
    {
        settings_->SetPlaybackBufferSizeMs( this->playbackBufferSlider->value() );
        settings_->SetEncodeQuality( this->encodeQualitySlider->value()*0.01 );
        settings_->SetDefaultVoiceMode( Settings::VoiceMode(this->defaultVoiceMode->currentIndex()) );
        settings_->SetMicrophoneLevel( this->microphoneLevelSlider->value()*0.01 );
        settings_->SetEnabled( this->enabledCheckBox->isChecked() );
        settings_->Save();

        settings_->property("playback_buffer_size_ms") = this->playbackBufferSlider->value();

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
