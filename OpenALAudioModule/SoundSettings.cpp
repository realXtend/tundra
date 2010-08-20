// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SoundSettings.h"
#include "OpenALAudioModule.h"

#include "Framework.h"
#include "ServiceManager.h"
#include "SoundServiceInterface.h"
#include "UiServiceInterface.h"

#include <QUiLoader>
#include <QFile>
#include <QAbstractSlider>

#include "MemoryLeakCheck.h"

namespace OpenALAudio
{
    SoundSettings::SoundSettings(Foundation::Framework* framework) : framework_(framework)
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

        QWidget *settings_widget = loader.load(&file); 
        if (!settings_widget)
            return;

        ui->AddSettingsWidget(settings_widget, "Sound");

        Foundation::SoundServiceInterface *soundsystem = framework_->GetService<Foundation::SoundServiceInterface>();
        if (!soundsystem)
            return;
        QAbstractSlider* slider = settings_widget->findChild<QAbstractSlider*>("slider_master");
        if (slider)
        {
            slider->setValue(soundsystem->GetMasterGain() * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterGainChanged(int)));
        }
        slider = settings_widget->findChild<QAbstractSlider*>("slider_triggered");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Triggered) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(TriggeredGainChanged(int)));
        }
        slider = settings_widget->findChild<QAbstractSlider*>("slider_ambient");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Ambient) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(AmbientGainChanged(int)));
        }
        slider = settings_widget->findChild<QAbstractSlider*>("slider_voice");
        if (slider)
        {
            slider->setValue(soundsystem->GetSoundMasterGain(Foundation::SoundServiceInterface::Voice) * 100);
            QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(VoiceGainChanged(int)));
        }
    }

    SoundSettings::~SoundSettings()
    {
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
