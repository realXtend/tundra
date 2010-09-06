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

    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    void SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream);

public slots:
    /** ComponentInterface override.
        This component supports the following actions:
        -MoveForward
        -MoveBackward
        -MoveLeft
        -MoveRight
        -RotateLeft
        -RotateRight
    */
    void Exec(const QString &action, const QVector<QString> &params);

private:
    /** Constructor.
        @param module Declaring module.
     */
    explicit EC_Movable(Foundation::ModuleInterface *module);

    event_category_id_t frameworkCategory_;

    ProtocolUtilities::WorldStreamPtr worldStream_;
};

#endif
