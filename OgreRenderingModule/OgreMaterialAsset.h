// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreMaterialAsset_h
#define incl_OgreRenderer_OgreMaterialAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "Color.h"
#include <OgreMaterial.h>
#include "OgreModuleApi.h"

class OGRE_MODULE_API OgreMaterialAsset : public IAsset
{

Q_OBJECT

public:
    OgreMaterialAsset(AssetAPI *owner, const QString &type_, const QString &name_) : IAsset(owner, type_, name_) {}
    ~OgreMaterialAsset();

    /// Load material from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load material into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Unload material from ogre
    virtual void DoUnload();

    /// Handle load errors detected by AssetAPI
    virtual void HandleLoadError(const QString &loadError);

    /// Return references of this material, in most cases textures
    virtual std::vector<AssetReference> FindReferences() const;

    bool IsLoaded() const;

    /// Material ptr to the asset in ogre
    Ogre::MaterialPtr ogreMaterial;

    /// Specifies the unique texture name Ogre uses in its asset pool for this material.
    QString ogreAssetName;

    /// references to other resources this resource depends on
    std::vector<AssetReference> references_;
    
    /// Function that safely returns a technique, or 0 if did not exist
    Ogre::Technique* GetTechnique(int techIndex);
    /// Function that safely returns a pass, or 0 if did not exist
    Ogre::Pass* GetPass(int techIndex, int passIndex);
    /// Function that safely returns a texture unit, or 0 if did not exist
    Ogre::TextureUnitState* GetTextureUnit(int techIndex, int passIndex, int texUnitIndex);
    
public slots:
    /// Return number of material techniques. Returns -1 if the material is unloaded
    int GetNumTechniques();
    /// Return number of passes in a technique. Returns -1 if the technique does not exist
    int GetNumPasses(int techIndex);
    /// Return number of texture units in a pass. Returns -1 if the pass does not exist
    int GetNumTextureUnits(int techIndex, int passIndex);
    /// Return whether has a technique by index
    bool HasTechnique(int techIndex);
    /// Return whether a technique has a pass by index
    bool HasPass(int techIndex, int passIndex);
    
    /// Create a new technique. Its index number will be returned, or -1 if could not be created
    int CreateTechnique();
    /// Create a new pass to a technique. Its index number will be returned, or -1 if could not be created
    int CreatePass(int techIndex);
    /// Create a new texture unit to a technique. Its index number will be returned, or -1 if could not be created
    int CreateTextureUnit(int techIndex, int passIndex);
    /// Remove a texture unit from a pass. Returns true if successful. Note: texture unit indices will be adjusted so that they are continuous
    bool RemoveTextureUnit(int techIndex, int passIndex, int texUnitIndex);
    /// Remove a pass from a technique. Returns true if successful. Note: pass indices will be adjusted so that they are continuous
    bool RemovePass(int techIndex, int passIndex);
    /// Remove a technique. Returns true if successful. Note: technique indices will be adjusted so that they are continuous
    bool RemoveTechnique(int techIndex);
    
    /// Set texture in a texture unit. The AssetPtr must contain a loaded TextureAsset. Return true if successful.
    bool SetTexture(int techIndex, int passIndex, int texUnitIndex, AssetPtr texture);
    /// Set shader names of the pass. Depending on Ogre, this may cause any nasty things to happen. Returns true if did not cause an Ogre exception
    bool SetShaders(int techIndex, int passIndex, const QString& vertexProgramName, const QString& fragmentProgramName);
    /// Enable or disable lighting in a pass
    bool SetLighting(int techIndex, int passIndex, bool enable);
    /// Set diffuse color of a pass. Return true if successful.
    bool SetDiffuseColor(int techIndex, int passIndex, const Color& color);
    /// Set ambient color of a pass
    bool SetAmbientColor(int techIndex, int passIndex, const Color& color);
    /// Set specular color of a pass
    bool SetSpecularColor(int techIndex, int passIndex, const Color& color);
    /// Set emissive color of a pass
    bool SetEmissiveColor(int techIndex, int passIndex, const Color& color);
    /// Set replace blend mode
    bool SetReplace(int techIndex, int passIndex);
    /// Set alpha blend mode
    bool SetAlphaBlend(int techIndex, int passIndex);
    /// Set additive blend mode
    bool SetAdditive(int techIndex, int passIndex);
    /// Set modulate blend mode
    bool SetModulate(int techIndex, int passIndex);
    /// Set solid polygon mode
    bool SetSolid(int techIndex, int passIndex);
    /// Set wireframe polygon mode
    bool SetWireframe(int techIndex, int passIndex);
    /// Set depth write on/off
    bool SetDepthWrite(int techIndex, int passIndex, bool enable);
    /// Set constant depth bias
    bool SetDepthBias(int techIndex, int passIndex, float bias);
};

#endif


