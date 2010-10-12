/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Movable.h
 *  @brief  Contains Entity Actions for moving entity with this component in scene.
 */

#ifndef incl_EC_Movable_EC_Movable_h
#define incl_EC_Movable_EC_Movable_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Vector3D.h"

class Quaternion;

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

/// Contains Entity Actions for moving entity with this component in scene.
/**
<table class="header">
<tr>
<td>
<h2>Movable</h2>
Contains Entity Actions for moving entity with this component in scene.

Registered by RexLogic::RexLogicModule.

<b>No Attributes</b>.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Move": Move(Forward), Move(Backward), Move(Left), Move(Right)
<li>"Rotate": Rotate(Left), Rotate(Right)
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable and OpenSimPrim</b>.
</table>
*/
class EC_Movable : public IComponent
{
    DECLARE_EC(EC_Movable);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_Movable();

    void SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream);

public slots:
    /// Moves the entity this component is attached to.
    /** @param direction Forward, Backward, Left or Right
    */
    void Move(const QString &direction);

    /// Rotates the entity this component is attached to.
    /** @param direction Left or Right
    */
    void Rotate(const QString &direction);

private:
    /** Constructor.
        @param module Declaring module.
     */
    explicit EC_Movable(IModule *module);

    void SendMultipleObjectUpdatePacket(const Vector3df &deltaPos, const Quaternion &deltaOri);

    ProtocolUtilities::WorldStreamPtr worldStream_;

private slots:
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
};

#endif
