// For conditions of distribution and use, see copyright notice in license.txt

#include "UpdateModuleMac.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "Application.h"

UpdateModule::UpdateModule() :
    IModule("UpdateModule"),
    updater_(0)
{
}

UpdateModule::~UpdateModule()
{
}

void UpdateModule::Initialize()
{
    CocoaInitializer initializer;
    updater_ = new SparkleAutoUpdater("http://adminotech.data.s3.amazonaws.com/clients/tundra2/appcast.xml");
}

void UpdateModule::RunUpdater(QString parameter)
{
    if (updater_)
        updater_->checkForUpdates();
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new UpdateModule();
        fw->RegisterModule(module);
    }
}
