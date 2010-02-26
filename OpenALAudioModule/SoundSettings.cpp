// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSettings.h"
#include "OpenALAudioModule.h"
#include "SoundServiceInterface.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiSceneManager.h"
#include "Framework.h"

#include <QUiLoader>
#include <QFile>
#include <QAbstractSlider>

#include "UiModule.h"

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
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

        // If this occurs, we're most probably operating in headless mode.
        if (ui_module.get() == 0)
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

        ui_module->GetSceneManager()->AddSettingsWidget(settings_widget_, "Sound");

        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
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
    }
    
    void SoundSettings::MasterGainChanged(int value)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;
        soundsystem->SetMasterGain(value / 100.0);
    }

    void SoundSettings::TriggeredGainChanged(int value)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;
        soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Triggered, value / 100.0);
    }

    void SoundSettings::AmbientGainChanged(int value)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;
        soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Ambient, value / 100.0);
    }
    
    void SoundSettings::VoiceGainChanged(int value)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem.get())
            return;
        soundsystem->SetSoundMasterGain(Foundation::SoundServiceInterface::Voice, value / 100.0);
    }
}