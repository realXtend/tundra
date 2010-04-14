// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AudioSignalLabel.h
 *  @brief  Inherited by QLabel will convert the audio signal into visual format.
 *  Label will resize the signal image when it's size is changed.
 */

#ifndef incl_OgreAssetEditorModule_AudioSignalLabel_h
#define incl_OgreAssetEditorModule_AudioSignalLabel_h

#include <RexTypes.h>
#include <QLabel>

namespace Naali
{
    class AudioSignalLabel: public QLabel
    {
        Q_OBJECT
    public:
        AudioSignalLabel(QWidget *parent = 0, Qt::WindowFlags flags = 0);
        virtual ~AudioSignalLabel();
        //! Set new audio data into label.
        void SetAudioData(std::vector<u8> &data, uint frequency, uint bits, bool stereo);
        //! Get audio clip durtation in seconds.
        float GetAudioDuration() const;
    private slots:
        //! Generate new audio signal using soundBuffer information.
        void GenerateAudioSignalImage();
        //! Resize label and regenerate a new audio signal image.
        void ResizeImage(QSize size);

    private:

        //! Audio data vector.
        std::vector<u8> audioData_;
        //! What is audio file's frequency
        uint frequency_;
        //! How many bits is used per audio sample.
        uint bitsPerSample_;
        //! Is audio file using stereo sound.
        bool stereo_;
        //! Audio clip duration in seconds.
        float duration_;
    };
}

#endif