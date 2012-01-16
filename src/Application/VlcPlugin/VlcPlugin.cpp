// For conditions of distribution and use, see copyright notice in LICENSE

#include "VlcPlugin.h"
#include "EC_MediaPlayer.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"

VlcPlugin::VlcPlugin() :
    IModule("VlcPlugin")
{
}

void VlcPlugin::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_MediaPlayer>));
}

void VlcPlugin::Uninitialize()
{
}

void VlcPlugin::Unload()
{
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new VlcPlugin();
        fw->RegisterModule(module);
    }
}
