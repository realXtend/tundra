// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneModule.h"
#include "EC_Geometry.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"

namespace Scene
{
    SceneModule::SceneModule() : ModuleInterface_Impl(Foundation::Module::Type_Scene)
    {
    }

    SceneModule::~SceneModule()
    {
    }

    // virtual
    void SceneModule::Load()
    {
        using namespace Geometry;
        DECLARE_MODULE_EC(EC_Geometry);

        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void SceneModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void SceneModule::Initialize(Foundation::Framework *framework)
    {
        LogInfo("Module " + Name() + " initialized.");
    }

    void SceneModule::Update()
    {
    }

    // virtual 
    void SceneModule::Uninitialize(Foundation::Framework *framework)
    {
        LogInfo("Module " + Name() + " uninitialized.");
    }
}

