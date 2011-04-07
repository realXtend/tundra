// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarModule_AvatarDescAsset_h
#define incl_AvatarModule_AvatarDescAsset_h

#include "AvatarDescHelpers.h"
#include "IAsset.h"

class QDomDocument;
class QDomElement;

//! Avatar appearance description asset
class AV_MODULE_API AvatarDescAsset : public IAsset
{
    Q_OBJECT;
public:
    AvatarDescAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~AvatarDescAsset();

    virtual void DoUnload();
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes);
    virtual bool SerializeTo(std::vector<u8> &dst, const QString &serializationParameters);
    virtual std::vector<AssetReference> FindReferences() const;

    bool IsLoaded() const;

    /// Stores the downloaded avatar appearance XML file as raw .xml data.
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
    //! Morph modifiers
    std::vector<MorphModifier> morphModifiers_;
    //! Master modifiers, which may drive either bones or morphs
    std::vector<MasterModifier> masterModifiers_; 
    //! Miscellaneous properties (freedata)
    QMap<QString, QString> properties_;
    
private:
    /// Parse from XML data. Return true if successful
    bool ReadAvatarAppearance(const QDomDocument& source);
    /// Read a bone modifier
    void ReadBoneModifierSet(const QDomElement& source);
    /// Read the weight value (0 - 1) for an already existing bone modifier
    void AvatarDescAsset::ReadBoneModifierParameter(const QDomElement& source);
    /// Read a morph modifier
    void ReadMorphModifier(const QDomElement& source);
    /// Read a master modifier
    void ReadMasterModifier(const QDomElement& source);
    /// Read animations
    void ReadAnimationDefinitions(const QDomDocument& source);
    /// Read one animation definition
    void ReadAnimationDefinition(const QDomElement& elem);
    /// Read attachment mesh definition
    void AvatarDescAsset::ReadAttachment(const QDomElement& elem);
    /// Recalculate master modifier values
    void CalculateMasterModifiers();
    /// Find modifier by name and type
    AppearanceModifier* FindModifier(const std::string& name, AppearanceModifier::ModifierType type);
    /// Add reference to a reference vector if not empty
    void AddReference(std::vector<AssetReference>& refs, const QString& ref) const;
    
public slots:
    /// Set a master modifier value. Triggers DynamicAppearanceChanged
    void SetMasterModifierValue(QString name, float value);
    /// Set a morph or bone modifier value. It will be brought under manual control, ie. master modifiers no longer have an effect. Triggers DynamicAppearanceChanged
    void SetModifierValue(QString name, float value);
    /// Return whether a property exists
    bool HasProperty(QString name) const;
    /// Return property value, or empty if does not exist
    const QString& GetProperty(QString value);
    
signals:
    /// Mesh, skeleton, mesh materials or attachment meshes have changed. The entity using this avatar desc should refresh its appearance completely
    void AppearanceChanged();
    /// Dynamic properties (morphs, bone modifiers) have changed. The entity using this avatar desc should refresh those parts of the appearance
    void DynamicAppearanceChanged();
};

typedef boost::shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

#endif
