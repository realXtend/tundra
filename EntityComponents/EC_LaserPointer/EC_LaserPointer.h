/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.h
 *  @brief  EC_LaserPointer adds laser pointer to entities.
 */

#pragma once

#include "IComponent.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Color.h"
#include "InputFwd.h"

#include <OgreMaterial.h>

/// EC_LaserPointer adds laser pointer to entities.
/** The laser is drawn from the parent entity's placeable position to the position hit by raycast made from the mouse position.
    @note The entity must have EC_Placeable available in advance. */
class EC_LaserPointer : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_LaserPointer", 888);

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
    /// Returns whether or not the laser pointer is visible.
    /** Note that is not the same thing as the enabled attribute, as laser pointer can be enabled but hidden
        when e.g. mouse cursor is not on top of the applicataion window.*/
    bool IsVisible() const;

private:
    /// Laser object (3d line)
    Ogre::ManualObject* laserObject_;
    Ogre::MaterialPtr laserMaterial_;

    /// Unique laser id, same as this EC's owner id, used to differentiate laser object and node names from one another
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
    /// Creates laser object and laser (child) node
    void CreateLaser();

    /// Destroys the laser and laser node
    void DestroyLaser();

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
