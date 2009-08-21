// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "QtModule.h"

#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "RendererEvents.h"

#include <QGraphicsScene>

#include "InputEvents.h"
#include "InputModuleOIS.h"

namespace QtUI
{

namespace
{
    const std::string moduleName("QtModule");
}

QtModule::QtModule()
:ModuleInterfaceImpl(Foundation::Module::MT_Gui), controller_(0)
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
    // Sanity check

    if ( controller_ != 0)
    {
        delete controller_;
        controller_ = 0;
    }
    
    controller_ = new UIController;

    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    
    if ( renderer != 0)
        controller_->SetParentWindowSize(QSize(renderer->GetWindowWidth(), renderer->GetWindowHeight()));
    else
    {
        // We have a problem deal it somehow?
    }
    

   
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
    delete controller_;
    controller_ = 0;

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
        controller_->SetParentWindowSize(QSize(windowResized->width_, windowResized->height_));

        
    }

    ///\todo Implement event-based buffered keyboard input.

    return false;
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

        

        if (input->IsButtonDown(OIS::MB_Left) && !mouse_left_button_down_)
        {
            
            controller_->InjectMousePress(pos.x(), pos.y());
            mouse_left_button_down_ = true;
        }
        else if (!input->IsButtonDown(OIS::MB_Left) && mouse_left_button_down_)
        {
          
            controller_->InjectMouseRelease(pos.x(),pos.y());
            mouse_left_button_down_ = false;
        }
        else
        {
		 
            controller_->InjectMouseMove(pos.x(),pos.y());
        }	
		
        PROFILE(QtSceneRender);

        ///\todo Optimize to redraw only those rectangles that are dirty.
        
   
		controller_->Update();
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
