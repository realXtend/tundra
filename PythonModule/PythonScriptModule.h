// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptingModule_h
#define incl_PythonScriptingModule_h

#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "Script.h"

namespace Foundation
{
    class Framework;
}

namespace PythonScript
{
    //! interface for modules
    class PythonScriptModule : public Foundation::ModuleInterface_Impl
    {
    public:
        PythonScriptModule();
        virtual ~PythonScriptModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();

    /* python interpreter? private:
        OgreRenderer::RendererPtr renderer_;*/
    };
}

#endif
