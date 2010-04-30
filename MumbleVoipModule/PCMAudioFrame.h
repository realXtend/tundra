// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_PCMAudioFrame_h
#define incl_MumbleVoipModule_PCMAudioFrame_h

namespace MumbleVoip
{
    /**
     * Raw pulse code modulated audio data
     *
     */
    class PCMAudioFrame
    {
    public:
        //! Allocates memory for data
        PCMAudioFrame(int sample_rate, int sample_width, int channels, int data_size);

        //! Copies data from given source
        PCMAudioFrame(int sample_rate, int sample_widh, int channels, char* data, int data_size);

        //! Copies data from given audio frame
        PCMAudioFrame(PCMAudioFrame* frame);

        virtual ~PCMAudioFrame();
        virtual char* DataPtr();
        virtual int SampleAt(int i);
        virtual int Channels();
        virtual int SampleRate();
        virtual int SampleWidth();
        virtual int SampleCount();
        virtual int LengthMs();
        virtual int DataSize();

    private:
        int channels_;
        int sample_rate_;
        int sample_width_;
        char* data_;
        int data_size_;
    };

}// namespace MumbleVoip

#endif // incl_MumbleVoipModule_PCMAudioFrame_h
