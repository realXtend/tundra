// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EnvironmentModule_EC_WaterPlane_h
#define incl_EnvironmentModule_EC_WaterPlane_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "Color.h"
#include "Quaternion.h"
#include "RexTypes.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"

#include <QString>
#include <OgreColourValue.h>

namespace Environment
{

/// Makes the entity a water plane.
/**
<table class="header">
<tr>
<td>
<h2>Water plane</h2>

Water plane component creates a cubic water plane. Inside the water cube scene fog is overridden by underwater fog properties.
Despite the cubic nature, water plane is visible for the outside viewer only as a plane.

<h3> Using component to synchronize ocean in Taiga </h3>

Currently (not in Tundra) EC_WaterPlane component can be used to synchronize Ocean in Taiga worlds. This can be done
so that user creates entity and sets entity EC_Name-component. If this component name is set as "WaterEnvironment" our current implementation
will create automatically a EC_WaterPlane-component on it. This component is now usable for every users and all changes on it will be transfered 
to all users. This synchronized water plane component can also edit through environment editor (in world tools).

Registered by Enviroment::EnvironmentModule.

<b>Attributes</b>:
<ul>
<li> int : xSize.
<div> Water plane size in x-axis. </div>
<li> int : ySize.
<div> Water plane size in y-axis. </div>
<li> int : depth.
<div> Depth value defines that how much below from surface water fog colour is used. Meaning this attribute defines how "deep" is our ocean/pond. </div>
<li> Vector3df : position.
<div> Defines position of water plane in world coordinate system. </div>
<li> Quaternion : rotation.
<div> Defines rotation of water plane in world coordinate system. </div>
<li> float : scaleUfactor.
<div> Water plane texture factor which defines how many times the texture should be repeated in the u direction. Note current default value 
 is so small 0.002, so it does not show up correctly in EC editor. </div>
<li> float : scaleVfactor.
<div> Water plane texture factor which defines how many times the texture should be repeated in the v direction. Note current default value 
 is so small 0.002, so it does not show up correctly in EC editor. </div>
<li> int : xSegments.
<div> The number of segments to the plane in the x direction.  </div>
<li> int : ySegments.
<div> The number of segments to the plane in the y direction.  </div>
<li> QString : materialName.
<div> Defines what material is used in creating plane. </div>
<li> AssetReference : materialRef
<div> Defines what material is used in creating plane. </div>
<li> Color : fogColor.
<div> Defines what is fog color when camera is inside of water cube which this plane defines. </div>
<li> float : fogStartDistance.
<div> Underwater fog start distance (meters) </div>
<li> float : fogEndDistance.
<div> Underwater fog end distance (meters) </div>
<li> enum :  fogMode.
<div> UnderWater fog mode, defines how Fog density increases.</div>
</ul>

Does not emit any actions.

<b>Can use component EC_Placeable</b>. If entity has the position defined by the EC_Placeable component then it also specifies the position
in the world space where this water plane is by default is placed at. Note component does not need Placeable component.
</table>
*/
class EC_WaterPlane : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_WaterPlane);

public:
    virtual ~EC_WaterPlane();

    virtual bool IsSerializable() const { return true; }

    /// Water plane x-size
    DEFINE_QPROPERTY_ATTRIBUTE(int, xSize);
    Q_PROPERTY(int xSize READ getxSize WRITE setxSize);

    /// Water plane y-size
    DEFINE_QPROPERTY_ATTRIBUTE(int, ySize);
    Q_PROPERTY(int ySize READ getySize WRITE setySize);

    /// Water plane "depth". This is used to define when we are below water and inside of water cube.
    DEFINE_QPROPERTY_ATTRIBUTE(int, depth);
    Q_PROPERTY(int depth READ getdepth WRITE setdepth);

    /// Water plane position (this is used if there is not EC_Placeable)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, position);
    Q_PROPERTY(Vector3df position READ getposition WRITE setposition);

    /// Water plane rotation
    DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, rotation);
    Q_PROPERTY(Quaternion rotation READ getrotation WRITE setrotation);

    ///U Scale, factor which defines how many times the texture should be repeated in the u direction.
    DEFINE_QPROPERTY_ATTRIBUTE(float, scaleUfactor);
    Q_PROPERTY(float scaleUfactor READ getscaleUfactor WRITE setscaleUfactor);

    ///V Scale, factor which defines how many times the texture should be repeated in the v direction.
    DEFINE_QPROPERTY_ATTRIBUTE(float, scaleVfactor);
    Q_PROPERTY(float scaleVfactor READ getscaleVfactor WRITE setscaleVfactor);

    /// The number of segments to the plane in the x direction 
    DEFINE_QPROPERTY_ATTRIBUTE(int, xSegments);
    Q_PROPERTY(int xSegments READ getxSegments WRITE setxSegments);

    /// The number of segments to the plane in the y direction 
    DEFINE_QPROPERTY_ATTRIBUTE(int, ySegments);
    Q_PROPERTY(int ySegments READ getySegments WRITE setySegments);

    /// Material name
    /// @todo Remove! Use only materialRef.
    DEFINE_QPROPERTY_ATTRIBUTE(QString, materialName);
    Q_PROPERTY(QString materialName READ getmaterialName WRITE setmaterialName);

    /// Material asset reference.
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
    Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);

    // Material texture, currently commented out, working feature.
    //DEFINE_QPROPERTY_ATTRIBUTE(QString, textureNameAttr);
    //Q_PROPERTY(QString textureNameAttr READ gettextureNameAttr WRITE settextureNameAttr);

    /// Underwater fog color
    DEFINE_QPROPERTY_ATTRIBUTE(Color, fogColor);
    Q_PROPERTY(Color fogColor READ getfogColor WRITE setfogColor);

    /// Underwater fog start distance (meters)
    DEFINE_QPROPERTY_ATTRIBUTE(float, fogStartDistance);
    Q_PROPERTY(float fogStartDistance READ getfogStartDistance WRITE setfogStartDistance);

    /// Underwater fog end distance (meters)
    DEFINE_QPROPERTY_ATTRIBUTE(float, fogEndDistance);
    Q_PROPERTY(float fogEndDistance READ getfogEndDistance WRITE setfogEndDistance);

    /// UnderWater fog mode, defines how Fog density increases.
    DEFINE_QPROPERTY_ATTRIBUTE(int, fogMode);
    Q_PROPERTY(int fogMode READ getfogMode WRITE setfogMode);

    /// Returns color value in Ogre format.
    Ogre::ColourValue GetFogColorAsOgreValue() const;

public slots: 
    /// Returns true if camera is inside of water cube.
    bool IsCameraInsideWaterCube();

    /// Returns true if point is inside of water cube.
    /// @param point in world coordinate system.
    bool IsPointInsideWaterCube(const Vector3df& point) const;

    /// Returns the point on the water plane in world space that lies on top of the given world space coordinate.
    /// @param point The point in world space to get the corresponding map point (in world space) for.
    Vector3df GetPointOnPlane(const Vector3df &point) const;

    /// Returns distance from plane (note, here is assumption that point is top/or below plane), distance in here is distance from water plane surface.
    /// @param point is point in world coordinate system.
    float GetDistanceToWaterPlane(const Vector3df& point) const;

    /// Returns true if given point is top or below water plane.
    /// @param point is in world coordinate system.
    bool IsTopOrBelowWaterPlane(const Vector3df& point) const;

    /// When called creates new water plane into world and tries to attach it.
    void CreateWaterPlane();

    /// When called removes water plane from world.
    void RemoveWaterPlane();

    //! Attach a new entity to scene node that world scene owns.
    void AttachEntity();

    //! Detach entity from the scene node.
    void DetachEntity();

    /// Called If some of the attributes has been changed.
    void AttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

    /// Called if parent entity has set.
    void SetParent();

    /// Called if component is removed from parent entity.
    void ComponentRemoved(IComponent* component, AttributeChange::Type type);

    /// Called if component is added to parent entity.
    void ComponentAdded(IComponent* component, AttributeChange::Type type);

private:
    /// Constructor.
    /** @param module Module where component belongs.
    */
    explicit EC_WaterPlane(IModule *module);

    /// Finds out that is EC_Placeable component connected to same entity where water plane component is placed. 
    /** @returns component pointer to EC_Placeable component.
    */
    ComponentPtr FindPlaceable() const;

    /// Helper function which is used to update water plane state.
    void ChangeWaterPlane(IAttribute* attribute);

    /// Changes water plane position.
    /** This function should be called only if the parent entity of this component has no EC_Placeable component.
        @note Uses attribute @p position to for water plane defining water plane position 
    */
    void SetPosition();

    /// Changes water plane rotation
    /** This function should be called only if the parent entity of this component has no EC_Placeable component.
        @note Uses attribute @p rotation to for water plane defining water plane rotation
    */
    void SetOrientation();

    OgreRenderer::RendererWeakPtr renderer_;
    Ogre::Entity* entity_;
    Ogre::SceneNode* node_;

    bool attached_;
    bool attachedToRoot_;
    int lastXsize_;
    int lastYsize_;
};

}

#endif
