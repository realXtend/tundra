/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Movable.h
 *  @brief  Contains Entity Actions for moving entity with this component in scene.
 */

#ifndef incl_EC_Movable_EC_Movable_h
#define incl_EC_Movable_EC_Movable_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3d.h"

class Quaternion;

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

/** Contains Entity Actions for moving entity with this component in scene.
 */
class EC_Movable : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Movable);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_Movable();

    void SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream);

public slots:
    /*
        This component supports the following actions:
        -MoveForward
        -MoveBackward
        -MoveLeft
        -MoveRight
        -RotateLeft
        -RotateRight
    */
    void Move(const QString &direction);

    void Rotate(const QString &direction);
private:
    /** Constructor.
        @param module Declaring module.
     */
    explicit EC_Movable(Foundation::ModuleInterface *module);

    void SendMultipleObjectUpdatePacket(const Vector3df &deltaPos, const Quaternion &deltaOri);

    event_category_id_t frameworkCategory_;

    ProtocolUtilities::WorldStreamPtr worldStream_;

private slots:
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
};

#endif
