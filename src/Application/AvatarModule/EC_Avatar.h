// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "AssetReference.h"
#include "AvatarModuleApi.h"
#include "AssetFwd.h"

struct BoneModifier;
class AvatarDescAsset;
typedef shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

/// Avatar component.
/** <table class="header">
    <tr>
    <td>
    <h2>Avatar</h2>

    @note This component no longer generates the required EC_Mesh, EC_Placeable and EC_AnimationController
    components to an entity to display an avatar.

    @todo Write better description!

    Registered by AvatarModule.

    <b>Attributes</b>:
    <ul>
    <li>AssetReference: appearanceRef
    <div> @copydoc appearanceRef</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    </ul>

    Does not emit any actions.

    <b>Depends on the components @ref EC_Mesh "Mesh" and @ref EC_Placeable "Placeable".</b>
    </table> */
class AV_MODULE_API EC_Avatar : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Avatar", 1)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Avatar(Scene* scene);
    virtual ~EC_Avatar();

    /// Asset id for the avatar appearance file that will be used to generate the visible avatar.
    Q_PROPERTY(AssetReference appearanceRef READ getappearanceRef WRITE setappearanceRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, appearanceRef);

public slots:
    /// Refresh appearance completely
    void SetupAppearance();
    /// Refresh dynamic parts of the appearance (morphs, bone modifiers)
    void SetupDynamicAppearance();
    /// Return the avatar description asset, if set
    AvatarDescAssetPtr AvatarDesc() const;
    /// Returns a generic property from the avatar description, or an empty string if not found
    QString AvatarProperty(const QString& name) const;

private slots:
    /// Avatar asset loaded.
    void OnAvatarAppearanceLoaded(AssetPtr asset);
    /// Avatar asset failed to load.
    void OnAvatarAppearanceFailed(IAssetTransfer* transfer, QString reason);

private:
    /// Called when some of the attributes has been changed.
    void AttributesChanged();
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
    weak_ptr<AvatarDescAsset> avatarAsset_;
};
