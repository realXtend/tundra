/**
  *  For conditions of distribution and use, see copyright notice in license.txt
  *  @file   EC_Sittable.h
  *  @brief  EC_Sittable calculates position and orienation needed for sitting
  *  @note   The entity must have EC_Placeable and EC_Mesh available in advance.
*/

#ifndef incl_EC_Sittable_EC_Sittable_h
#define incl_EC_Sittable_EC_Sittable_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "OgreModuleFwd.h"
#include "IModule.h"
#include "IAttribute.h"
#include "Vector3D.h"
#include "Entity.h"
#include "SceneAPI.h"

class EC_Sittable : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Sittable);

    //! Sit position
    Q_PROPERTY(Vector3df sitPosition READ getsitPosition WRITE setsitPosition);
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, sitPosition);

    //! Sit orientation
    Q_PROPERTY(Vector3df sitOrientation READ getsitOrientation WRITE setsitOrientation);
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, sitOrientation);

    //! Auto walk own avatar to the current sit position
    Q_PROPERTY(bool autoWalk READ getautoWalk WRITE setautoWalk);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, autoWalk);

    //! If true, someone is already sitting on this place
    Q_PROPERTY(bool occupied READ getoccupied WRITE setoccupied);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, occupied);

private:

    explicit EC_Sittable(IModule *module);

    boost::shared_ptr<EC_Placeable> placeable_;
    boost::shared_ptr<EC_Placeable> avatarPlaceable_;
    boost::shared_ptr<EC_Mesh> mesh_;
    bool canUpdate_;
    bool canCheck_;

    Scene::Entity* GetOwnAvatar();

public:
    ~EC_Sittable();
    bool IsSerializable() const { return true; }

private slots:
    void InitComponent();
    void HandleAttributeChange(IAttribute*, AttributeChange::Type);
    void EntityClicked(Scene::Entity*, Qt::MouseButton, RaycastResult*);
    void CalculateSitPosAndRot();
    void GoTowardsTarget();
    void Update(float);
    void CheckIfOccupied();
    void EnableChecking();
    void StopMovement();
};

#endif