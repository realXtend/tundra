// For conditions of distribution and use, see copyright notice in LICENSE

#include "CeltCodec.h"
#include "CoreTypes.h"

#include "SoundBuffer.h"

namespace MumbleAudio
{
    CeltCodec::CeltCodec() :
        bitStreamVersion(-1),    
        celtMode(0),
        encoder(0),
        decoder(0)
    {
        celtMode = celt_mode_create(MUMBLE_AUDIO_SAMPLE_RATE, MUMBLE_AUDIO_SAMPLES_IN_FRAME, NULL);
    }

    CeltCodec::~CeltCodec()
    {
        if (celtMode)
        {
            if (encoder)
            {
                celt_encoder_destroy(encoder);
                encoder = 0;
            }
            if (decoder)
            {
                celt_decoder_destroy(decoder);
                decoder = 0;
            }
            celt_mode_destroy(const_cast<CELTMode*>(celtMode));
        }
        celtMode = 0;
    }

    int CeltCodec::BitStreamVersion()
    {
        if (celtMode && bitStreamVersion == -1)
            celt_mode_info(celtMode, CELT_GET_BITSTREAM_VERSION, reinterpret_cast<celt_int32 *>(&bitStreamVersion));
        return bitStreamVersion;
    }

    int CeltCodec::Encode(SoundBuffer &pcmFrame, unsigned char *compressed, int bitrate)
    {
        celt_encoder_ctl(Encoder(), CELT_SET_PREDICTION(0));
        //celt_encoder_ctl(Encoder(), CELT_SET_VBR_RATE(bitrate));
        return celt_encode(Encoder(), (celt_int16*)&pcmFrame.data[0], MUMBLE_AUDIO_SAMPLES_IN_FRAME, compressed, qMin(bitrate / (8 * 100), 127));
    }

    int CeltCodec::Decode(const char *data, int dataLength, SoundBuffer &soundFrame)
    {
        soundFrame.data.resize(MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8);
        soundFrame.frequency = MUMBLE_AUDIO_SAMPLE_RATE;
        soundFrame.is16Bit = true;
        soundFrame.stereo = false;

        return celt_decode(Decoder(), (const unsigned char*)data, dataLength, (celt_int16*)&soundFrame.data[0], MUMBLE_AUDIO_SAMPLES_IN_FRAME);
    }

    CELTEncoder *CeltCodec::Encoder()
    {
        if (!encoder)
            encoder = celt_encoder_create_custom(celtMode, 1, NULL);
        return encoder;
    }

    CELTDecoder *CeltCodec::Decoder()
    {
        if (!decoder)
            decoder = celt_decoder_create_custom(celtMode, 1, NULL);
        return decoder;
    }
}
