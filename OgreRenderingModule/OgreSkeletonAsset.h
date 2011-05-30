// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IAsset.h"
#include "OgreModuleApi.h"
#include <OgreSkeleton.h>

/// An Ogre-specific skeleton resource, contains bone structure and skeletal animations
/** \ingroup OgreRenderingModuleClient */
class OGRE_MODULE_API OgreSkeletonAsset : public IAsset
{
    Q_OBJECT;
public:
    OgreSkeletonAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    virtual ~OgreSkeletonAsset();

    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "") const;

    virtual void DoUnload();

    bool IsLoaded() const;

    Ogre::SkeletonPtr ogreSkeleton;

    std::string internal_name_;
};

