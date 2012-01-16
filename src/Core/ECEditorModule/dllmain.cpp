// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "Framework.h"

#include "ECEditorModule.h"
#include "SceneStructureModule.h"

extern "C"
{

DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *ecModule = new ECEditorModule();
    fw->RegisterModule(ecModule);
    IModule *ssModule = new SceneStructureModule();
    fw->RegisterModule(ssModule);
}

} // ~extern "C"
