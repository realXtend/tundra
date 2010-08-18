// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSettings.h"
#include "OpenALAudioModule.h"
#include "SoundServiceInterface.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "Inworld/InworldSceneController.h"
#include "Framework.h"

#include <QUiLoader>
#include <QFile>
#include <QAbstractSlider>

#ifndef UISERVICE_TEST
#include "UiModule.h"
#endif

namespace OpenALAudio
{
    SoundSettings::SoundSettings(Foundation::Framework* framework) :
        framework_(framework),
        settings_widget_(0)
    {
        InitWindow();
    }
    
    SoundSettings::~SoundSettings()
    {
        SAFE_DELETE(settings_widget_);
    }

    void SoundSettings::InitWindow()
    {
#ifndef UISERVICE_TEST
        UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
        if (!ui_module)
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

        ui_module->GetInworldSceneController()->AddSettingsWidget(settings_widget_, "Sound");

        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (!soundsystem)
            return;
        QAbstractSlider* slider = settings_widget_->findChild<QAbstractSlider*>("slider_master");
        if (slider)
        {
            slider->setValue(soundsystem->GetMasterGain() * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_triggered");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Triggered) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(TriggeredGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_ambient");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Ambient) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(AmbientGainChanged(int)));
        }
        slider = settings_widget_->findChild<QAbstractSlider*>("slider_voice");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Voice) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(VoiceGainChanged(int)));
        }
#endif
    }

    void SoundSettings::MasterGainChanged(int value)
    {
        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (soundsystem)
            soundsystem->SetMasterGain(value / 100.0);
    }

    void SoundSettings::TriggeredGainChanged(int value)
    {
        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Triggered, value / 100.0);
    }

    void SoundSettings::AmbientGainChanged(int value)
    {
        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Ambient, value / 100.0);
    }
    
    void SoundSettings::VoiceGainChanged(int value)
    {
        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (soundsystem)
            soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Voice, value / 100.0);
    }
}
