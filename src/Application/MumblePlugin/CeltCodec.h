// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleFwd.h"
#include "MumbleDefines.h"

#include <QObject>
#include <celt/celt.h>

class SoundBuffer;

namespace MumbleAudio
{
    class CeltCodec : public QObject
    {
    Q_OBJECT

    public:
        CeltCodec();
        ~CeltCodec();

        int BitStreamVersion();

        CELTEncoder *Encoder();
        CELTDecoder *Decoder();

        int Encode(const SoundBuffer &pcmFrame, unsigned char *compressed, int bitrate);
        int Decode(const char *data, int dataLength, SoundBuffer &soundFrame);

    private:
        CELTMode *celtMode;
        CELTEncoder *encoder;
        CELTDecoder *decoder;

        int bitStreamVersion;
    };
}
