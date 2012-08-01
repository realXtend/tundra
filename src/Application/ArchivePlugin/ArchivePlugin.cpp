// For conditions of distribution and use, see copyright notice in LICENSE

#include "ArchiveBundleFactory.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "AssetAPI.h"

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *framework)
    {
        Framework::SetInstance(framework);
        framework->Asset()->RegisterAssetBundleTypeFactory(AssetBundleTypeFactoryPtr(new ArchiveBundleFactory()));
    }
}
