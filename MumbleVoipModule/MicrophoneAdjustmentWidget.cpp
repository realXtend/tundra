// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MicrophoneAdjustmentWidget.h"
#include "Settings.h"
#include "PCMAudioFrame.h"
#include "SoundServiceInterface.h"
#include "MumbleDefines.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    MicrophoneAdjustmentWidget::MicrophoneAdjustmentWidget(Foundation::Framework* framework, Settings* settings) :
        QWidget(),
        framework_(framework),
        settings_(settings),
        sound_id_(0),
        voice_activity_level_(0)
    {
        setupUi(this);
        connect(&read_audio_timer_, SIGNAL(timeout()), this, SLOT(HandleRecordedAudio()));
        connect(microphoneLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateUI()));
        connect(microphoneLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(ApplySliderValue()));
        connect(saveButton, SIGNAL(clicked()), this, SLOT(SaveSettings()));
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
        read_audio_timer_.start(20);

        microphone_level_ = settings_->GetMicrophoneLevel();
        microphoneLevelSlider->setValue(static_cast<int>(microphone_level_*100));

        UpdateUI();

        StartAudioRecording();
    }

    MicrophoneAdjustmentWidget::~MicrophoneAdjustmentWidget()
    {
        read_audio_timer_.stop();
    }

    void MicrophoneAdjustmentWidget::StartAudioRecording()
    {
        if (!framework_)
            return;

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

        if (!service_manager.get())
            return;

        boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();

        if (!sound_service.get())
            return;

        int frequency = SAMPLE_RATE;
        bool sixteenbit = true;
        bool stereo = false;
        int buffer_size = SAMPLE_WIDTH/8*frequency*AUDIO_RECORDING_BUFFER_MS_/1000;
        sound_service->StartRecording("", frequency, sixteenbit, stereo, buffer_size);
    }

    void MicrophoneAdjustmentWidget::HandleRecordedAudio()
    {
        if (!framework_)
            return;

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

        if (!service_manager.get())
            return;

        boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();

        if (!sound_service.get())
            return;

        int bytes_per_frame = SAMPLES_IN_FRAME*SAMPLE_WIDTH/8;
        PCMAudioFrame frame(SAMPLE_RATE, SAMPLE_WIDTH, NUMBER_OF_CHANNELS, bytes_per_frame );
        while (sound_service->GetRecordedSoundSize() > bytes_per_frame)
        {
            int bytes = sound_service->GetRecordedSoundData(frame.DataPtr(), bytes_per_frame);
            if (bytes != bytes_per_frame)
            {
                return;
            }
            ApplyMicrophoneLevel(&frame);
            PlaybackAudioFrame(&frame);
        }
        UpdateUI();
    }

    void MicrophoneAdjustmentWidget::PlaybackAudioFrame(PCMAudioFrame* frame)
    {
        if (!framework_)
            return ;

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

        if (!service_manager.get())
            return ;

        boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();

        if (!sound_service)
            return ;

        Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
        
        sound_buffer.data_.resize(frame->DataSize());
        memcpy(&sound_buffer.data_[0], frame->DataPtr(), frame->DataSize());

        sound_buffer.frequency_ = frame->SampleRate();
        if (frame->SampleWidth() == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        
        if (frame->Channels() == 2)
            sound_buffer.stereo_ = true;
        else
            sound_buffer.stereo_ = false;

        sound_id_ = sound_service->PlaySoundBuffer(sound_buffer, Foundation::SoundServiceInterface::Voice, sound_id_);
    }

    void MicrophoneAdjustmentWidget::UpdateUI()
    {
        audioLevelProgressBar->setValue(static_cast<int>(100.0*voice_activity_level_+0.5));
        microphoneLevelLabel->setText( QString("%1 %").arg(microphoneLevelSlider->value()) );
    }

    void MicrophoneAdjustmentWidget::ApplyMicrophoneLevel(PCMAudioFrame* frame)
    {
        int top_value = 0;
        int bottom_value = 0; 
        double microphone_level = microphoneLevelSlider->value() * 0.01;
        for(int i = 0; i < frame->SampleCount(); ++i)
        {
            int sample = static_cast<int>( static_cast<double>(frame->SampleAt(i)) * microphone_level );
            if (sample > top_value)
                top_value = sample;
            if (sample < bottom_value)
                bottom_value = sample;

            frame->SetSampleAt(i, sample);
        }

        switch (frame->SampleWidth())
        {
        case 8:
            voice_activity_level_ = 0.001*((1000 * std::max(abs(top_value), abs(bottom_value))) >> 7);
            break;
        case 16:
            voice_activity_level_ = 0.001*((1000 * std::max(abs(top_value), abs(bottom_value))) >> 15);
            break;
        }
        if (voice_activity_level_ > 1)
            voice_activity_level_ = 1;
        if (voice_activity_level_ < 0)
            voice_activity_level_ = 0;
    }

    void MicrophoneAdjustmentWidget::SaveSettings()
    {
        settings_->SetMicrophoneLevel( static_cast<double>(microphoneLevelSlider->value()*0.01) );
        close();
    }

    void MicrophoneAdjustmentWidget::ApplySliderValue()
    {
        microphone_level_ = static_cast<double>(microphoneLevelSlider->value()*0.01);
    }
 
} // MumbleVoip
