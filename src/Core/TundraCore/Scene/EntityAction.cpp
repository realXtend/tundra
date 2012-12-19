/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EntityAction.cpp
 *  @brief  Represent an executable command on an Entity.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EntityAction.h"
#include "MemoryLeakCheck.h"

void EntityAction::Trigger(const QString &param1, const QString &param2, const QString &param3, const QStringList &params)
{
    emit Triggered(param1, param2, param3, params);
}

EntityAction::EntityAction(const QString &name_)
:name(name_)
{
}
