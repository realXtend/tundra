/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_Billboard.h
    @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity. */

#pragma once

#include "IComponent.h"
#include "Math/float3.h"
#include "AssetReference.h"
#include "AssetRefListener.h"
#include "OgreModuleFwd.h"
#include "OgreModuleApi.h"

/// Shows a billboard (3D sprite) that is attached to an entity.
/** <table class="header">
    <tr>
    <td>
    <h2>Billboard</h2>
    Shows a billboard (3D sprite) that is attached to an entity.

    Registered by OgrRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li>AssetReference: materialRef
    <div> @copydoc materialRef </div>
    <li>float3: position
    <div> @copydoc position </div>
    <li>float: width
    <div> @copydoc width </div>
    <li>float: height
    <div> @copydoc height </div>
    <li>float: rotation
    <div> @copydoc rotation </div>
    <li>bool: show
    <div> @copydoc show </div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"Show": @copydoc Show
    <li>"Hide: @copydoc Hide
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>...
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on components @ref EC_Placeable "Placeable".</b>
    </table> */
class OGRE_MODULE_API EC_Billboard : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Billboard", 2)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Billboard(Scene* scene);
    ~EC_Billboard();

    /// Material asset reference for the billboard
    Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
    
    /// Billboard position in relation to the placeable component
    Q_PROPERTY(float3 position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);
    
    /// Billboard width
    Q_PROPERTY(float width READ getwidth WRITE setwidth);
    DEFINE_QPROPERTY_ATTRIBUTE(float, width);
    
    /// Billboard height
    Q_PROPERTY(float height READ getheight WRITE setheight);
    DEFINE_QPROPERTY_ATTRIBUTE(float, height);
    
    /// Billboard rotation in degrees
    Q_PROPERTY(float rotation READ getrotation WRITE setrotation);
    DEFINE_QPROPERTY_ATTRIBUTE(float, rotation);
    
    /// Show flag
    Q_PROPERTY(bool show READ getshow WRITE setshow);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, show);

    /// Returns the Ogre Billboard.
    Ogre::Billboard *OgreBillboard() const { return billboard_; }

    /// Returns the Ogre BillboardSet.
    Ogre::BillboardSet *OgreBillboardSet() const { return billboardSet_; }

    // DEPRECATED
    Ogre::Billboard *GetBillBoard() const { return OgreBillboard(); } /**< @deprecated Use OgreBillBoard @todo Remove */
    Ogre::BillboardSet *GetBillBoardSet() const { return OgreBillboardSet(); } /**< @deprecated Use OgreBillBoardSet @todo Remove */

public slots:
    /// Shows the billboard. Does not affect the @c show attribute.
    void Show();
    
    /// Hides the billboard. Does not affect the @c show attribute.
    void Hide();

private slots:
    /// Component has been assigned to an entity
    void OnParentEntitySet();
    
    /// Check for placeable being added to the entity
    void CheckForPlaceable();
    
    /// Component removed; check if it was the placeable
    void OnComponentRemoved(IComponent* component, AttributeChange::Type change);
    
    /// Called when material asset has been downloaded.
    void OnMaterialAssetLoaded(AssetPtr material);
    
    /// Called when material asset failed to load
    void OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason);
    
private:
    /// Create billboardset & billboard
    void CreateBillboard();
    
    /// Update position & dimensions
    void UpdateBillboardProperties();
    
    /// Destroy billboardset
    void DestroyBillboard();
    
    /// Attach billboardset to the placeable's scene node
    void AttachBillboard();
    
    /// Detach billboardset from the placeable's scene node
    void DetachBillboard();

    void AttributesChanged();

    /// Ogre world ptr
    OgreWorldWeakPtr world_;
    
    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;
    
    /// Attached flag
    bool attached_;
    
    /// Placeable pointer
    shared_ptr<EC_Placeable> placeable_;
    
    /// Asset ref listener for the material
    AssetRefListenerPtr materialAsset_;
};
