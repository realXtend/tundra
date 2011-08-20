/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Selected.cpp
 *  @brief  EC_Selected enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Selected.h"
#include "IModule.h"
#include "Entity.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Selected")

#include "MemoryLeakCheck.h"

EC_Selected::EC_Selected(IModule *module) :
    IComponent(module->GetFramework())
{
}

EC_Selected::~EC_Selected()
{
}

