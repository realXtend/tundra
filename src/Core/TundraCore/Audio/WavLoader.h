// For conditions of distribution and use, see copyright notice in LICENSE

#include <vector>
#include "CoreTypes.h"
#include "SoundBuffer.h"
#include "TundraCoreApi.h"

// Functions for loading uncompressed WAV format audio data.

namespace WavLoader
{
/// Loads a WAV file from memory buffer.
/// @param fileData [in] Points to a .wav file contents that has been loaded into memory.
/// @param numBytes The length of the input buffer fileData, in bytes.
/// @param dst [out] Stores the outputted raw PCM WAV data.
/// @param isStereo [out] Stores whether the WAV data is stereo (true) or mono (false).
/// @param is16Bit [out] Stores whether the WAV data is 16 bits per sample (true) or 8 bits per sample (false).
/// @param frequency [out] Stores the sample frequency of the WAV data.
bool TUNDRACORE_API LoadWavFromFileInMemory(const u8 *fileData, size_t numBytes, std::vector<u8> &dst, bool *isStereo, bool *is16Bit, int *frequency);

/// Loads the given .wav file in memory into a new SoundBuffer structure.
/// @param dst [out] This structure will receive the loaded sound data.
/// @return True on success, false oherwise.
inline bool TUNDRACORE_API LoadWavFileToSoundBuffer(const u8 *data, size_t numBytes, SoundBuffer &dst)
{
    return LoadWavFromFileInMemory(data, numBytes, dst.data, &dst.stereo, &dst.is16Bit, &dst.frequency);
}

/// Returns true the header of the given file in memory matches a .wav file.
bool TUNDRACORE_API IdentifyWavFileInMemory(const u8 *fileData, size_t numBytes);

} // ~WavLoader
