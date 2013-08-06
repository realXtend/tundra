// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "EnvironmentModuleApi.h"
#include "IComponent.h"
#include "CoreTypes.h"
#include "Color.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"
#include "AssetFwd.h"

/// Makes the entity a water plane.
/** <table class="header">
    <tr>
    <td>
    <h2>WaterPlane</h2>

    Water plane component creates a cubic water plane. Inside the water cube scene fog is overridden by underwater fog properties.
    Despite the cubic nature, water plane is visible for the outside viewer only as a plane.

    Registered by EnvironmentComponents plugin.

    <b>Attributes</b>:
    <ul>
    <li>int: xSize
    <div> @copydoc xSize </div>
    <li>int: ySize
    <div> @copydoc ySize </div>
    <li>int: depth
    <div> @copydoc depth </div>
    <li>float3: position
    <div> @copydoc position </div>
    <li>Quat: rotation
    <div> @copydoc rotation </div>
    <li>float: scaleUfactor
    <div> @copydoc scaleUfactor </div>
    <li>float: scaleVfactor
    <div> @copydoc </div>
    <li>int: xSegments
    <div> @copydoc xSegments </div>
    <li>int: ySegments
    <div> @copydoc ySegments </div>
    <li>AssetReference: materialRef
    <div> @copydoc materialRef </div>
    <li>Color: fogColor
    <div> @copydoc fogColor </div>
    <li>float : fogStartDistance
    <div> @copydoc </div>
    <li>float : fogEndDistance
    <div> @copydoc fogEndDistance </div>
    <li>enum: fogMode
    <div> @copydoc fogMode </div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"IsCameraInsideWaterCube": @copydoc IsCameraInsideWaterCube
    <li>"IsPointInsideWaterCube": @copydoc IsPointInsideWaterCube
    <li>"GetPointOnPlane": @copydoc GetPointOnPlane
    <li>"GetDistanceToWaterPlane": @copydoc GetDistanceToWaterPlane
    <li>"IsTopOrBelowWaterPlane": @copydoc IsTopOrBelowWaterPlane
    <li>"CreateWaterPlane": @copydoc CreateWaterPlane
    <li>"RemoveWaterPlane": @copydoc RemoveWaterPlane
    </ul>

    Does not emit any actions.

    <b>Can use @ref EC_Placeable "Placeable" component</b>. If entity has the position defined by the Placeable component then it also specifies the position
    in the world space where this water plane is by default is placed at. Note component does not need Placeable component.
    </table> */
class ENVIRONMENT_MODULE_API EC_WaterPlane : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_WaterPlane", 12)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_WaterPlane(Scene* scene);

    virtual ~EC_WaterPlane();

    /// Water plane size in x-axis.
    Q_PROPERTY(int xSize READ getxSize WRITE setxSize);
    DEFINE_QPROPERTY_ATTRIBUTE(int, xSize);

    /// Water plane size in y-axis.
    Q_PROPERTY(int ySize READ getySize WRITE setySize);
    DEFINE_QPROPERTY_ATTRIBUTE(int, ySize);

    /// Defines how much below the surface water fog color is used. Meaning this attribute defines how "deep" is our ocean/pond. 
    Q_PROPERTY(int depth READ getdepth WRITE setdepth);
    DEFINE_QPROPERTY_ATTRIBUTE(int, depth);

    /// Position of the water plane.
    /** Used if no EC_Placeable present in the same entity. */
    Q_PROPERTY(float3 position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);

    /// Orientation of the water plane.
    Q_PROPERTY(Quat rotation READ getrotation WRITE setrotation);
    DEFINE_QPROPERTY_ATTRIBUTE(Quat, rotation);

    /// U Scale, factor which defines how many times the texture should be repeated in the u direction.
    /** @note The current default value  is so small 0.002, so it does not show up correctly in EC editor. */
    Q_PROPERTY(float scaleUfactor READ getscaleUfactor WRITE setscaleUfactor);
    DEFINE_QPROPERTY_ATTRIBUTE(float, scaleUfactor);

    /// V Scale, factor which defines how many times the texture should be repeated in the v direction.
    /** @note The current default value is so small 0.002, so it does not show up correctly in EC editor. */    Q_PROPERTY(float scaleVfactor READ getscaleVfactor WRITE setscaleVfactor);
    DEFINE_QPROPERTY_ATTRIBUTE(float, scaleVfactor);

    /// The number of segments to the plane in the x direction.
    Q_PROPERTY(int xSegments READ getxSegments WRITE setxSegments);
    DEFINE_QPROPERTY_ATTRIBUTE(int, xSegments);

    /// The number of segments to the plane in the y direction.
    Q_PROPERTY(int ySegments READ getySegments WRITE setySegments);
    DEFINE_QPROPERTY_ATTRIBUTE(int, ySegments);

    /// @deprecated Use materialRef instead. @todo Remove!
    Q_PROPERTY(QString materialName READ getmaterialName WRITE setmaterialName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, materialName);

    /// The material used for the water plane.
    Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);

    /// Color of underwater fog.
    Q_PROPERTY(Color fogColor READ getfogColor WRITE setfogColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, fogColor);

    /// Underwater fog start distance (meters), @ref EC_Fog::Linear "Linear" mode only.
    Q_PROPERTY(float fogStartDistance READ getfogStartDistance WRITE setfogStartDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, fogStartDistance);

    /// Underwater fog end distance (meters), @ref EC_Fog::Linear "Linear" mode only.
    Q_PROPERTY(float fogEndDistance READ getfogEndDistance WRITE setfogEndDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, fogEndDistance);

    /// Underwater fog mode, defines how the fog density increases.
    /** @see EC_Fog::FogMode */
    Q_PROPERTY(int fogMode READ getfogMode WRITE setfogMode);
    DEFINE_QPROPERTY_ATTRIBUTE(int, fogMode);

    /// The density of the fog in @ref EC_Fog::Exponentially "Exponentially" or @ref EC_Fog::ExponentiallySquare "ExponentiallySquare" mode, as a value between 0 and 1. The default is 0.001.
    Q_PROPERTY(float fogExpDensity READ getfogExpDensity WRITE setfogExpDensity);
    DEFINE_QPROPERTY_ATTRIBUTE(float, fogExpDensity);

public slots:
    /// Returns true if camera is inside of water cube.
    bool IsCameraInsideWaterCube();

    /// Returns true if point is inside of water cube.
    /** @param point in world coordinate system. */
    bool IsPointInsideWaterCube(const float3& point) const;

    /// Returns the point on the water plane in world space that lies on top of the given world space coordinate.
    /** @param point The point in world space to get the corresponding map point (in world space) for. */
    float3 GetPointOnPlane(const float3 &point) const;

    /// Returns distance from plane (note, here is assumption that point is top/or below plane), distance in here is distance from water plane surface.
    /** @param point is point in world coordinate system. */
    float GetDistanceToWaterPlane(const float3& point) const;

    /// Returns true if given point is top or below water plane.
    /** @param point is in world coordinate system. */
    bool IsTopOrBelowWaterPlane(const float3& point) const;

    /// When called creates new water plane into world and tries to attach it.
    void CreateWaterPlane();

    /// When called removes water plane from world.
    void RemoveWaterPlane();

private slots:
    void OnMaterialAssetLoaded(const AssetPtr &mat);

    /// Called if parent entity has set.
    void Create();

    /// Called if component is removed from parent entity.
    void ComponentRemoved(IComponent* component, AttributeChange::Type type);

    /// Called if component is added to parent entity.
    void ComponentAdded(IComponent* component, AttributeChange::Type type);

    void Update();

private:
    /// Called If some of the attributes has been changed.
    void AttributesChanged();

    /// Attach a new entity to scene node that world scene owns.
    void AttachEntity();

    /// Detach entity from the scene node.
    void DetachEntity();

    /// Finds out that is EC_Placeable component connected to same entity where water plane component is placed. 
    /** @returns component pointer to EC_Placeable component. */
    ComponentPtr FindPlaceable() const;

    /// Changes water plane position.
    /** This function should be called only if the parent entity of this component has no EC_Placeable component.
        @note Uses attribute @p position to for water plane defining water plane position  */
    void SetPosition();

    /// Changes water plane rotation
    /** This function should be called only if the parent entity of this component has no EC_Placeable component.
        @note Uses attribute @p rotation to for water plane defining water plane rotation */
    void SetOrientation();

    void SetUnderwaterFog();
    void RestoreFog();
    void UpdateMaterial();

    OgreWorldWeakPtr world_;
    Ogre::Entity* entity_;
    Ogre::SceneNode* node_;

    bool attached_;
    bool attachedToRoot_;

    /// Used for caching whether or not the camera is inside this water plane.
    /// If it was last frame, but isn't anymore, the original scene fog is restored (if existent in the first place).
    bool cameraInsideWaterCube;

    AssetRefListenerPtr materialAsset;
    QString currentMaterial; ///< Ogre resource name for the currently used material.
};
