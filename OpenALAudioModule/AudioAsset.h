// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudioModule_AudioAsset_h
#define incl_OpenALAudioModule_AudioAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "ISoundService.h"

class AudioAsset : public IAsset
{
    Q_OBJECT;
public:
    AudioAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    virtual bool DeserializeFromData(const u8 *data, size_t numBytes)
    {

    }

    ISoundService::SoundBuffer audioData;
};

typedef boost::shared_ptr<AudioAsset> AudioAssetPtr;

#endif
