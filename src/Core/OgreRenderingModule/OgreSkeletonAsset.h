// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAsset.h"
#include "OgreModuleApi.h"

#include <OgreSkeleton.h>
#include <OgreResourceBackgroundQueue.h>

/// An Ogre-specific skeleton resource, contains bone structure and skeletal animations
/** \ingroup OgreRenderingModuleClient */

class OGRE_MODULE_API OgreSkeletonAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{

Q_OBJECT

public:
    /// Constructor.
    OgreSkeletonAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
        IAsset(owner, type_, name_)
    {
    }

    /// Deconstructor.
    virtual ~OgreSkeletonAsset();

    /// IAsset override.
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, const bool allowAsynchronous);

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// IAsset override.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "") const;

    /// IAsset override.
    virtual void DoUnload();

    /// IAsset override.
    bool IsLoaded() const;

    /// Ogre Skeleton ptr.
    Ogre::SkeletonPtr ogreSkeleton;

    /// Internal name for the skeleton.
    std::string internal_name_;

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;
};

