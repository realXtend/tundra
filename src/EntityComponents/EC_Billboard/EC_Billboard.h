/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EC_Billboard.h
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_Placeable component available in advance.
 */

#pragma once

#include "IComponent.h"
#include "Math/float3.h"
#include "AssetReference.h"
#include "AssetRefListener.h"
#include "OgreModuleFwd.h"

class EC_Placeable;

namespace Ogre
{
    class BillboardSet;
    class Billboard;
}

/// Shows a billboard (3D sprite) that is attached to an entity.
/**
<table class="header">
<tr>
<td>
<h2>Billboard</h2>
Shows a billboard (3D sprite) that is attached to an entity.


Registered by TundraLogicModule.

<b>Attributes</b>:
<ul>
<li>AssetReference: materialRef
<div>Material used by the billboard.</div>
<li>float3: position
<div>Position of billboard relative to the placeable component's position.</div> 
<li>float: width
<div>Billboard width.</div>
<li>float: height
<div>Billboard height.</div>
<li>float: rotation
<div>Billboard rotation in degrees.</div>
<li>bool: show
<div>Whether to show the billboard.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows billboard. Does not affect the "show" -attribute.
<li>"Hide: Hides the billboard. Does not affect the "show" -attribute.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on components Placeable</b>.  
</table>
*/
class EC_Billboard : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Billboard", 2)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Billboard(Scene* scene);

    /// Destructor.
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

public slots:
    /// Show billboard.
    void Show();
    
    /// Hide billboard.
    void Hide();

public:
    /// Returns the Ogre Billboard ptr.
    Ogre::Billboard *GetBillBoard() const { return billboard_; }
    
    /// Returns the Ogre BillboardSet ptr.
    Ogre::BillboardSet *GetBillBoardSet() const { return billboardSet_; }
    
private slots:
    /// Component has been assigned to an entity
    void OnParentEntitySet();
    
    /// Check for placeable being added to the entity
    void CheckForPlaceable();
    
    /// Component removed; check if it was the placeable
    void OnComponentRemoved(IComponent* component, AttributeChange::Type change);
    
    /// Called when some of the attributes has been changed
    void OnAttributeUpdated(IAttribute *attribute);
    
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

    /// Ogre world ptr
    OgreWorldWeakPtr world_;
    
    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;
    
    /// Attached flag
    bool attached_;
    
    /// Placeable pointer
    boost::shared_ptr<EC_Placeable> placeable_;
    
    /// Asset ref listener for the material
    AssetRefListenerPtr materialAsset_;
};

