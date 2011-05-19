/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.cpp
 *  @brief  Interface for module objects.
 *          See @ref ModuleArchitecture for details.
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "Framework.h"
#include "IModule.h"
#include "LoggingFunctions.h"

IModule::IModule(const std::string &name_)
:name(name_), 
framework_(0)
{
}

IModule::~IModule()
{
}

Framework *IModule::GetFramework() const
{
    return framework_;
}
