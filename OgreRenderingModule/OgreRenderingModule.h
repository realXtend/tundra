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
    class REX_API OgreRenderingModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();

        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Renderer;
    private:
        OgreRenderer::RendererPtr renderer_;
        Foundation::Framework* framework_;
    };
}

#endif
