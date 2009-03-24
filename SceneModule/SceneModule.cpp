// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneModule.h"

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
    void SceneModule::Initialize()
    {
        scene_manager_ = Foundation::SceneManagerPtr(new SceneManager(this));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_SceneManager, scene_manager_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void SceneModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(scene_manager_.get());
        scene_manager_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

