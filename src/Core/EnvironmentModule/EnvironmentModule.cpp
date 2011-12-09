/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment plugin is be responsible of visual environment features like terrain, sky & water.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_WaterPlane.h"
#include "EC_Fog.h"
#include "EC_Sky.h"
#include "EC_EnvironmentLight.h"
#include "EC_Terrain.h"

#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "Scene.h"
#include "IComponentFactory.h"

#include "MemoryLeakCheck.h"

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Terrain>));
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_WaterPlane>));
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Fog>));
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Sky>));
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_EnvironmentLight>));

    // Create an asset type factory for Terrain assets. The terrain assets are handled as binary blobs - the EC_Terrain parses it when showing the asset.
    fw->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Terrain")));
}
}
