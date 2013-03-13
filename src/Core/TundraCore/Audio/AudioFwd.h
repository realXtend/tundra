// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include <map>

class AudioAPI;

class SoundBuffer;

typedef unsigned int sound_id_t;

class SoundChannel;
typedef shared_ptr<SoundChannel> SoundChannelPtr;
typedef weak_ptr<SoundChannel> SoundChannelWeakPtr;

typedef std::map<sound_id_t, SoundChannelPtr> SoundChannelMap;

class AudioAsset;
typedef shared_ptr<AudioAsset> AudioAssetPtr;
typedef weak_ptr<AudioAsset> AudioAssetWeakPtr;

// We don't want to include the OpenAL headers here directly (<AL/al.h>, <AL/alc.h>). Pulled the necessary declarations here directly.
/** unsigned 32-bit integer */
typedef unsigned int ALuint;
