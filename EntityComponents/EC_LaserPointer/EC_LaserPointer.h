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
#include "MouseEvent.h"
#include "InputContext.h"
#include "IAttribute.h"
#include "Vector3D.h"

class EC_LaserPointer : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_LaserPointer);

    //! Laser start point (tied to EC_Placeable position attribute)
    Attribute<Vector3df> startPos_;

    //! Laser end point 
    Attribute<Vector3df> endPos_;

    //! Enable/Disable toggle
    Attribute<bool> enabled_;

private:
    //! Constructor
    explicit EC_LaserPointer(IModule *module);

    //! Laser object (3d line)
    Ogre::ManualObject* laserObject_;
    //! Laser id, same as this EC's owner id, used to differentiate laser object and node names from one another
    Ogre::String id_;
    //! Parent entity EC_Placeable pointer
    EC_Placeable *node_;
    //! Parent entity input context
    InputContext *input_;
    //! Update limiter so that we do not overload the server
    bool canUpdate_;
    //! Update interval (default is 20ms)
    int updateInterval_;

    //! Renderer pointer
    OgreRenderer::RendererWeakPtr renderer_;

private slots:
    //! (If it is allowed) updates start and end points on mousemove
    void Update(MouseEvent *e);
    //! Handles start and end point changes
    void HandleAttributeChange(IAttribute*, AttributeChange::Type);

public:
    //! Destructor
    ~EC_LaserPointer();
    virtual bool IsSerializable() const { return true; }

public slots:
    //! Creates laser object and laser (child) node
    void CreateLaser();
    //! Destroys the laser and laser node
    void DestroyLaser();
    //! Convenience function to enable laser pointer
    void Enable();
    //! Convenience function to disable laser pointer
    void Disable();
    //! Enables update; used for update limiter
    void EnableUpdate();
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
    //! Check function if laser is enabled
    //! @returns current enabled state
    bool IsEnabled();
};

#endif