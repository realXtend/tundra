// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_PCMAudioFrame_h
#define incl_MumbleVoipModule_PCMAudioFrame_h

namespace MumbleVoip
{
    /**
     * Makes copy of given data when constructed. Data will be freed in deconstructor.
     *
     */
    class PCMAudioFrame
    {
    public:
        //! Copies data from given source
        PCMAudioFrame(int sample_rate, int sample_widh, int channels, char* data, int data_size);

        ////! Creates
        //PCMAudioFrame(int sample_rate, int sample_widh, int channels, int data_size);

        virtual ~PCMAudioFrame();
        virtual char* Data();
        virtual int Channels();
        virtual int SampleRate();
        virtual int SampleWidth();
        virtual int Samples();
        virtual int GetLengthMs();
        virtual int GetLengthBytes();

    private:
        int channels_;
        int sample_rate_;
        int sample_width_;
        char* data_;
        int data_size_;
    };

}// namespace MumbleVoip

#endif // incl_MumbleVoipModule_PCMAudioFrame_h
