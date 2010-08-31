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
        LoadInitialState();
    }

    SettingsWidget::~SettingsWidget()
    {
        settings_->Save();
    }

    void SettingsWidget::InitializeUI()
    {
        setupUi(this);

        connect(this->testMicrophoneButton, SIGNAL(clicked()), this, SLOT(OpenMicrophoneAdjustmentWidget()));

        this->playbackBufferSlider->setRange(0,1000);
        this->playbackBufferSlider->setTickInterval(10);
        this->encodeQualitySlider->setRange(0,100);
        this->encodeQualitySlider->setTickInterval(1);
        QStringList items;
        items.append("Allways off");
        items.append("Allways On");
        items.append("Push-to-talk");
        this->defaultVoiceMode->addItems(items);

        UpdateUI();

        connect(this->playbackBufferSlider, SIGNAL(valueChanged(int)), this, SLOT(SaveSettings()));
        connect(this->encodeQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(SaveSettings()));
        connect(this->defaultVoiceMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SaveSettings()));
    }

    void SettingsWidget::LoadInitialState()
    {
        this->playbackBufferSlider->setValue(settings_->playback_buffer_size_ms);
        this->encodeQualitySlider->setValue(settings_->encode_quality);
        this->defaultVoiceMode->setCurrentIndex(static_cast<int>(settings_->default_voice_mode));
    }

    void SettingsWidget::OpenMicrophoneAdjustmentWidget()
    {
        // \todo Show the widget
    }

    void SettingsWidget::SaveSettings()
    {
        settings_->playback_buffer_size_ms = this->playbackBufferSlider->value();
        settings_->encode_quality = this->encodeQualitySlider->value();
        settings_->default_voice_mode = Settings::VoiceMode(this->defaultVoiceMode->currentIndex());
        settings_->Save();

        UpdateUI();
    }

    void SettingsWidget::UpdateUI()
    {
        playbackBufferSizeLabel->setText(QString("%1 ms").arg(settings_->playback_buffer_size_ms));
        encodeQualityLabel->setText(QString("%1 %").arg(settings_->encode_quality));
    }

} // MumbleVoip
