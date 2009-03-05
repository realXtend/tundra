// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_h
#define incl_OgreRenderingModule_h

#include "ModuleInterface.h"
#include "Renderer.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    //! interface for modules
    class OgreRenderingModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

    private:
        OgreRenderer::RendererPtr renderer_;
    };
}

#endif
