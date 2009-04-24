// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_h
#define incl_OgreRenderingModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "OgreModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace Gtk
{
    class Window;
}

namespace OgreRenderer
{
    class OgreWidget;
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! A renderer module using Ogre
    class OGRE_MODULE_API OgreRenderingModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Unload();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(Core::f64 frametime);
        
        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! returns renderer
        RendererPtr GetRenderer() const { return renderer_; }
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        //! callback for console command
        Console::CommandResult ConsoleRequestTexture(const Core::StringVector &params);

        //! callback for console command
        Console::CommandResult ConsoleRequestMesh(const Core::StringVector &params);

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Renderer;

        void OnOgreGtkWindowClosed();

    private:
        //! renderer
        RendererPtr renderer_;

        //! widget for embedding the renderer
        OgreWidget* ogre_widget_;
        //! window for embedding the renderer
        Gtk::Window* ogre_window_;

        //! asset event category
        Core::event_category_id_t assetcategory_id_;

        //! resource event category
        Core::event_category_id_t resourcecategory_id_;
    };
}

#endif
