// For conditions of distribution and use, see copyright notice in license.txt

#if SKYX_ENABLED
#include "EC_SkyX.h"
#endif
#if HYDRAX_ENABLED
#include "EC_Hydrax.h"
#endif
#include "Framework.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"
///\todo HydraxConfigFile support
//#include "AssetAPI.h"
//#include "HydraxConfigAsset.h"

extern "C"
{

DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
#if SKYX_ENABLED
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_SkyX>));
#endif
#if HYDRAX_ENABLED
    fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Hydrax>));
    ///\todo HydraxConfigFile support
//    fw->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<HydraxConfigAsset>("HydraxConfigFile")));
#endif
}

} // ~extern "C"
