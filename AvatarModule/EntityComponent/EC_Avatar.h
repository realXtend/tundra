// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_EC_Avatar_h
#define incl_Avatar_EC_Avatar_h

#include "IComponent.h"
#include "IAsset.h"
#include "RexUUID.h"
#include "AvatarModuleApi.h"
#include "Declare_EC.h"
#include "AssetFwd.h"
#include "SceneFwd.h"

struct BoneModifier;
class AvatarDescAsset;
typedef boost::shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

//! Avatar component.
/**
<table class="header">
<tr>
<td>
<h2>Avatar</h2>
This component generates the required Mesh, Placeable, AnimationController and AvatarAppearance
components to an entity to display an avatar. Not to be used in OpenSim worlds, where avatar generation
is handled in a hardcoded manner instead.

Registered by Avatar::AvatarModule.

<b>Attributes</b>:
<ul>
<li>QString: appearanceRef
<div>Asset id for the avatar appearance file that will be used to generate the visible avatar.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
</ul>

<b>Reacts on the following actions:</b>
<ul>
</ul>

Does not emit any actions.

<b>Depends on the components Mesh, Placeable, AnimationController and AvatarAppearance</b>.
</table>
*/
class AV_MODULE_API EC_Avatar : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Avatar);

public:
    //! Asset id for the avatar appearance file that will be used to generate the visible avatar. Asset request is handled by the component.
    Q_PROPERTY(AssetReference appearanceRef READ getappearanceRef WRITE setappearanceRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, appearanceRef);

    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }

    //! Destructor
    virtual ~EC_Avatar();

public slots:
    //! Refresh appearance completely
    void SetupAppearance();
    //! Refresh dynamic parts of the appearance (morphs, bone modifiers)
    void SetupDynamicAppearance();
    //! Get a generic property from the avatar description, or empty string if not found
    QString GetAvatarProperty(const QString& name);
    //! Return the avatar description asset
    AvatarDescAsset* GetAvatarDesc() { return avatarAsset_.get(); }
    
private slots:
    //! Called when some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute *attribute);
    
    void OnAvatarAppearanceLoaded(AssetPtr asset);

private:
    //! constructor
    /*! \param module avatar module
     */
    EC_Avatar(IModule* module);

    //! Adjust avatar's height offset dynamically
    void AdjustHeightOffset();
    //! Rebuild mesh and set materials
    void SetupMeshAndMaterials();
    //! Set morphs to values in avatar desc asset
    void SetupMorphs();
    //! Set bone modifiers to values in avatar desc asset
    void SetupBoneModifiers();
    //! Rebuild attachment meshes
    void SetupAttachments();
    //! Lookup absolute asset reference
    QString LookupAsset(const QString& ref);

    //! The current avatar description asset
    AvatarDescAssetPtr avatarAsset_;
};

#endif
