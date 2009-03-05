// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "GeometrySystem.h"
#include "EC_Geometry.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"

namespace Geometry
{
    GeometrySystem::GeometrySystem() : ModuleInterface_Impl(Foundation::Module::Type_Geometry)
    {
    }

    GeometrySystem::~GeometrySystem()
    {
    }

    // virtual
    void GeometrySystem::Load()
    {
        using namespace Geometry;
        DECLARE_MODULE_EC(EC_Geometry);

        LOG("System " + Name() + " loaded.");
    }

    // virtual
    void GeometrySystem::Unload()
    {
        LOG("System " + Name() + " unloaded.");
    }

    // virtual
    void GeometrySystem::Initialize(Foundation::Framework *framework)
    {
        LOG("System " + Name() + " initialized.");
    }

    // virtual 
    void GeometrySystem::Uninitialize(Foundation::Framework *framework)
    {
        LOG("System " + Name() + " uninitialized.");
    }
}

using namespace Geometry;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(GeometrySystem)
POCO_END_MANIFEST

