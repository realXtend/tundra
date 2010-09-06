/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Movable.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "EC_Movable.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Movable")

EC_Movable::~EC_Movable()
{
}

void EC_Movable::Exec(const QString &action, const QVector<QString> &params)
{
    if (action == "MoveForward")
        LogDebug("MoveForward");
    else if (action == "MoveBackward")
        LogDebug("MoveBackward");
    else if (action == "MoveLeft")
        LogDebug("MoveLeft");
    else if (action == "MoveRight")
        LogDebug("MoveRight");
    else if (action == "RotateLeft")
        LogDebug("RotateLeft");
    else if (action == "RotateRight")
        LogDebug("RotateRight");
}

EC_Movable::EC_Movable(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework())
{
}

