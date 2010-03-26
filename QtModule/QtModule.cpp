// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "ComponentRegistrarInterface.h"
#include "DebugOperatorNew.h"
#include "SceneEvents.h"
#include "SceneManager.h"

#include "EC_UICanvas.h"
#include "QtModule.h"
#include "UICanvasManager.h"
#include "UICanvasTestEdit.h"

#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "RendererEvents.h"

#include <QGraphicsScene>
#include <QDebug>
#include <QApplication>

#include "InputEvents.h"
#include "InputModuleOIS.h"

#include "MemoryLeakCheck.h"

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"

namespace QtUI
{

namespace
{
    const std::string moduleName("QtModule");
}

QtModule::QtModule()
:ModuleInterfaceImpl(Foundation::Module::MT_Gui), controller_(0), canvasManager_(0), uicanvastestedit_(0)
{
}

QtModule::~QtModule()
{
}

void QtModule::Load()
{
    DECLARE_MODULE_EC(EC_UICanvas);
}

void QtModule::Unload()
{}

void QtModule::PreInitialize()
{
}

void QtModule::Initialize()
{
    // Sanity check
    SAFE_DELETE(controller_);
    
    controller_ = new UIController;
	canvasManager_ = new UICanvasManager(framework_);

    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    
    if (!renderer)
        throw Exception("Error! Could not find renderer service!");

    controller_->SetParentWindowSize(QSize(renderer->GetWindowWidth(), renderer->GetWindowHeight()));
    
    //Initialize OISKeyCode map. 

    InitializeKeyCodes();
   
    mouse_left_button_down_ = false;
    
    uicanvastestedit_ = new UICanvasTestEdit(framework_);
}

void QtModule::PostInitialize()
{
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
    input_event_category_ = event_manager->QueryEventCategory("Input");
    renderer_event_category_ = event_manager->QueryEventCategory("Renderer");
    scene_event_category_ = event_manager->QueryEventCategory("Scene");
}

void QtModule::Uninitialize()
{
    SAFE_DELETE(controller_);
	SAFE_DELETE(canvasManager_);
	SAFE_DELETE(uicanvastestedit_);
}

bool QtModule::HandleEvent(event_category_id_t category_id,
    event_id_t event_id, 
    Foundation::EventDataInterface* data)
{
    ///\todo Currently OIS mouse is polled in Update(). Convert all input to come through here.
    /// Requires that we can track when a canvas needs to be redrawn due to visual changes on mouse hover.
    
    if ( category_id == input_event_category_ && event_id == Input::Events::RIGHT_MOUSECLICK_PRESSED || event_id == Input::Events::RIGHT_MOUSECLICK_RELEASED)
    {
 
        bool event_handled = false;
        
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
      
        if (!input.get())
            return false;

        Input::Events::Movement* mouse = checked_static_cast<Input::Events::Movement*>(data);
        QPointF pos = QPointF(mouse->x_.abs_, mouse->y_.abs_);
        QPoint mousePos = pos.toPoint();

        UICanvas *canvas = controller_->GetCanvasAt(mousePos.x(), mousePos.y());
        
        controller_->SetCurrentModifier(GetCurrentModifier(input));
        controller_->SetActiveMouseButton(Qt::RightButton);
        
        if ( event_id == Input::Events::RIGHT_MOUSECLICK_PRESSED )
        {
            controller_->InjectMousePress(pos.x(), pos.y(), canvas);
            
            if ( canvas )
                event_handled = true;
            else
            {
                // Generate mouse pressed for inworld canvas 
               boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
                <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
               
                Foundation::RaycastResult result = renderer->Raycast(pos.x(), pos.y());
              
                // Has a entity a EC_CANVAS 

                boost::shared_ptr<UICanvas> in_world_canvas = GetCanvas(result);
                if ( in_world_canvas.get() != 0)
                {
                    // Note here coordinate system looks quite strange, but there happends inside of InjectMousePress "counter" fix for coordinates. 
                
                    QPoint pos = in_world_canvas->GetPosition().toPoint();
                    QSize size = in_world_canvas->GetSize();
                    QPoint location = QPoint(size.width() * result.u_ + pos.x(), size.height() * result.v_ + pos.y());
                    controller_->InjectMousePress(pos.x(), pos.y(), in_world_canvas.get());
                    event_handled = true;
                }

            }

            if (controller_->IsKeyboardFocus() && input->GetState() != Input::State_Buffered)
              input->SetState(Input::State_Buffered);
            else if (!controller_->IsKeyboardFocus())
              input->SetState(Input::State_Unknown);
        }
        else 
        {
            if ( canvas != 0) 
            {
                controller_->InjectMouseRelease(pos.x(),pos.y(), canvas);
                event_handled = true;
            }
            else
            {
               // Generate mouse released event for inworld canvas 
               boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
                <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
               
                Foundation::RaycastResult result = renderer->Raycast(pos.x(), pos.y());
              
                // Has entity a EC_CANVAS

                boost::shared_ptr<UICanvas> in_world_canvas = GetCanvas(result);
                if ( in_world_canvas.get() != 0)
                {
                    // Note here coordinate system looks quite strange, but there happends inside of InjectMouseRelease "counter" fix for coordinates. 
                
                    QPoint pos = in_world_canvas->GetPosition().toPoint();
                    QSize size = in_world_canvas->GetSize();
                    QPoint location = QPoint(size.width() * result.u_ + pos.x(), size.height() * result.v_ + pos.y());
                    controller_->InjectMouseRelease(pos.x(), pos.y(), in_world_canvas.get());
                    event_handled = true;
                }
                else
                {
                    // Deactivation release!
                  
                    controller_->InjectMouseRelease(pos.x(),pos.y(), canvas);
                }
        
            }
        }

        return event_handled;
        
    }

    //todo After OISModule refaction there should come events LEFT_MOUSE_CLICK etc. 

    if (category_id == input_event_category_ && event_id == Input::Events::INWORLD_CLICK || event_id == Input::Events::INWORLD_CLICK_REL)
    {
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
      
        if (!input.get())
            return false;

        Input::Events::Movement* mouse = checked_static_cast<Input::Events::Movement*>(data);
        QPointF pos = QPointF(mouse->x_.abs_, mouse->y_.abs_);
        QPoint mousePos = pos.toPoint();

        bool event_handled = false;
    
        UICanvas *canvas = controller_->GetCanvasAt(mousePos.x(), mousePos.y());
    
        bool oisLMBDown = input->IsButtonDown(OIS::MB_Left);
        
        controller_->SetCurrentModifier(GetCurrentModifier(input));
        controller_->SetActiveMouseButton(Qt::LeftButton);
        
        if (oisLMBDown && !mouse_left_button_down_)
        {
            //if (controller_->GetCanvasAt(pos.x(), pos.y()))
            //    framework_->GetQApplication()->setActiveWindow(controller_->GetCanvasAt(pos.x(), pos.y()));
            
            controller_->InjectMousePress(pos.x(), pos.y(), canvas);
            
            if ( canvas )
                event_handled = true;
           
            // In world click, to a inworld canvas is handled through event : EVENT_ENTITY_GRAB (currently)
        
            mouse_left_button_down_ = true;
         
            if (controller_->IsKeyboardFocus() && input->GetState() != Input::State_Buffered)
              input->SetState(Input::State_Buffered);
            else if (!controller_->IsKeyboardFocus())
              input->SetState(Input::State_Unknown);
        }
        else if (!oisLMBDown && mouse_left_button_down_)
        {
            if ( canvas != 0) 
            {
                controller_->InjectMouseRelease(pos.x(),pos.y(), canvas);
                event_handled = true;
            }
            else
            {
               
               // Generate mouse release for inworld canvas 
               boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
                <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
               
                Foundation::RaycastResult result = renderer->Raycast(pos.x(), pos.y());             
              
                // Has a entity a EC_CANVAS ?
                boost::shared_ptr<UICanvas> in_world_canvas = GetCanvas(result);
                if ( in_world_canvas.get() != 0)
                {
                    // Note here coordinate system looks quite strange, but there happends inside of InjectMousePress "counter" fix for coordinates. 
                    QPoint pos = in_world_canvas->GetPosition().toPoint();
                    QSize size = in_world_canvas->GetSize();
                    QPoint location = QPoint(size.width() * result.u_ + pos.x(), size.height() * result.v_ + pos.y());
                    controller_->InjectMouseRelease(location.x(), location.y(), in_world_canvas.get()); 
                    event_handled = true;
                }
                else
                    controller_->InjectMouseRelease(pos.x(),pos.y(), canvas);
                
              
            }
           
            mouse_left_button_down_ = false;
              
        }
        

        return event_handled;

    } 
    else if (category_id == renderer_event_category_ && event_id == OgreRenderer::Events::WINDOW_RESIZED)
    {
        // To properly compute the relative overlay sizes and positions for canvases, we need to keep track of the 
        // absolute size of the render window. Update the render window size on each WINDOW_RESIZED message.

        OgreRenderer::Events::WindowResized *windowResized = checked_static_cast<OgreRenderer::Events::WindowResized *>(data);
        controller_->SetParentWindowSize(QSize(windowResized->width_, windowResized->height_));
    }
    else if (category_id == input_event_category_ && (event_id == Input::Events::BUFFERED_KEY_PRESSED ||event_id == Input::Events::BUFFERED_KEY_RELEASED))
    {
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();
        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
        
        if (input.get() == 0)
            return false;

        // Get the key event OIS sent and convert it to a Qt key event.

        Input::Events::BufferedKey *key = checked_static_cast<Input::Events::BufferedKey* >(data);

        Qt::Key value;
        if (converterMap_.contains(static_cast<int>(key->code_)))
            value = converterMap_[static_cast<int>(key->code_)];
        else
            value = Qt::Key_unknown;

        ///\todo Are we missing the handling of Ctrl,Alt,AltGr,Win,Shift modifiers on keys?
        controller_->SetCurrentModifier(GetCurrentModifier(input));
        //Qt::KeyboardModifier modifier = Qt::NoModifier;

        // Inject the key event to the controller. It will propagate the event to the currently active canvas.
        QString text(QChar(key->text_));
        if (event_id == Input::Events::BUFFERED_KEY_PRESSED)
            controller_->InjectKeyPressed(text, value);
        else
            controller_->InjectKeyReleased(text, value);

        if ( controller_->IsKeyboardFocus() )
        {
            // Some of our widget has keyboard focus, so we will supress event. 
            return true;
        }
    }
    else if (category_id == scene_event_category_ && event_id == Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED)
    {
        // If entity has changed geometry or materials, and it has EC_UICanvas, make sure the EC_UICanvas refreshes
        RefreshEC_UICanvas(data);
    }
    else if ( category_id == scene_event_category_ && event_id == Scene::Events::EVENT_ENTITY_GRAB)
    {    
        // Check that has corresponding entity a canvas.
        Scene::Events::RaycastEventData* raycast_data = dynamic_cast<Scene::Events::RaycastEventData*>(data);
        if (raycast_data != 0)
        {
            Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
            if ( scene.get() != 0)
            {
                // Perform object selection for the UICanvasTestEdit
                if (uicanvastestedit_)
                {
                    uicanvastestedit_->SetEntityId(raycast_data->localID);
                }
                Scene::EntityPtr entity = scene->GetEntity(raycast_data->localID);
                if ( entity.get() != 0)
                {
                    EC_UICanvas* ui_canvas = entity->GetComponent<EC_UICanvas>().get();
                    if (ui_canvas)
                    {
                        // Has canvas.
                        boost::shared_ptr<UICanvas> canvas = ui_canvas->GetCanvas();
                        if (canvas)
                        {
                            // Note here coordinate system looks quite strange, but there happends inside of InjectMousePress "counter" fix for coordinates. 
                            QPoint pos = canvas->GetPosition().toPoint();
                            QSize size = canvas->GetSize();
                            QPoint position = QPoint(size.width() * raycast_data->u + pos.x(), size.height() * raycast_data->v + pos.y());
                        
                            // Poll from OIS which modifiers are on.

                             boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
                                framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

                            boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();

                            controller_->SetCurrentModifier(GetCurrentModifier(input));
                            controller_->InjectMousePress(position.x(), position.y(), canvas.get());
                            mouse_left_button_down_ = true;
                            
                            // Forced redraw (needed?)
                            controller_->Redraw(canvas);

                            if (controller_->IsKeyboardFocus() && input->GetState() != Input::State_Buffered)
                                input->SetState(Input::State_Buffered);
                            else if (!controller_->IsKeyboardFocus())
                                input->SetState(Input::State_Unknown);

                            return true;
                        }
                    }
                }
            }

        }
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
        framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
        // Clear keyboard.
        input->SetState(Input::State_Unknown);

    }
    else if (category_id == input_event_category_ && event_id == Input::Events::SCROLL)
    {
        // Mouse scroll event. 
        
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
      
        if (!input.get())
            return false;

        Input::Events::SingleAxisMovement* axis_movement = checked_static_cast<Input::Events::SingleAxisMovement*>(data);

   
        // Poll from mouse where mouse is located. 
        const Input::Events::Movement &mouse = input->GetMouseMovement();

        QPointF pos = QPointF(mouse.x_.abs_, mouse.y_.abs_);
        QPoint mousePos = pos.toPoint();

        bool event_handled = false;
    
        UICanvas *canvas = controller_->GetCanvasAt(mousePos.x(), mousePos.y());
        controller_->SetCurrentModifier(GetCurrentModifier(input));
        
        // Magical Qt wheel event step size.
        int step = 8;
        int delta = (axis_movement->z_.rel_)/step;
        
        if ( canvas != 0)
        {
            controller_->InjectMouseScroll(pos.x(), pos.y(),delta, canvas);
            event_handled = true;
        }
        else
        {
           
           // Generate mouse scroll for inworld canvas 
           boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
            <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
           
           if (renderer.get() == 0)
               return false;

            Foundation::RaycastResult result = renderer->Raycast(pos.x(), pos.y());
          
            // Has a entity a EC_CANVAS ?
            boost::shared_ptr<UICanvas> in_world_canvas = GetCanvas(result);
            if ( in_world_canvas.get() != 0)
            {
                // Note here coordinate system looks quite strange, but there happends inside of InjectMousePress "counter" fix for coordinates. 
                QPoint pos = in_world_canvas->GetPosition().toPoint();
                QSize size = in_world_canvas->GetSize();
                QPoint location = QPoint(size.width() * result.u_ + pos.x(), size.height() * result.v_ + pos.y());
                controller_->InjectMouseScroll(location.x(), location.y(), delta, in_world_canvas.get()); 
                event_handled = true;
            }
          
        }
        
        return event_handled;
    }

    return false;
}

boost::weak_ptr<UICanvas> QtModule::CreateCanvas(UICanvas::DisplayMode mode)
{
    boost::shared_ptr<UICanvas> canvas = controller_->CreateCanvas(mode).lock();   
    return canvas;
}

boost::weak_ptr<UICanvas> QtModule::GetCanvas(const QString& id)
{
    if ( controller_ != 0 )
       return controller_->GetCanvasByID(id);
    return boost::weak_ptr<UICanvas>();
}

Qt::KeyboardModifier QtModule::GetCurrentModifier(const boost::shared_ptr<Input::InputModuleOIS>& input) const
{
    Qt::KeyboardModifier modifier = Qt::NoModifier;

    if ( input->IsKeyDown(OIS::KC_LMENU) || input->IsKeyDown(OIS::KC_RMENU ) )
    {
        modifier = Qt::AltModifier;

    } else if ( input->IsKeyDown(OIS::KC_LSHIFT) || input->IsKeyDown(OIS::KC_RSHIFT) )
    {
        modifier = Qt::ShiftModifier;

    } else if ( input->IsKeyDown(OIS::KC_LCONTROL) || input->IsKeyDown(OIS::KC_RCONTROL ) )
    {
        modifier = Qt::ControlModifier;
    }
    
    return modifier;
}

void QtModule::AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle)
{
	if (canvasManager_ != 0)
        canvasManager_->AddCanvasToControlBar(canvas, buttonTitle);
}

void QtModule::AddCanvasToControlBar(const QString& id, const QString &buttonTitle)
{
	if (canvasManager_ != 0)
		canvasManager_->AddCanvasToControlBar(controller_->GetCanvasByID(id).lock(), buttonTitle);
}

bool QtModule::RemoveCanvasFromControlBar(const QString& id)
{
	if (canvasManager_ != 0)
		return canvasManager_->RemoveCanvasFromControlBar(id);
	return false;
}

void QtModule::Update(f64 frametime)
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    {
        PROFILE(QtModule_Update);

        // Poll mouse move input from OIS. 
    
        ///\todo Remove this in favor of events.
        
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
            framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();
      
        if (!input.get())
            return;
        
        const Input::Events::Movement &mouse = input->GetMouseMovement();
        QPointF pos = QPointF(mouse.x_.abs_, mouse.y_.abs_);
        QPoint change =  pos.toPoint() - lastPos_; 

        if (change.manhattanLength() >= 1)
        {
            bool handled = false;
            UICanvas* canvas = controller_->GetCanvasAt(pos.x(), pos.y());

            if ( canvas != 0 && !canvas->IsHidden())
            {
                controller_->InjectMouseMove(pos.x(),pos.y(), change.x(), change.y(), canvas);        
                handled = true;
            }
            else 
            {
                //Do raycast, generate mouse move to result of entity if it has a EC_CANVAS. 
                Foundation::RaycastResult result = renderer->Raycast(pos.x(), pos.y());
                boost::shared_ptr<UICanvas> in_world_canvas = GetCanvas(result);
                if ( in_world_canvas.get() != 0)
                {
                     // Note here coordinate system looks quite strange, but there happends inside of InjectMousePress "counter" fix for coordinates. 
                    QPoint pos = in_world_canvas->GetPosition().toPoint();
                    QSize size = in_world_canvas->GetSize();
                    QPoint location = QPoint(size.width() * result.u_ + pos.x(), size.height() * result.v_ + pos.y());
                    change = location - lastLocation_;
                    controller_->InjectMouseMove(location.x(), location.y(), change.x(), change.y(), in_world_canvas.get());  
                    lastLocation_ = location;
                    handled = true;
                }
                
            }
            
            if ( !handled )
            {
                // Check that was this a mouse move event?
                controller_->InjectMouseMove(pos.x(),pos.y(), change.x(), change.y(), 0);
            }
            lastPos_ = pos.toPoint();
        }

        ///\todo Optimize to redraw only those rectangles that are dirty.
        controller_->Update();
        
        PROFILE(QtSceneRender);
    }
    
    RESETPROFILER;
}

const std::string &QtModule::NameStatic()
{
    return moduleName;
}


void QtModule::InitializeKeyCodes()
{
    /* OIS gives us hardware keyboard scan codes, but Qt has its own code system where characters are 
        represented either in ascii, or if there's no equivalent, using a custom ID. Currently the
        only way we know of how to handle this is to perform a manual mapping between these two.

        The following keys haven't been mapped to any OIS input (see http://doc.trolltech.com/4.5/qt.html#Key-enum):  
        Qt::Key_AltGr, 
        Qt::Key_F16 - Qt::Key_F35 
        All keys between: Qt::Key_Super_L - Qt::Key_Direction_R except Qt::Key_Menu
        Qt::Key_Any, 
        Qt::Key_Exclam, 
        Qt::Key_QuoteDbl, 
        Qt::Key_NumberSign, 
        Qt::Key_Dollar, 
        Qt::Key_Percent, 
        Qt::Key_Ampersand, 
        Qt::Key_ParenLeft, 
        Qt::Key_ParenRight
        Qt::Key_Less 
        Qt::Key_Greater 
        Qt::Key_Question
    */

    converterMap_.insert(OIS::KC_ESCAPE, Qt::Key_Escape);
    converterMap_.insert(OIS::KC_TAB, Qt::Key_Tab);
    converterMap_.insert(OIS::KC_BACK, Qt::Key_Backspace);
    converterMap_.insert(OIS::KC_RETURN, Qt::Key_Return);
    converterMap_.insert(OIS::KC_INSERT, Qt::Key_Insert);
    converterMap_.insert(OIS::KC_DELETE, Qt::Key_Delete);
    converterMap_.insert(OIS::KC_PAUSE, Qt::Key_Pause);
    converterMap_.insert(OIS::KC_SYSRQ, Qt::Key_SysReq);
    converterMap_.insert(OIS::KC_HOME, Qt::Key_Home);
    converterMap_.insert(OIS::KC_END, Qt::Key_End);
    converterMap_.insert(OIS::KC_LEFT, Qt::Key_Left);
    converterMap_.insert(OIS::KC_UP, Qt::Key_Up);
    converterMap_.insert(OIS::KC_RIGHT, Qt::Key_Right);
    converterMap_.insert(OIS::KC_DOWN, Qt::Key_Down);
    converterMap_.insert(OIS::KC_PGUP, Qt::Key_PageUp);
    converterMap_.insert(OIS::KC_PGDOWN, Qt::Key_PageDown);
    converterMap_.insert(OIS::KC_LSHIFT, Qt::Key_Shift);
    converterMap_.insert(OIS::KC_RSHIFT, Qt::Key_Shift);
    converterMap_.insert(OIS::KC_LCONTROL, Qt::Key_Control);
    converterMap_.insert(OIS::KC_RCONTROL, Qt::Key_Control);
    converterMap_.insert(OIS::KC_LWIN, Qt::Key_Meta);
    converterMap_.insert(OIS::KC_LMENU, Qt::Key_Alt);
    converterMap_.insert(OIS::KC_CAPITAL, Qt::Key_CapsLock);
    converterMap_.insert(OIS::KC_NUMLOCK, Qt::Key_NumLock);
    converterMap_.insert(OIS::KC_SCROLL, Qt::Key_ScrollLock);
    converterMap_.insert(OIS::KC_F1, Qt::Key_F1);
    converterMap_.insert(OIS::KC_F2, Qt::Key_F2);
    converterMap_.insert(OIS::KC_F3, Qt::Key_F3);
    converterMap_.insert(OIS::KC_F4, Qt::Key_F4);
    converterMap_.insert(OIS::KC_F5, Qt::Key_F5);
    converterMap_.insert(OIS::KC_F6, Qt::Key_F6);
    converterMap_.insert(OIS::KC_F7, Qt::Key_F7);
    converterMap_.insert(OIS::KC_F8, Qt::Key_F8);
    converterMap_.insert(OIS::KC_F9, Qt::Key_F9);
    converterMap_.insert(OIS::KC_F10, Qt::Key_F10);
    converterMap_.insert(OIS::KC_F11, Qt::Key_F11);
    converterMap_.insert(OIS::KC_F12, Qt::Key_F12);
    converterMap_.insert(OIS::KC_F13, Qt::Key_F13);
    converterMap_.insert(OIS::KC_F14, Qt::Key_F14);
    converterMap_.insert(OIS::KC_F15, Qt::Key_F15);
    converterMap_.insert(OIS::KC_APPS, Qt::Key_Menu);
    converterMap_.insert(OIS::KC_SPACE, Qt::Key_Space);
    converterMap_.insert(OIS::KC_APOSTROPHE, Qt::Key_Apostrophe);
    converterMap_.insert(OIS::KC_MULTIPLY, Qt::Key_Asterisk);
    converterMap_.insert(OIS::KC_ADD, Qt::Key_Plus);
    converterMap_.insert(OIS::KC_COMMA, Qt::Key_Comma);
    converterMap_.insert(OIS::KC_MINUS, Qt::Key_Minus);
    converterMap_.insert(OIS::KC_PERIOD, Qt::Key_Period);
    converterMap_.insert(OIS::KC_SLASH, Qt::Key_Slash);
    converterMap_.insert(OIS::KC_0, Qt::Key_0);
    converterMap_.insert(OIS::KC_1, Qt::Key_1);
    converterMap_.insert(OIS::KC_2, Qt::Key_2);
    converterMap_.insert(OIS::KC_3, Qt::Key_3); 
    converterMap_.insert(OIS::KC_4, Qt::Key_4);
    converterMap_.insert(OIS::KC_5, Qt::Key_5);
    converterMap_.insert(OIS::KC_6, Qt::Key_6);
    converterMap_.insert(OIS::KC_7, Qt::Key_7);
    converterMap_.insert(OIS::KC_8, Qt::Key_8);
    converterMap_.insert(OIS::KC_9, Qt::Key_9);
    converterMap_.insert(OIS::KC_NUMPAD0, Qt::Key_0);
    converterMap_.insert(OIS::KC_NUMPAD1, Qt::Key_1);
    converterMap_.insert(OIS::KC_NUMPAD2, Qt::Key_2);
    converterMap_.insert(OIS::KC_NUMPAD3, Qt::Key_3); 
    converterMap_.insert(OIS::KC_NUMPAD4, Qt::Key_4);
    converterMap_.insert(OIS::KC_NUMPAD5, Qt::Key_5);
    converterMap_.insert(OIS::KC_NUMPAD6, Qt::Key_6);
    converterMap_.insert(OIS::KC_NUMPAD7, Qt::Key_7);
    converterMap_.insert(OIS::KC_NUMPAD8, Qt::Key_8);
    converterMap_.insert(OIS::KC_NUMPAD9, Qt::Key_9);   
    converterMap_.insert(OIS::KC_NUMPADCOMMA, Qt::Key_Comma);
    converterMap_.insert(OIS::KC_DECIMAL, Qt::Key_Comma);
    converterMap_.insert(OIS::KC_DIVIDE, Qt::Key_division);
    converterMap_.insert(OIS::KC_NUMPADEQUALS, Qt::Key_Equal);
    converterMap_.insert(OIS::KC_NUMPADENTER, Qt::Key_Enter);
    converterMap_.insert(OIS::KC_COLON, Qt::Key_Colon);
    converterMap_.insert(OIS::KC_SEMICOLON , Qt::Key_Semicolon);
    converterMap_.insert(OIS::KC_EQUALS, Qt::Key_Equal);
    converterMap_.insert(OIS::KC_AT , Qt::Key_At);
    converterMap_.insert(OIS::KC_A , Qt::Key_A);
    converterMap_.insert(OIS::KC_B , Qt::Key_B);
    converterMap_.insert(OIS::KC_C , Qt::Key_C);
    converterMap_.insert(OIS::KC_D , Qt::Key_D);
    converterMap_.insert(OIS::KC_E , Qt::Key_E);
    converterMap_.insert(OIS::KC_F , Qt::Key_F);
    converterMap_.insert(OIS::KC_G , Qt::Key_G);
    converterMap_.insert(OIS::KC_H , Qt::Key_H);
    converterMap_.insert(OIS::KC_I , Qt::Key_I);
    converterMap_.insert(OIS::KC_J , Qt::Key_J);
    converterMap_.insert(OIS::KC_K , Qt::Key_K);
    converterMap_.insert(OIS::KC_L , Qt::Key_L);
    converterMap_.insert(OIS::KC_M , Qt::Key_M);
    converterMap_.insert(OIS::KC_N , Qt::Key_N);
    converterMap_.insert(OIS::KC_O , Qt::Key_O);
    converterMap_.insert(OIS::KC_P , Qt::Key_P);
    converterMap_.insert(OIS::KC_Q , Qt::Key_Q);
    converterMap_.insert(OIS::KC_R , Qt::Key_R);
    converterMap_.insert(OIS::KC_S , Qt::Key_S);
    converterMap_.insert(OIS::KC_T , Qt::Key_T);
    converterMap_.insert(OIS::KC_U , Qt::Key_U);
    converterMap_.insert(OIS::KC_V , Qt::Key_V);
    converterMap_.insert(OIS::KC_W , Qt::Key_W);
    converterMap_.insert(OIS::KC_X , Qt::Key_X);
    converterMap_.insert(OIS::KC_Y, Qt::Key_Y);
    converterMap_.insert(OIS::KC_Z , Qt::Key_Z);
    converterMap_.insert(OIS::KC_LBRACKET , Qt::Key_BracketLeft);
}

void QtModule::RefreshEC_UICanvas(Foundation::EventDataInterface* data)
{
    Scene::Events::EntityEventData *entity_data = dynamic_cast<Scene::Events::EntityEventData*>(data);
    if (!entity_data)
        return;
        
    Scene::EntityPtr entity = entity_data->entity;
    if (!entity)
        return;
    
    EC_UICanvas* uicanvas = entity->GetComponent<EC_UICanvas>().get();
    if (uicanvas)
        uicanvas->Refresh();
}

Foundation::ComponentPtr QtModule::CreateEC_UICanvasToEntity(Scene::Entity* entity, boost::shared_ptr<UICanvas> canvas)
{
    if (!canvas || !entity)    
        return Foundation::ComponentPtr();

    Foundation::ComponentPtr uicanvasptr = entity->GetComponent(EC_UICanvas::NameStatic());
    if (!uicanvasptr)
    {
        uicanvasptr = framework_->GetComponentManager()->CreateComponent(EC_UICanvas::NameStatic());
        if (!uicanvasptr)
            return uicanvasptr;
        entity->AddEntityComponent(uicanvasptr);
    }
    
    EC_UICanvas* uicanvas = checked_static_cast<EC_UICanvas*>(uicanvasptr.get());    
    uicanvas->SetCanvas(canvas);
    uicanvas->SetEntity(entity);
    
    return uicanvasptr;    
}

boost::shared_ptr<UICanvas> QtModule::GetCanvas(const Foundation::RaycastResult& result)
{
    if ( result.entity_ != 0 )
    {
        EC_UICanvas* ui_canvas = result.entity_->GetComponent<EC_UICanvas>().get();
        if (ui_canvas)
        {
            // Has canvas.
            boost::shared_ptr<UICanvas> canvas = ui_canvas->GetCanvas();
            return canvas;
        }
    }

    return boost::shared_ptr<UICanvas>();
}

void QtModule::SetShowControlBar(bool show)
{
    canvasManager_->SetVisible(show);
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
