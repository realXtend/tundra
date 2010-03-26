// For conditions of distribution and use, see copyright notice in license.txt

#include "Sound.h"

namespace OpenALAudio
{
    //! Functions for loading uncompressed WAV format audio data. Used by SoundSystem
    class WavLoader
    {
    public:
        //! Load WAV from file
        static bool LoadFromFile(Sound* sound, const std::string& filename);

        //! Load WAV from memory buffer
        static bool LoadFromBuffer(Sound* sound, u8* data, uint size);
    };
    
}