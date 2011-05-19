/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment module. Environment module is be responsible of visual environment features like terrain, sky & water.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EnvironmentModule.h"
#include "EC_WaterPlane.h"
#include "EC_Fog.h"
#include "EC_Sky.h"
#include "EC_EnvironmentLight.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "Renderer.h"

#include "SceneManager.h"

#include "CompositionHandler.h"
#include "EC_Name.h"
#include "EC_Terrain.h"
#include "IComponentFactory.h"
#include "MemoryLeakCheck.h"

namespace Environment
{
    EnvironmentModule::EnvironmentModule()
    :IModule("Environment")
    {
    }

    EnvironmentModule::~EnvironmentModule()
    {
    }

    void EnvironmentModule::Load()
    {
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Terrain>));
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_WaterPlane>));
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Fog>));
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Sky>));
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_EnvironmentLight>));

        /// Create an asset type factory for Terrain assets. The terrain assets are handled as binary blobs - the EC_Terrain parses it when showing the asset.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Terrain")));
    }

    void EnvironmentModule::Initialize()
    {
    }

    void EnvironmentModule::PostInitialize()
    {
    }

    void EnvironmentModule::Uninitialize()
    {
    }

    void EnvironmentModule::Update(f64 frametime)
    {
    }

}


extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new Environment::EnvironmentModule();
    fw->RegisterModule(module);
}
}
