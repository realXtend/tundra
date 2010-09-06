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

/** Contains Entity Actions for moving entity with this component in scene.
 */
class EC_Movable : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Movable);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_Movable();

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
};

#endif
