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
        items.append("Push-To-Talk");
        this->defaultVoiceMode->addItems(items);

        LoadInitialState();

        connect(this->testMicrophoneButton, SIGNAL(clicked()), this, SLOT(OpenMicrophoneAdjustmentWidget()));
        connect(this->applyPlaybackBufferSizeButton, SIGNAL(clicked()), this, SLOT(ApplyPlaybackBufferSize()));
        connect(this->applyEncodeQualityButton, SIGNAL(clicked()), this, SLOT(ApplyEncodeQuality()));
        connect(this->playbackBufferSlider, SIGNAL(valueChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->encodeQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->defaultVoiceMode, SIGNAL(currentIndexChanged(int)), this, SLOT(ApplyChanges()));
        connect(this->microphoneLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(ApplyChanges()));
    }

    void SettingsWidget::LoadInitialState()
    {
        this->playbackBufferSlider->setValue(settings_->playback_buffer_size_ms);
        this->encodeQualitySlider->setValue(settings_->encode_quality*100);
        this->defaultVoiceMode->setCurrentIndex(static_cast<int>(settings_->default_voice_mode));
        this->microphoneLevelSlider->setValue(settings_->microphone_level*100);
        this->enabledCheckBox->setChecked(settings_->enabled);
    }

    void SettingsWidget::OpenMicrophoneAdjustmentWidget()
    {
        // \todo Show the widget
    }

    void SettingsWidget::ApplyChanges()
    {
        settings_->playback_buffer_size_ms = this->playbackBufferSlider->value();
        settings_->encode_quality = this->encodeQualitySlider->value()*0.01;
        settings_->default_voice_mode = Settings::VoiceMode(this->defaultVoiceMode->currentIndex());
        settings_->microphone_level = this->microphoneLevelSlider->value()*0.01;
        settings_->enabled = this->enabledCheckBox->isChecked();
        settings_->Save();

        UpdateUI();
    }

    void SettingsWidget::UpdateUI()
    {
        playbackBufferSizeLabel->setText(QString("%1 ms").arg(playbackBufferSlider->value()));
        encodeQualityLabel->setText(QString("%1 %").arg(encodeQualitySlider->value()));
        microphoneLevelLabel->setText(QString("%1 %").arg(microphoneLevelSlider->value()));
        if (!settings_->enabled)
        {
            for(QObjectList::const_iterator i = this->groupBox->children().begin(); i != this->groupBox->children().end(); ++i)
            {
                (*i)->setProperty( "enabled", false );
            }

        //    groupBox->enabled = false;
        }
    }
    
    void SettingsWidget::ApplyEncodeQuality()
    {
        ApplyChanges();
    }

    void SettingsWidget::ApplyPlaybackBufferSize()
    {
        ApplyChanges();
    }

} // MumbleVoip
