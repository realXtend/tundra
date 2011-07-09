// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "IAsset.h"
#include "AvatarModuleApi.h"
#include "AssetFwd.h"
#include "SceneFwd.h"

struct BoneModifier;
class AvatarDescAsset;
typedef boost::shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

/// Avatar component.
/**
<table class="header">
<tr>
<td>
<h2>Avatar</h2>
This component generates the required Mesh, Placeable and AnimationController
components to an entity to display an avatar.

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

<b>Depends on the components Mesh, Placeable and AnimationController</b>.
</table>
*/
class AV_MODULE_API EC_Avatar : public IComponent
{
    Q_OBJECT

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Avatar(Scene* scene);

    /// Asset id for the avatar appearance file that will be used to generate the visible avatar. Asset request is handled by the component.
    Q_PROPERTY(AssetReference appearanceRef READ getappearanceRef WRITE setappearanceRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, appearanceRef);

    /// Destructor
    virtual ~EC_Avatar();

    COMPONENT_NAME("EC_Avatar", 1)
public slots:

    /// Refresh appearance completely
    void SetupAppearance();
    /// Refresh dynamic parts of the appearance (morphs, bone modifiers)
    void SetupDynamicAppearance();
    /// Return the avatar description asset, if set
    AvatarDescAssetPtr GetAvatarDesc();
    /// Get a generic property from the avatar description, or empty string if not found
    QString GetAvatarProperty(const QString& name);
    
private slots:
    /// Called when some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute *attribute);
    
    void OnAvatarAppearanceLoaded(AssetPtr asset);

private:
    /// Adjust avatar's height offset dynamically
    void AdjustHeightOffset();
    /// Rebuild mesh and set materials
    void SetupMeshAndMaterials();
    /// Set morphs to values in avatar desc asset
    void SetupMorphs();
    /// Set bone modifiers to values in avatar desc asset
    void SetupBoneModifiers();
    /// Rebuild attachment meshes
    void SetupAttachments();
    /// Lookup absolute asset reference
    QString LookupAsset(const QString& ref);

    /// Ref listener for the avatar asset
    AssetRefListenerPtr avatarAssetListener_;
    /// Last set avatar asset
    boost::weak_ptr<AvatarDescAsset> avatarAsset_;
};

