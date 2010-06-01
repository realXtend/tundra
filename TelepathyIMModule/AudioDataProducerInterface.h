// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_AudioDataProducerInterface_h
#define incl_Comm_AudioDataProducerInterface_h

#include <QObject>
#include "CoreTypes.h"

namespace Communication
{
    /** Object that produces raw audio data and keep it in local buffer until data
      * is requested with GetAudioData method call.
      */
    class AudioDataProducerInterface : public QObject
    {
        Q_OBJECT
    public:
        //! @return sample rate aka. sampling frequency. Return -1 if sample rate is unknwon.
        virtual int GetSampleRate() const = 0;

        //! @return sample width as bits. Return -1 if sample rate is unknwon.
        virtual int GetSampleWidth() const = 0;

        //! @return channel count. Usually returns 1 for mono or 2 for stereo stream
        virtual int GetChannelCount() const = 0;

        //! @return count of available bytes in buffer
        virtual int GetAvailableAudioDataLength() const = 0;

        //! Fill given buffer with audio data up to given max count of bytes or bytes available in buffer.
        //! The audio data is removed from local buffer when data is written to given buffer.
        //! @return the number of bytes wrote in given buffer
        virtual int GetAudioData(u8* buffer, int max) = 0;

    signals:
        //! Triggered when new audio data is available in buffer
        void AudioDataAvailable(int count);

        //! Triggered when new audio data was available but cannot be stored becouse buffer is full.
        void AudioBufferOverflow(int count);
    };

}

#endif

