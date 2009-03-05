// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_GeometrySystem_System_h
#define incl_GeometrySystem_System_h

#include "ModuleInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Geometry
{
    //! interface for modules
    class GeometrySystem : public Foundation::ModuleInterface_Impl
    {
    public:
        GeometrySystem();
        virtual ~GeometrySystem();

        virtual void load();
        virtual void unload();
        virtual void initialize(Foundation::Framework *framework);
        virtual void uninitialize(Foundation::Framework *framework);
    };
}

#endif
