/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EntityAction.cpp
 *  @brief  Represent an executable command on an Entity.
 *
 *          Components (or other instances) can register to these actions by using Entity::ConnectAction().
 *          Actions allow more complicated in-world logic to be built in slightly more data-driven fashion.
 *          Actions cannot be created directly, they're created by Entity::RegisterAction().
 */

#include "StableHeaders.h"
#include "EntityAction.h"

void EntityAction::Trigger(const QString &param1, const QString &param2, const QString &param3, const QStringList &params)
{
    emit Triggered(param1, param2, param3, params);
}
