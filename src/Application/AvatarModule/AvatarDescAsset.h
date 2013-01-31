// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AvatarDescHelpers.h"
#include "IAsset.h"

class QDomDocument;
class QDomElement;

/// Avatar appearance description asset
class AV_MODULE_API AvatarDescAsset : public IAsset
{
    Q_OBJECT

public:
    AvatarDescAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    ~AvatarDescAsset();

    virtual void DoUnload();
    /// Deserialize from XML data
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous);
    /// Serialize to XML data
    virtual bool SerializeTo(std::vector<u8> &dst, const QString &serializationParameters) const;
    /// Return depended upon asset references
    virtual std::vector<AssetReference> FindReferences() const;
    /// Called whenever another asset this asset depends on is loaded.
    virtual void DependencyLoaded(AssetPtr dependee);
    /// Check if asset is loaded. Checks only XML data size
    bool IsLoaded() const;

private:
    /// Asset references have changed. (Re)request them and trigger appearance changed when all are loaded
    void AssetReferencesChanged();
    /// Parse from XML data. Return true if successful
    bool ReadAvatarAppearance(const QDomDocument& source);
    /// Read a bone modifier
    void ReadBoneModifierSet(const QDomElement& source);
    /// Read the weight value (0 - 1) for an already existing bone modifier
    void ReadBoneModifierParameter(const QDomElement& source);
    /// Read a morph modifier
    void ReadMorphModifier(const QDomElement& source);
    /// Read a master modifier
    void ReadMasterModifier(const QDomElement& source);
    /// Read animations
    void ReadAnimationDefinitions(const QDomDocument& source);
    /// Read one animation definition
    void ReadAnimationDefinition(const QDomElement& elem);
    /// Read attachment mesh definition
    void ReadAttachment(const QDomElement& elem);
    /// Recalculate master modifier values
    void CalculateMasterModifiers();
    /// Write to XML data
    void WriteAvatarAppearance(QDomDocument& dest) const;
    /// Write bone modifier to XML
    void WriteBoneModifierSet(QDomDocument& dest, QDomElement& dest_elem, const BoneModifierSet& bones) const;
    /// Write bone to XML
    QDomElement WriteBone(QDomDocument& dest, const BoneModifier& bone) const;
    /// Write morph modifier to XML
    QDomElement WriteMorphModifier(QDomDocument& dest, const MorphModifier& morph) const;
    /// Write master modifier to XML
    QDomElement WriteMasterModifier(QDomDocument& dest, const MasterModifier& morph) const;
    /// Write animation definition to XML
    QDomElement WriteAnimationDefinition(QDomDocument& dest, const AnimationDefinition& anim) const;
    /// Write attachment to XML
    QDomElement WriteAttachment(QDomDocument& dest, const AvatarAttachment& attachment, const QString& mesh) const;
    /// Find modifier by name and type
    AppearanceModifier* FindModifier(const QString & name, AppearanceModifier::ModifierType type);
    /// Add reference to a reference vector if not empty
    void AddReference(std::vector<AssetReference>& refs, const QString& ref) const;
    
public slots:
    /// Set a master modifier value. Triggers DynamicAppearanceChanged
    void SetMasterModifierValue(const QString& name, float value);
    /// Set a morph or bone modifier value. It will be brought under manual control, ie. master modifiers no longer have an effect. Triggers DynamicAppearanceChanged
    void SetModifierValue(const QString& name, float value);
    /// Change a material ref
    void SetMaterial(uint index, const QString& ref);
    /// Remove an attachment
    /** @param index The index of the attachment in attachments_ to be removed. */
    void RemoveAttachment(uint index);
    /// Removes all attachments of given category.
    /** @param category The name of the category of attachments to be removed. */
    void RemoveAttachmentsByCategory(QString category);
    /// Add an attachment
    /** @param data The attachment to be added to the avatar.*/
    void AddAttachment(AssetPtr assetPtr);
    /// Return whether a property exists
    bool HasProperty(const QString &name) const;
    /// Return property value, or empty if does not exist
    const QString& GetProperty(const QString& value);

signals:
    /// Mesh, skeleton, mesh materials or attachment meshes have changed. The entity using this avatar desc should refresh its appearance completely
    void AppearanceChanged();
    /// Dynamic properties (morphs, bone modifiers) have changed. The entity using this avatar desc should refresh those parts of the appearance
    void DynamicAppearanceChanged();

public:
    /// Stores the avatar appearance XML file as raw .xml data. Note: if parameters such as bonemodifiers change "live", this won't be updated.
    QString avatarAppearanceXML_;
    
    /// Avatar mesh asset reference
    QString mesh_;
    /// Avatar skeleton asset reference
    QString skeleton_;
    /// Avatar material asset references
    std::vector<QString> materials_;
    
    /// Animation defines
    std::vector<AnimationDefinition> animations_;
    /// Attachments
    std::vector<AvatarAttachment> attachments_;
    /// Bone modifiers
    std::vector<BoneModifierSet> boneModifiers_;
    /// Morph modifiers
    std::vector<MorphModifier> morphModifiers_;
    /// Master modifiers, which may drive either bones or morphs
    std::vector<MasterModifier> masterModifiers_; 
    /// Miscellaneous properties (freedata)
    QMap<QString, QString> properties_;
};

typedef shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

