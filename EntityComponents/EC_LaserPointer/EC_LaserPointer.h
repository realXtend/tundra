/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.h
 *  @brief  EC_LaserPointer adds laser pointer to entities.
 *  @note   The entity must have EC_Placeable and EC_InputMapper available in advance.
*/

#ifndef incl_EC_LaserPointer_EC_LaserPointer_h
#define incl_EC_LaserPointer_EC_LaserPointer_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "OgreModuleFwd.h"
#include "OgreString.h"
#include "OgreMaterialUtils.h"
#include "IAttribute.h"
#include "Vector3D.h"
#include "Color.h"

#include "InputFwd.h"

#include <QColor>

class EC_LaserPointer : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_LaserPointer);

    //! Laser start point (tied to EC_Placeable position attribute)
    Q_PROPERTY(Vector3df startPos READ getstartPos WRITE setstartPos)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, startPos);

    //! Laser end point 
    Q_PROPERTY(Vector3df endPos READ getendPos WRITE setendPos)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, endPos);

    //! Laser color
    Q_PROPERTY(Color color READ getcolor WRITE setcolor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);

    //! If laser drawing is enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    //! Track placeable and mouse, this is not synced over network
    Q_PROPERTY(bool tracking READ gettracking WRITE settracking)

public slots:
    //! Tracking getter/setter
    bool gettracking() { return tracking; }
    void settracking(bool t) { tracking = t; }

    //! Is painting enabled
    bool IsEnabled() { return enabled.Get(); }

    //! Creates laser object and laser (child) node
    void CreateLaser();

    //! Destroys the laser and laser node
    void DestroyLaser();

    //! Convenience function to enable laser pointer
    void Enable();

    //! Convenience function to disable laser pointer
    void Disable();

    //! Sets the start point of the laser line
    //! @param New starting position
    void SetStartPos(const Vector3df);

    //! Sets the end point of the laser line
    //! @param New ending position
    void SetEndPos(const Vector3df);

    //! Gets the current start position
    //! @returns current start position
    Vector3df GetStartPos() const;

    //! Gets the current end position
    //! @returns current end position
    Vector3df GetEndPos() const;

    //! Sets laser color
    //! @param new color : QColor
    void SetQColor(const QColor &);

    //! Sets laser color
    //! @param red - red amount
    //! @param green - green amount
    //! @param blue - blue amount
    //! @param alpha - alpha ammount
    void SetColor(int, int, int, int);

    //! Gets current color
    //! @returns current color : Color
    Color GetColor() const;

    //! Gets current color
    //! @returns current color : QColor
    QColor GetQColor() const;

private:
    //! Constructor
    explicit EC_LaserPointer(IModule *module);

    //! Laser object (3d line)
    Ogre::ManualObject* laserObject_;
    Ogre::MaterialPtr laserMaterial_;

    //! Laser id, same as this EC's owner id, used to differentiate laser object and node names from one another
    Ogre::String id_;

    //! Parent entity input context
    InputContextPtr input_;

    //! Update limiter so that we do not overload the server
    bool canUpdate_;

    //! Tracking boolean
    bool tracking;

    //! Update interval (default is 20ms)
    int updateInterval_;

    //! Renderer pointer
    OgreRenderer::RendererWeakPtr renderer_;

private slots:
    //! (If it is allowed) updates start and end points on mousemove
    void Update(MouseEvent *e);

    //! Handles start and end point changes
    void HandleAttributeChange(IAttribute *attribute, AttributeChange::Type change);

    //! Handle placeable changes
    void HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change);

    //! Enables update; used for update limiter
    void EnableUpdate();

    //! Disables updates for <updateInterval_> time
    void DisableUpdate();

    //! Updates color if it is changed
    void UpdateColor();

    bool IsMouseInsideWindow();

    bool IsItemUnderMouse();

public:
    //! Destructor
    ~EC_LaserPointer();
    virtual bool IsSerializable() const { return true; }
};

#endif