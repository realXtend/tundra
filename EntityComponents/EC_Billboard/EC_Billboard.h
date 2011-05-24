/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Billboard.h
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_Placeable component available in advance.
 */

#ifndef incl_EC_Billboard_EC_BillBoard_h
#define incl_EC_Billboard_EC_BillBoard_h

#include "IComponent.h"
#include "Vector3D.h"
#include "AssetReference.h"
#include "AssetRefListener.h"

class EC_Placeable;

namespace Ogre
{
    class BillboardSet;
    class Billboard;
}

/// EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
/**
<table class="header">
<tr>
<td>
<h2>Billboard</h2>
Billboard shows a billboard (3D sprite) that is attached to an entity.


Registered by TundraLogicModule.

<b>Attributes</b>:
<ul>
<li>AssetReference: materialRef
<div>Material used by the billboard.</div>
<li>Vector3df: position
<div>Position of billboard relative to the placeable component's position.</div> 
<li>float: width
<div>Billboard width.</div>
<li>float: height
<div>Billboard height.</div>
<li>float: rotation
<div>Billboard rotation in degrees.</div>
<li>bool: show
<div>Whether to show the billboard.</div>
<li>float: autoHideTime
<div>Time in seconds after which the billboard will hide itself automatically after showing. Negative values will not hide.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows billboard (and autohides after autoHideTime seconds, if it's positive). Does not affect the "show" -attribute.
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
    explicit EC_Billboard(Framework *fw);

    /// Destructor.
    ~EC_Billboard();

    /// Material asset reference for the billboard
    Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
    
    /// Billboard position in relation to the placeable component
    Q_PROPERTY(Vector3df position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, position);
    
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

    /// Time (in seconds) in which to autohide the billboard after showing. Negative values never hide
    Q_PROPERTY(float autoHideTime READ getautoHideTime WRITE setautoHideTime);
    DEFINE_QPROPERTY_ATTRIBUTE(float, autoHideTime);

public slots:

    /// Show billboard. If autoHideTime is positive, it will be autohidden after the time has passed
    void Show();
    
    /// Hide billboard
    void Hide();
    
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

#endif
