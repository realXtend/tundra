// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Audio_AudioFwd_h
#define incl_Audio_AudioFwd_h

#include <map>
#include <boost/shared_ptr.hpp>

class AudioAPI;
struct AudioApiImpl;

class SoundBuffer;

typedef unsigned int sound_id_t;

class SoundChannel;
typedef boost::shared_ptr<SoundChannel> SoundChannelPtr;
typedef boost::weak_ptr<SoundChannel> SoundChannelWeakPtr;

typedef std::map<sound_id_t, SoundChannelPtr> SoundChannelMap;

class AudioAsset;
typedef boost::shared_ptr<AudioAsset> AudioAssetPtr;
typedef boost::weak_ptr<AudioAsset> AudioAssetWeakPtr;

// We don't want to include the OpenAL headers here directly (<AL/al.h>, <AL/alc.h>). Pulled the necessary declarations here directly.
/** unsigned 32-bit integer */
typedef unsigned int ALuint;

#endif
