// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneModule.h"
#include "EC_Geometry.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"

namespace Geometry
{
    SceneModule::SceneModule() : ModuleInterface_Impl(Foundation::Module::Type_Geometry)
    {
    }

    SceneModule::~SceneModule()
    {
    }

    // virtual
    void SceneModule::Load()
    {
        using namespace Geometry;
        DECLARE_MODULE_EC(EC_Geometry);

        LOG("Module " + Name() + " loaded.");
    }

    // virtual
    void SceneModule::Unload()
    {
        LOG("Module " + Name() + " unloaded.");
    }

    // virtual
    void SceneModule::Initialize(Foundation::Framework *framework)
    {
        LOG("Module " + Name() + " initialized.");
    }

    void SceneModule::Update()
    {
    }

    // virtual 
    void SceneModule::Uninitialize(Foundation::Framework *framework)
    {
        LOG("Module " + Name() + " uninitialized.");
    }
}

using namespace Geometry;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(SceneModule)
POCO_END_MANIFEST

