// For conditions of distribution and use, see copyright notice in LICENSE

#include "ArchivePlugin.h"
#include "ArchiveBundleFactory.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "AssetAPI.h"

ArchivePlugin::ArchivePlugin() :
    IModule("ArchivePlugin")
{
}

ArchivePlugin::~ArchivePlugin()
{
}

void ArchivePlugin::Initialize()
{
    framework_->Asset()->RegisterAssetBundleTypeFactory(AssetBundleTypeFactoryPtr(new ArchiveBundleFactory()));
}

void ArchivePlugin::Uninitialize()
{
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw);
        IModule *module = new ArchivePlugin();
        fw->RegisterModule(module);
    }
}
