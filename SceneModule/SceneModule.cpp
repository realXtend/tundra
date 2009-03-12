// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneModule.h"
#include "Foundation.h"

namespace Scene
{
    SceneModule::SceneModule() : ModuleInterface_Impl(type_static_)
    {
    }

    SceneModule::~SceneModule()
    {
    }

    // virtual
    void SceneModule::Load()
    {
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
        scene_manager_ = Foundation::ScenePtr(new SceneManager(framework));
        framework->GetServiceManager()->RegisterService(Foundation::Service::ST_Scene, scene_manager_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    void SceneModule::Update()
    {
    }

    // virtual 
    void SceneModule::Uninitialize(Foundation::Framework *framework)
    {
        framework->GetServiceManager()->UnregisterService(scene_manager_.get());
        scene_manager_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

