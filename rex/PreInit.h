// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexPreInit_h
#define incl_RexPreInit_h


#include "Foundation.h"
#include "SceneModule.h"

struct PreInit
{
    void operator()(Foundation::Framework *fw)
    {
        fw->GetModuleManager()->ExcludeModule(Foundation::Module::Type_Test);
        
        Foundation::ModuleInterface *module = new Scene::SceneModule;
        fw->GetModuleManager()->DeclareStaticModule(module);
    }
};

#endif
