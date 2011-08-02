/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.h
 *  @brief  EC_LaserPointer adds laser pointer to entities.
 *  @note   The entity must have EC_Placeable and EC_InputMapper available in advance.
*/

#pragma once

#include "IComponent.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Color.h"
#include "InputFwd.h"

#include <OgreMaterial.h>

class EC_LaserPointer : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_LaserPointer", 888);
    /// Track placeable and mouse, this is not synced over network
    Q_PROPERTY(bool tracking READ IsTracking WRITE SetTracking)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_LaserPointer(Scene* scene);
    ~EC_LaserPointer();

    /// Laser start point (tied to EC_Placeable position attribute)
    Q_PROPERTY(float3 startPos READ getstartPos WRITE setstartPos)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, startPos);

    /// Laser end point 
    Q_PROPERTY(float3 endPos READ getendPos WRITE setendPos)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, endPos);

    /// Laser color
    Q_PROPERTY(Color color READ getcolor WRITE setcolor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);

    /// If laser drawing is enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

public slots:
    bool IsTracking() const { return tracking; }
    void SetTracking(bool t) { tracking = t; }

    /// Creates laser object and laser (child) node
    void CreateLaser();

    /// Destroys the laser and laser node
    void DestroyLaser();

private:
    /// Laser object (3d line)
    Ogre::ManualObject* laserObject_;
    Ogre::MaterialPtr laserMaterial_;

    /// Laser id, same as this EC's owner id, used to differentiate laser object and node names from one another
    std::string id_;

    /// Parent entity input context
    InputContextPtr input_;

    /// Update limiter so that we do not overload the server
    bool canUpdate_;

    /// Tracking boolean
    bool tracking;

    /// Update interval (default is 20ms)
    int updateInterval_;

    OgreWorldWeakPtr world_;

private slots:
    /// (If it is allowed) updates start and end points on mousemove
    void Update(MouseEvent *e);

    /// Handles start and end point changes
    void HandleAttributeChange(IAttribute *attribute, AttributeChange::Type change);

    /// Handle placeable changes
    void HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change);

    /// Enables update; used for update limiter
    void EnableUpdate();

    /// Disables updates for <updateInterval_> time
    void DisableUpdate();

    /// Updates color if it is changed
    void UpdateColor();

    bool IsMouseInsideWindow();

    bool IsItemUnderMouse();
};
