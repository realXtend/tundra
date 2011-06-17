// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreSkeletonAsset_h
#define incl_OgreRenderer_OgreSkeletonAsset_h

#include "IAsset.h"
#include "OgreModuleApi.h"

#include <OgreSkeleton.h>
#include <OgreResourceBackgroundQueue.h>

//! An Ogre-specific skeleton resource, contains bone structure and skeletal animations
/*! \ingroup OgreRenderingModuleClient */

class OGRE_MODULE_API OgreSkeletonAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{

Q_OBJECT

public:
    /// Constructor.
    OgreSkeletonAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    /// Deconstructor.
    ~OgreSkeletonAsset();

    /// IAsset override.
    virtual AssetLoadState DeserializeFromData(const u8 *data_, size_t numBytes);

    /// IAsset override.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// IAsset override.
    virtual void DoUnload();

    bool IsLoaded() const;

    /// Ogre Skelton ptr.
    Ogre::SkeletonPtr ogreSkeleton;

    /// Internal name for the skeleton.
    std::string internal_name_;

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;
};

#endif
