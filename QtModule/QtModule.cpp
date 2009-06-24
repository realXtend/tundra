// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "QtModule.h"

#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "RendererEvents.h"

#include <QGraphicsScene>
#include <QPushButton>
#include <QtUiTools>

#include "OgreUIScene.h"
#include "OgreUIView.h"
#include "InputEvents.h"
#include "InputModuleOIS.h"

namespace QtUI
{

namespace
{
    const std::string moduleName("QtModule");
}

QtModule::QtModule()
:ModuleInterfaceImpl(moduleName)
{
}

QtModule::~QtModule()
{
}

void QtModule::Load()
{
}

void QtModule::Unload()
{
}

void QtModule::PreInitialize()
{
}

void QtModule::Initialize()
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (renderer)
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

    ///\todo Refactor the following into a separate function to allow creating several scenes, some of them
    /// might be 2D, others 3D. With the current rendering method, a canvas can be switched from 2D 
    /// to 3D without having to recreate any resources.

    // Create the single main 2D scene.
    main_scene_ = new QGraphicsScene();

    main_view_ = new OgreUIView();
    main_view_->setScene(main_scene_);
    main_view_->SetViewCanvasSize(128, 128);
    main_view_->SetParentWindowSize(renderer->GetWindowWidth(), renderer->GetWindowHeight());

    ///\todo We currently do a fixed canvas size as above. One method would be to do a fullscreen 
    /// render window sized canvas like below, but probably the most optimal method is to have the
    /// client generate the size (or automatically generate the canvas size according to the widget
    /// window size)
//    main_view_->SetViewCanvasSize(renderer->GetWindowWidth(), renderer->GetWindowHeight());

/*
    ///\todo This is just for feature testing. Can be removed when final.
    for(int x = 0; x < main_view_->width() - 200; x += 100)
        for(int y = 0; y < 100; y += 30)
        {
            QPushButton* pButton = new QPushButton("Button");
            main_scene_->addWidget(pButton);
            pButton->move(x, y);
        }

   QUiLoader loader;
   QFile file("./data/ui/login.ui");
   QWidget *login_widget_ = loader.load(&file); 
   QGraphicsProxyWidget *w = main_scene_->addWidget(login_widget_);
*/
    mouse_left_button_down_ = false;
}

void QtModule::PostInitialize()
{
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
    input_event_category_ = event_manager->QueryEventCategory("Input");
    renderer_event_category_ = event_manager->QueryEventCategory("Renderer");
}

void QtModule::Uninitialize()
{
}

bool QtModule::HandleEvent(Core::event_category_id_t category_id,
    Core::event_id_t event_id, 
    Foundation::EventDataInterface* data)
{
    ///\todo Currently OIS mouse is polled in Update(). Convert all input to come through here.
    /// Requires that we can track when a canvas needs to be redrawn due to visual changes on mouse hover.
/*  if (category_id == input_event_category_ && event_id == Input::Events::INWORLD_CLICK)
    {
        Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);
        main_view_->InjectMousePress(movement->x_.abs_, movement->y_.abs_);
    } */

    if (category_id == renderer_event_category_ && event_id == OgreRenderer::Events::WINDOW_RESIZED)
    {
        OgreRenderer::Events::WindowResized *windowResized = checked_static_cast<OgreRenderer::Events::WindowResized *>(data);
        main_view_->SetParentWindowSize(windowResized->width_, windowResized->height_);
//        main_view_->SetViewCanvasSize(windowResized->width_, windowResized->height_);
    }

    ///\todo Implement event-based buffered keyboard input.

    return false;
}

QGraphicsScene *QtModule::GetUIScene() const
{
    return main_scene_;
}

void QtModule::Update(Core::f64 frametime)
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    {
        PROFILE(QtModule_Update);

        // Poll mouse input from OIS. 
        ///\todo Remove this in favor of events.
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
        if (!input.get())
            return;
        
        const Input::Events::Movement &mouse = input->GetMouseMovement();
        QPointF pos = QPointF(mouse.x_.abs_, mouse.y_.abs_);

        // Translate the mouse coordinates from the main window coordinate system
        // to the Ogre overlay system.
        Ogre::OverlayContainer *container = main_view_->GetContainer();
        pos.rx() -= container->getLeft() * renderer->GetWindowWidth();
        pos.ry() -= container->getTop() * renderer->GetWindowHeight();

        if (input->IsButtonDown(OIS::MB_Left) && !mouse_left_button_down_)
        {
            main_view_->InjectMousePress(pos.x(), pos.y());
            mouse_left_button_down_ = true;
        }
        else if (!input->IsButtonDown(OIS::MB_Left) && mouse_left_button_down_)
        {
            main_view_->InjectMouseRelease(pos.x(), pos.y());
            mouse_left_button_down_ = false;
        }
        else
            main_view_->InjectMouseMove(pos.x(), pos.y());

        PROFILE(QtSceneRender);
        ///\todo We now redraw every frame. Optimize to redraw only those rectangles that are dirty.
        ///\ Optimize to redraw only when changed.
        main_view_->RenderSceneToOgreSurface();
    }
    RESETPROFILER;
}

const std::string &QtModule::NameStatic()
{
    return moduleName;
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace QtUI;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(QtModule)
POCO_END_MANIFEST
