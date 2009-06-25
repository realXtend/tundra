// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreGtkWindowModule_h
#define incl_OgreGtkWindowModule_h

/*

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

    //! A Gtk window for Ogre renderer. Ogre will render to this window, if this module is present.
    //! Depends on GtkmmUI module.
    //! \todo Probably causes a memoryleak
    class OGRE_MODULE_API OgreGtkWindowModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        OgreGtkWindowModule();
        virtual ~OgreGtkWindowModule();

        virtual void Load();
        virtual void Unload();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        
        MODULE_LOGGING_FUNCTIONS;


        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_RendererWindow;

        void OnOgreGtkWindowClosed();

    private:
        //! widget for embedding the renderer
        OgreWidget* ogre_widget_;
        //! window for embedding the renderer
        Gtk::Window* ogre_window_;
    };
}

*/

#endif
