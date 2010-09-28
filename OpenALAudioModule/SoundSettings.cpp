// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SoundSettings.h"
#include "OpenALAudioModule.h"

#include "Framework.h"
#include "ServiceManager.h"
#include "ISoundService.h"
#include "UiServiceInterface.h"

#include <QUiLoader>
#include <QFile>
#include <QAbstractSlider>

#include "MemoryLeakCheck.h"

namespace OpenALAudio
{
    SoundSettings::SoundSettings(Foundation::Framework* framework) :
        framework_(framework),
        settings_widget_(0)
    {
        Foundation::UiServiceInterface *ui = framework_->GetService<Foundation::UiServiceInterface>();
        if (!ui)
            return;

        QUiLoader loader;
        QFile file("./data/ui/soundsettings.ui");

        if (!file.exists())
        {
            OpenALAudioModule::LogError("Cannot find sound settings .ui file.");
            return;
        }

        settings_widget_ = loader.load(&file); 
        if (!settings_widget_)
            return;

        ui->AddSettingsWidget(settings_widget_, "Sound");

        ISoundService *soundsystem = framework_->GetService<ISoundService>();
        if (!soundsystem)
            return;
        QAbstractSlider* slider = settings_widget_->findChild<QAbstractSlider*>("slider_master");
        if (slider)
        {
            slider->setValue(soundsystem->GetMasterGain() * 100);
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_triggered");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(ISoundService::Triggered) * 100);
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(TriggeredGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_ambient");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(ISoundService::Ambient) * 100);
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(AmbientGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_voice");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(ISoundService::Voice) * 100);
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(VoiceGainChanged(int)));
        }
    }

    SoundSettings::~SoundSettings()
    {
        SAFE_DELETE(settings_widget_);
    }

    void SoundSettings::MasterGainChanged(int value)
    {
        ISoundService *soundsystem = framework_->GetService<ISoundService>();
        if (soundsystem)
            soundsystem->SetMasterGain(value / 100.0);
    }

    void SoundSettings::TriggeredGainChanged(int value)
    {
        ISoundService *soundsystem = framework_->GetService<ISoundService>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(ISoundService::Triggered, value / 100.0);
    }

    void SoundSettings::AmbientGainChanged(int value)
    {
        ISoundService *soundsystem = framework_->GetService<ISoundService>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(ISoundService::Ambient, value / 100.0);
    }
    
    void SoundSettings::VoiceGainChanged(int value)
    {
        ISoundService *soundsystem = framework_->GetService<ISoundService>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(ISoundService::Voice, value / 100.0);
    }
}
