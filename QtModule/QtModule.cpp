// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "QtModule.h"

#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "RendererEvents.h"

#include <QGraphicsScene>
#include <QDebug>
#include "InputEvents.h"
#include "InputModuleOIS.h"

#include "MemoryLeakCheck.h"

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
    
    //Initialize OISKeyCode map. 
    InitializeKeyCodes();
   
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
    else if ( category_id = input_event_category_ && event_id == Input::Events::KEY_PRESSED)
    {
        boost::weak_ptr<Input::InputModuleOIS> inputWeak = 
        framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();

        boost::shared_ptr<Input::InputModuleOIS> input = inputWeak.lock();

        Input::Events::Key* key = checked_static_cast<Input::Events::Key* >(data);
        
        if ( input.get() == 0)
            return false;
        else
        {
            Qt::Key value;
            if ( converterMap_.contains(static_cast<int>(key->code_)) )
                value = converterMap_[static_cast<int>(key->code_)];
            else
                return false;

            Qt::KeyboardModifier modifier;

            switch(key->modifiers_)
            {
            case OIS::Keyboard::Alt:
                {
                    modifier = Qt::AltModifier;
                    break;
                }
            case OIS::Keyboard::Ctrl:
                {
                    modifier = Qt::ControlModifier;
                    break;
                }
            case OIS::Keyboard::Shift:
                {
                    modifier = Qt::ShiftModifier;
                    break;
                }
            default:
                {
                    modifier = Qt::NoModifier;
                    break;
                }
            }

            controller_->InjectKeyPressed(value, modifier);
        }
    }
    else if ( category_id = input_event_category_ && event_id == Input::Events::KEY_RELEASED)
    {

    }
    

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

        boost::shared_ptr<Input::InputModuleOIS> input_ = inputWeak.lock();
        //input_ = inputWeak.lock();
        if (!input_.get())
            return;
        
        const Input::Events::Movement &mouse = input_->GetMouseMovement();
        QPointF pos = QPointF(mouse.x_.abs_, mouse.y_.abs_);
        QPoint change = lastPos_ - pos.toPoint(); 

        if (input_->IsButtonDown(OIS::MB_Left) && !mouse_left_button_down_)
        {
           
            controller_->InjectMousePress(pos.x(), pos.y());
            mouse_left_button_down_ = true;
           
        }
        else if (!input_->IsButtonDown(OIS::MB_Left) && mouse_left_button_down_)
        {
           
            controller_->InjectMouseRelease(pos.x(),pos.y());
            mouse_left_button_down_ = false;
        
        }
        else if ( change.manhattanLength() >= 1 )
        {
		    controller_->InjectMouseMove(pos.x(),pos.y());
        }	
		lastPos_ = pos.toPoint();

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


void  QtModule::InitializeKeyCodes()
{
    // One of the most dirties hack. 
    // Next keyboard thingies does not work :  Qt::Key_AltGr, Qt::Key_F16 - Qt::Key_F35 all keys between: Qt::Key_Super_L - Qt::Key_Direction_R except Qt::Key_Menu
    // Qt::Key_Any, Qt::Key_Exclam, Qt::Key_QuoteDbl, Qt::Key_NumberSign, Qt::Key_Dollar, Qt::Key_Percent, Qt::Key_Ampersand, Qt::Key_ParenLeft, Qt::Key_ParenRight
    // Qt::Key_Less Qt::Key_Greater Qt::Key_Question



    converterMap_.insert(OIS::KC_ESCAPE, Qt::Key_Escape);
    converterMap_.insert(OIS::KC_TAB, Qt::Key_Tab);
    converterMap_.insert(OIS::KC_BACK, Qt::Key_Backspace);
    converterMap_.insert(OIS::KC_RETURN, Qt::Key_Return);
    converterMap_.insert(OIS::KC_NUMPADENTER, Qt::Key_Enter);
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
    converterMap_.insert(OIS::KC_LCONTROL, Qt::Key_Control);
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
