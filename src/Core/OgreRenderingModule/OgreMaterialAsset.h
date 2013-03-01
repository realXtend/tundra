// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IAsset.h"
#include "Color.h"

#include <OgreMaterial.h>

/// Contains an Ogre .material loaded to memory.
class OGRE_MODULE_API OgreMaterialAsset : public IAsset
{
    Q_OBJECT

public:
    OgreMaterialAsset(AssetAPI *owner, const QString &type_, const QString &name_) : IAsset(owner, type_, name_) {}
    ~OgreMaterialAsset();

    /// IAsset overload.
    /// Loads material from memory.
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous);

    /// IAsset overload.
    /** The @c data will contain asset references in desanitated format. */
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Unload material from ogre
    virtual void DoUnload();

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
    Ogre::Technique* GetTechnique(int techIndex) const;
    /// Function that safely returns a pass, or 0 if did not exist
    Ogre::Pass* GetPass(int techIndex, int passIndex) const;
    /// Function that safely returns a texture unit, or 0 if did not exist
    Ogre::TextureUnitState* GetTextureUnit(int techIndex, int passIndex, int texUnitIndex) const;

public slots:
    /// Makes a clone of this asset.
    /// For this function to succeed, the asset must be loaded in memory. (IsLoaded() == true)
    /// @param newAssetName The name for the new asset. This will be the 'assetRef' of the new asset
    virtual AssetPtr Clone(QString newAssetName) const;

    /// Copy content from another OgreMaterialAsset using Ogre internal functions, without having to serialize/deserialize
    void CopyContent(AssetPtr source);
    
    /// Only call this after you have cloned a material and set all of its new attributes.
    /** Calls AssetAPI::AssetLoadCompleted() for this asset that will trigger the proper
        flow in AssetAPI to listen when all dependencies are completed and the emits the 
        Loaded signals for this asset. */
    void CloneCompleted();
    
    /// Set a material attribute using a key-value format.
    /** Format: key is "t<x> p<y> tu<z> paramname", to access technique, pass and texture unit specific attributes.
        These can also be omitted to affect all techniques, passes or units as applicable. */
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
    QString Texture(int techIndex, int passIndex, int texUnitIndex) const;

    /// Set vertex shader of the pass. Depending on Ogre, this may cause any nasty things to happen. Return true if did not cause an Ogre exception
    bool SetVertexShader(int techIndex, int passIndex, const QString& vertexShaderName);
    QString VertexShader(int techIndex, int passIndex) const;

    /// Set pixel shader of the pass. Depending on Ogre, this may cause any nasty things to happen. Return true if did not cause an Ogre exception
    bool SetPixelShader(int techIndex, int passIndex, const QString& pixelShaderName);
    QString PixelShader(int techIndex, int passIndex) const;

    /// Sets vertex shader parameter value.
    /** Supported value types are int, float, float4/Color, float4x4.
        @param techIndex Technique index.
        @param passIndexx Pass index.
        @param name Name of the shader parameter.
        @param value List of values to be set. */
    bool SetVertexShaderParameter(int techIndex, int passIndex, const QString& name, const QVariantList &value);

    /// Sets pixel shader parameter value.
    /** Supported value types are int, float, float4/Color, float4x4.
        @param techIndex Technique index.
        @param passIndexx Pass index.
        @param name Name of the shader parameter.
        @param value List of values to be set. */
    bool SetPixelShaderParameter(int techIndex, int passIndex, const QString& name, const QVariantList &value);

    /// Enable or disable lighting in a pass
    bool SetLighting(int techIndex, int passIndex, bool enable);
    bool IsLightingEnabled(int techIndex, int passIndex) const;

    /// Set diffuse color of a pass. Return true if successful.
    bool SetDiffuseColor(int techIndex, int passIndex, const Color& color);
    Color DiffuseColor(int techIndex, int passIndex) const;

    /// Set ambient color of a pass
    bool SetAmbientColor(int techIndex, int passIndex, const Color& color);
    Color AmbientColor(int techIndex, int passIndex) const;

    /// Set specular color of a pass
    bool SetSpecularColor(int techIndex, int passIndex, const Color& color);
    Color SpecularColor(int techIndex, int passIndex) const;

    /// Set emissive color of a pass
    bool SetEmissiveColor(int techIndex, int passIndex, const Color& color);
    Color EmissiveColor(int techIndex, int passIndex) const;

    /// Sets the overall scene blend mode of a pass
    /** See Ogre::SceneBlendType for @c blendMode */
    bool SetSceneBlend(int techIndex, int passIndex, unsigned blendMode);
    /// @overload
    /** Sets the source and destination blend factors separately.
        See Ogre::SceneBlendFactor for @c srcFactor and @c dstFactor. */
    bool SetSceneBlend(int techIndex, int passIndex, unsigned srcFactor, unsigned dstFactor);
    unsigned SourceSceneBlendFactor(int techIndex, int passIndex) const;
    unsigned DestinationSceneBlendFactor(int techIndex, int passIndex) const;

    /// Set polygon mode of a pass
    /** See Ogre::PolygonMode for @c polygonMode */
    bool SetPolygonMode(int techIndex, int passIndex, unsigned polygonMode);
    unsigned PolygonMode(int techIndex, int passIndex) const;

    /// Set depth check on/off
    bool SetDepthCheck(int techIndex, int passIndex, bool enable);
    bool IsDepthCheckEnabled(int techIndex, int passIndex) const;

    /// Set depth write on/off
    bool SetDepthWrite(int techIndex, int passIndex, bool enable);
    bool IsDepthWriteEnabled(int techIndex, int passIndex) const;

    /// Set constant depth bias
    bool SetDepthBias(int techIndex, int passIndex, float bias);
    float DepthBias(int techIndex, int passIndex) const;

    /// Sets the hardware culling mode.
    /** See Ogre::CullingMode for @c mode. */
    bool SetHardwareCullingMode(int techIndex, int passIndex, unsigned mode);
    unsigned HardwareCullingMode(int techIndex, int passIndex) const;

    /// Sets the (polygon) shading mode.
    /** See Ogre::ShadeOptions for @c mode. */
    bool SetShadingMode(int techIndex, int passIndex, unsigned mode);
    unsigned ShadingMode(int techIndex, int passIndex) const;

    /// Sets the (polygon) fill mode.
    /** See Ogre::PolygonMode for @c mode. */
    bool SetFillMode(int techIndex, int passIndex, unsigned mode);
    unsigned FillMode(int techIndex, int passIndex) const;

    /// Sets color write on/off
    bool SetColorWrite(int techIndex, int passIndex, bool enable);
    bool IsColorWriteEnabled(int techIndex, int passIndex) const;

    /// Sets texture coordinate set for texture layer.
    bool SetTextureCoordSet(int techIndex, int passIndex, int texUnitIndex, uint value);
    uint TextureCoordSet(int techIndex, int passIndex, int texUnitIndex) const;

    /// Sets the (polygon) fill mode.
    /** Sets the same mode for u, v and w.
        See Ogre::TextureUnitState::TextureAddressingMode for @c mode. */
    bool SetTextureAddressingMode(int techIndex, int passIndex, int texUnitIndex, unsigned mode);
    /// @overload
    /** Specifies the mode for u, v and w separately */
    bool SetTextureAddressingMode(int techIndex, int passIndex, int texUnitIndex, unsigned uMode, unsigned vMode, unsigned wMode);
    unsigned TextureAddressingModeU(int techIndex, int passIndex, int texUnitIndex) const;
    unsigned TextureAddressingModeV(int techIndex, int passIndex, int texUnitIndex) const;
    unsigned TextureAddressingModeW(int techIndex, int passIndex, int texUnitIndex) const;

    /// Sets animated scroll for texture layer.
    bool SetScrollAnimation(int techIndex, int passIndex, int texUnitIndex, float uSpeed, float vSpeed);
    float ScrollAnimationU(int techIndex, int passIndex, int texUnitIndex) const;
    float ScrollAnimationV(int techIndex, int passIndex, int texUnitIndex) const;

    /// Sets animated rotation for texture layer.
    bool SetRotateAnimation(int techIndex, int passIndex, int texUnitIndex, float speed);
    float RotateAnimation(int techIndex, int passIndex, int texUnitIndex) const;

    /// Returns whether or not texture unit has specific effect.
    /** See Ogre::TextureUnitState::TextureEffectType for @c effect.
        @note Ogre weirdness: if texture has scroll effect of which u and v values are the same, it has Ogre Ogre::TextureUnitState::ET_UVSCROLL.
        If it has scroll effect with different u and v values it has both Ogre::TextureUnitState::ET_USCROLL and Ogre::TextureUnitState::ET_VSCROLL. */
    bool HasTextureEffect(int techIndex, int passIndex, int texUnitIndex, unsigned effect) const;

private slots:
    /// Asset transfer (for texture apply operation) succeeded
    void OnTransferSucceeded(AssetPtr asset);
    /// Asset transfer (for texture apply operation) failed
    void OnTransferFailed(IAssetTransfer *transfer, QString reason);

private:
    void DependencyLoaded(AssetPtr dependee);
    bool CreateOgreMaterial();
    bool CreateOgreMaterial(const std::string& materialData);
    bool SetMaterialAttribute(const QString& attr, const QString& val, const QString& origVal);
    bool SetTechniqueAttribute(Ogre::Technique* tech, int techIndex, const QString& attr, const QString& val, const QString& origVal);
    bool SetPassAttribute(Ogre::Pass* pass, int techIndex, int passIndex, const QString& attr, const QString& val, const QString& origVal);
    bool SetTextureUnitAttribute(Ogre::TextureUnitState* texUnit, int techIndex, int passIndex, int tuIndex, const QString& attr, const QString& val, const QString& origVal);
    
    /// Contains the original ogre material data that was downloaded, but with all material and texture references
    /// rewritten to refer to assets loaded from the Tundra Asset system (and not Ogre resource managers).
    std::string parsedOgreMaterialAsset;
    int numDependenciesCompleted;

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

