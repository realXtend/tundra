// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"

#include "ECEditorModule.h"
#include "SceneStructureModule.h"

extern "C"
{

__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *ecModule = new ECEditorModule();
    fw->RegisterModule(ecModule);
    IModule *ssModule = new SceneStructureModule();
    fw->RegisterModule(ssModule);
}

} // ~extern "C"
