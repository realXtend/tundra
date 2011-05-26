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
    /// Makes a clone of this asset.
    /// For this function to succeed, the asset must be loaded in memory. (IsLoaded() == true)
    /// @param newAssetName The name for the new asset. This will be the 'assetRef' of the new asset
    virtual AssetPtr Clone(QString newAssetName) const;

    /// Copy content from another OgreMaterialAsset using Ogre internal functions, without having to serialize/deserialize
    void CopyContent(AssetPtr source);
    
    /// Set a material attribute using a key-value format, where key is "t<x> p<y> tu<z> paramname", to access technique, pass and
    /// texture unit specific attributes. These can also be omitted to affect all techniques, passes or units as applicable.
    void SetAttribute(const QString& key, const QString& value);

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
    
    /// Set texture in a texture unit. Return true if successful.
    bool SetTexture(int techIndex, int passIndex, int texUnitIndex, const QString& assetRef);
    /// Set vertex shader of the pass. Depending on Ogre, this may cause any nasty things to happen. Return true if did not cause an Ogre exception
    bool SetVertexShader(int techIndex, int passIndex, const QString& vertexShaderName);
    /// Set pixel shader of the pass. Depending on Ogre, this may cause any nasty things to happen. Return true if did not cause an Ogre exception
    bool SetPixelShader(int techIndex, int passIndex, const QString& pixelShaderName);
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
    /// Set scene blend mode of a pass
    bool SetSceneBlend(int techIndex, int passIndex, unsigned blendMode);
    /// Set polygon mode of a pass
    bool SetPolygonMode(int techIndex, int passIndex, unsigned polygonMode);
    /// Set depth check on/off
    bool SetDepthCheck(int techIndex, int passIndex, bool enable);
    /// Set depth write on/off
    bool SetDepthWrite(int techIndex, int passIndex, bool enable);
    /// Set constant depth bias
    bool SetDepthBias(int techIndex, int passIndex, float bias);

private slots:
    /// Asset transfer (for texture apply operation) succeeded
    void OnTransferSucceeded(AssetPtr asset);
    /// Asset transfer (for texture apply operation) failed
    void OnTransferFailed(IAssetTransfer *transfer, QString reason);
    
private:
    bool SetMaterialAttribute(const QString& attr, const QString& val, const QString& origVal);
    bool SetTechniqueAttribute(Ogre::Technique* tech, int techIndex, const QString& attr, const QString& val, const QString& origVal);
    bool SetPassAttribute(Ogre::Pass* pass, int techIndex, int passIndex, const QString& attr, const QString& val, const QString& origVal);
    bool SetTextureUnitAttribute(Ogre::TextureUnitState* texUnit, int techIndex, int passIndex, int tuIndex, const QString& attr, const QString& val, const QString& origVal);
    
    /// Pending texture apply operation due to a texture asset request
    struct PendingTextureApply
    {
        int techIndex;
        int passIndex;
        int tuIndex;
        IAssetTransfer* transfer;
    };
    
    std::vector<PendingTextureApply> pendingApplies;
};

#endif


