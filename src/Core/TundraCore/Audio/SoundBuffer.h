// For conditions of distribution and use, see copyright notice in LICENSE
#pragma once

#include <vector>
#include "CoreTypes.h"
#include "TundraCoreApi.h"

/// Stores a fixed-length sound clip in raw PCM wav data.
class TUNDRACORE_API SoundBuffer
{
public:

    /// Stores the actual raw PCM wav data of the sound buffer.
    std::vector<u8> data;
    /// Specifies the number of samples per second this sound buffer contains. (8000, 16000, 22050, 44100, 48000, etc.)
    int frequency;
    /// If true, the data is stored as 16 bits per sample. If false, 8 bits per sample is used.
    bool is16Bit;
    /// If true, the data is stored in two-channel stereo mode (interleaved). If false, the data contains mono audio.
    bool stereo;
};

