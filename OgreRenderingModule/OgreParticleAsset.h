// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreParticleAsset_h
#define incl_OgreRenderer_OgreParticleAsset_h

#include "IAsset.h"
#include "OgreModuleApi.h"

//! An Ogre-specific particle system template resource. One resource may contain multiple templates.
/*! \ingroup OgreRenderingModuleClient */
class OGRE_MODULE_API OgreParticleAsset : public IAsset
{
    Q_OBJECT;
public:
    OgreParticleAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~OgreParticleAsset();

    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    virtual std::vector<AssetReference> FindReferences() const;

    virtual void DoUnload();
    
    /// Returns the number of templates in this particle system asset.
    int GetNumTemplates() const;
    
    /// Returns the name of the template at the given index.
    QString GetTemplateName(int index) const;

private:
    /// Removes all particle system templates.
    void RemoveTemplates();
    
    /// Stores the names of the loaded particle system templates.
    StringVector templates_;

    std::string internal_name_;

    //! references to other resources this resource depends on
    std::vector<AssetReference> references_;

};

#endif
