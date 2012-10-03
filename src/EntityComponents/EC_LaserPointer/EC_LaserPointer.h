/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.h
 *  @brief  Adds laser pointer to entity.
 */

#pragma once

#include "IComponent.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Color.h"
#include "InputFwd.h"

#include <OgreMaterial.h>

/// Adds laser pointer to entity.
/** The laser is drawn from the parent entity's placeable position to the position hit by raycast made from the mouse position.
    @note The entity must have EC_Placeable available in advance. */
class EC_LaserPointer : public IComponent
{
    Q_OBJECT
    /// Track placeable and mouse, this is not synced over network
    Q_PROPERTY(bool tracking READ IsTracking WRITE SetTracking)
    COMPONENT_NAME("EC_LaserPointer", 40);

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

    /// Is laser drawing is enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    // Tracking getter/setter
    ///\todo This is hackish, remove this boolean and implement better solution.
    bool IsTracking() const { return tracking; }
    void SetTracking(bool t) { tracking = t; }

public slots:
    /// Returns whether or not the laser pointer is visible.
    /** Note that is not the same thing as the enabled attribute, as laser pointer can be enabled but hidden
        when e.g. mouse cursor is not on top of the application window. */
    bool IsVisible() const;

private:
    Ogre::ManualObject* laserObject_; ///< Laser object (3d line)
    Ogre::MaterialPtr laserMaterial_;
    std::string id_; ///< Unique Ogre resource ID.
    InputContextPtr input_; ///< Input context for tracking mouse events.
    bool canUpdate_; ///< Update limiter so that we do not overload the server
    int updateInterval_; ///< Update interval (default is 20ms)
    OgreWorldWeakPtr world_;
    bool tracking;

    /// Handles start and end point changes
    void AttributesChanged();

private slots:
    /// Creates laser object.
    void CreateLaser();

    /// Destroys the laser object.
    void DestroyLaser();

    /// (If it is allowed) updates start and end points on mousemove
    void Update(MouseEvent *e);

    /// Handle placeable changes
    void HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change);

    /// Enables update; used for update limiter
    void EnableUpdate();

    /// Disables updates for <updateInterval_> time
    void DisableUpdate();

    /// Updates color if it is changed
    void UpdateColor();

    bool IsMouseInsideWindow() const;

    bool IsItemUnderMouse() const;
};
