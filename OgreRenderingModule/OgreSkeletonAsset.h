// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreSkeletonAsset_h
#define incl_OgreRenderer_OgreSkeletonAsset_h

#include "IAsset.h"
#include "OgreModuleApi.h"
#include <OgreSkeleton.h>

//! An Ogre-specific skeleton resource, contains bone structure and skeletal animations
/*! \ingroup OgreRenderingModuleClient */
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

    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "");

    virtual void DoUnload();

    bool IsLoaded() const;

    Ogre::SkeletonPtr ogreSkeleton;

    std::string internal_name_;
};

#endif
