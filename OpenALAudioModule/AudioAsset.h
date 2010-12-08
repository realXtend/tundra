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
    AudioAsset(const QString &type_, const QString &name_)//, const QString &ref_)
    :IAsset(type_, name_)//, ref_)
    {
    }

    virtual bool LoadFromFileInMemory(const u8 *data, size_t numBytes)
    {

    }

    ISoundService::SoundBuffer audioData;
};

typedef boost::shared_ptr<AudioAsset> AudioAssetPtr;

#endif
