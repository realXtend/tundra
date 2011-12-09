/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   IModule.cpp
 *  @brief  Interface for module objects.
 *          See @ref ModuleArchitecture for details.
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Framework.h"
#include "IModule.h"
#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

IModule::IModule(const QString &name_) : name(name_), framework_(0)
{
}

IModule::~IModule()
{
}

Framework *IModule::GetFramework() const
{
    return framework_;
}
