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
#include "TerrainWeightEditor.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "Renderer.h"

#include "SceneManager.h"
#include "ModuleManager.h"
#include "CompositionHandler.h"
#include "EC_Name.h"
#include "EC_Terrain.h"

#include "MemoryLeakCheck.h"

namespace Environment
{
    EnvironmentModule::EnvironmentModule()
    :IModule("Environment"),
    terrainWeightEditor_(0)
    {
    }

    EnvironmentModule::~EnvironmentModule()
    {
    }

    void EnvironmentModule::Load()
    {
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_WaterPlane);
        DECLARE_MODULE_EC(EC_Fog);
        DECLARE_MODULE_EC(EC_Sky);
        DECLARE_MODULE_EC(EC_EnvironmentLight);

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
        SAFE_DELETE(terrainWeightEditor_);
    }

    void EnvironmentModule::Update(f64 frametime)
    {
        RESETPROFILER;
        PROFILE(EnvironmentModule_Update);
    }

    void EnvironmentModule::ShowTerrainWeightEditor()
    {
        if (framework_->IsHeadless())
            return;

        if (terrainWeightEditor_)
        {
            terrainWeightEditor_->show();
            return;
        }

        terrainWeightEditor_ = new TerrainWeightEditor(framework_);
        terrainWeightEditor_->setWindowFlags(Qt::Tool);
        terrainWeightEditor_->show();
    }

}

void SetProfiler(Profiler *profiler)
{
    ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    IModule *module = new Environment::EnvironmentModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
