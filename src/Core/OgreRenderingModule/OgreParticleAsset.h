// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAsset.h"
#include "OgreModuleApi.h"

/// Represents an Ogre .particle file loaded to memory.
/** An Ogre-specific particle system resource. One resource may contain multiple templates. */
class OGRE_MODULE_API OgreParticleAsset : public IAsset
{
    Q_OBJECT

public:
    OgreParticleAsset(AssetAPI *owner, const QString &type, const QString &name) : IAsset(owner, type, name) {}
    ~OgreParticleAsset();

    virtual bool DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous);

    /// IAsset overload.
    /** The data will contain asset references in desanitated format. */
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "") const;

    virtual std::vector<AssetReference> FindReferences() const;

    /// Returns the number of templates in this particle system asset.
    size_t GetNumTemplates() const;

    /// Returns the name of the template at the given index.
    QString GetTemplateName(int index) const;

    bool IsLoaded() const;

private:
    virtual void DoUnload();

    /// Removes all particle system templates.
    void RemoveTemplates();

    StringVector templates; /// Stores the names of the loaded particle system templates.
    std::string internalName; ///< Internal Ogre name for the particle system.
    std::string originalData; ///< Original particle script file data, asset references are in sanitated format.
    std::vector<AssetReference> references; ///< references to other resources this resource depends on
};
