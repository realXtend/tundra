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
    void GeometrySystem::load()
    {
        using namespace Geometry;
        DECLARE_MODULE_EC(EC_Geometry);

        LOG("System " + name() + " loaded.");
    }

    // virtual
    void GeometrySystem::unload()
    {
        LOG("System " + name() + " unloaded.");
    }

    // virtual
    void GeometrySystem::initialize(Foundation::Framework *framework)
    {
        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void GeometrySystem::uninitialize(Foundation::Framework *framework)
    {
        LOG("System " + name() + " uninitialized.");
    }
}

using namespace Geometry;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(GeometrySystem)
POCO_END_MANIFEST

