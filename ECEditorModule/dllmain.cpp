// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"
#include "ModuleManager.h"
#include "ECEditorModule.h"
#include "SceneStructureModule.h"

extern "C"
{

__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *ecModule = new ECEditorModule();
    fw->GetModuleManager()->DeclareStaticModule(ecModule);
    IModule *ssModule = new SceneStructureModule();
    fw->GetModuleManager()->DeclareStaticModule(ssModule);
}

} // ~extern "C"
