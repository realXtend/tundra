#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AudioSignalLabel.h"
#include "OgreAssetEditorModule.h"

#include "MemoryLeakCheck.h"

namespace Naali
{
    AudioSignalLabel::AudioSignalLabel(QWidget *parent, Qt::WindowFlags flags):
        QLabel(parent, flags)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setScaledContents(true);
    }

    AudioSignalLabel::~AudioSignalLabel()
    {

    }

    void AudioSignalLabel::SetAudioData(std::vector<u8> &data, uint frequency, uint bits, bool stereo)
    {
        audioData_ = data;
        frequency_ = frequency;
        bitsPerSample_ = bits;
        stereo_ = stereo;
        GenerateAudioSignalImage();
    }

    float AudioSignalLabel::GetAudioDuration() const
    {
        return duration_;
    }

    void AudioSignalLabel::GenerateAudioSignalImage()
    {
        if(bitsPerSample_ <= 0 || frequency_ <= 0 || audioData_.empty())
            return;

        QSize newImageSize(width() * 0.95f, height() * 0.95f);
        /*if(pixmap())
        {
            QSize imageSize = pixmap()->size();
            // area size has not increased enought so no need to update the image just stretch it.
            if(abs(newImageSize.width() - imageSize.width()) <= 10 && abs(newImageSize.height() - imageSize.height()) <= 10)
                return;
        }*/

        QImage image(newImageSize, QImage::Format_RGB888);
        image.fill(qRgb(0,0,0));

        int channels = 2;
        if(!stereo_)
            channels = 1; 

        // Calculate audio file's total duration in seconds.
        duration_ = float(audioData_.size() / ((bitsPerSample_ / 8) * channels)) / float(frequency_);

        // Calcute how many audio samples are included per image pixel (usually one pixel contain many audio samples).
        float samples_per_pixel = (float(audioData_.size()) / float(bitsPerSample_ / 8)) / float(image.width());
        // Take inverse of samples per pixels value cause image width might contain more pixels than we have samples in audio file.
        // and in that case we need to repeat same audio value in many pixel.
        float steps = 1.0f / samples_per_pixel;
        QPair<int, int> min_max_values;
        min_max_values.first = 32767;
        min_max_values.second = -32768;

        short* sample_ptr;
        float sampleNum = 0;
        int image_width_index = 0;
        int steps_per_sample = bitsPerSample_ / 8;
        int half_image_height = image.height() / 2;
        for(uint i = 0; i < audioData_.size(); i += steps_per_sample)
        {
            sample_ptr = (short*)&audioData_[i];
            if(min_max_values.first > *sample_ptr)
                min_max_values.first = *sample_ptr;
            if(min_max_values.second < *sample_ptr)
                min_max_values.second = *sample_ptr;

            if(sampleNum >= 1.0f)
            {
                double normalized_max_value = (double(min_max_values.second) + 32768) / double(65536);
                double normalized_min_value = (double(min_max_values.first) + 32768) / double(65536);
                int start_height = (normalized_min_value * image.height());
                int end_height = (normalized_max_value * image.height());
                if(start_height == end_height && start_height > 0)
                    start_height--;

                while(sampleNum >= 1.0f)
                {
                    //Draw a verticalline between min and max signal values.
                    for(int i = start_height; i < end_height; i++)
                        image.setPixel(image_width_index, i, qRgb(0, 255, 0));
                    image_width_index++;
                    sampleNum -= 1.0f;
                }
                
                min_max_values.first = 32767;
                min_max_values.second = -32768;
            }
            sampleNum += steps;
        }
        setPixmap(QPixmap::fromImage(image));
    }

    void AudioSignalLabel::ResizeImage(QSize size)
    {
        setMaximumSize(QSize(size.width(), size.height() * 0.66f));
        resize(size);
        GenerateAudioSignalImage();
    }
}