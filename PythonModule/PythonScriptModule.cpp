// For conditions of distribution and use, see copyright notice in license.txt

#include "PythonScriptModule.h"

//disabled now 'cause couldn't get working the own version for this module for some reason
//#include "StableHeaders.h"
#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

#include "DebugOperatorNew.h"

#include "ServiceManager.h"
#include "ComponentRegistrarInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "InputEvents.h"
#include "InputServiceInterface.h"
#include "RenderServiceInterface.h"
#include "PythonEngine.h" //is this needed here?

#include "RexLogicModule.h" //much of the api is here
#include "WorldStream.h" //for SendObjectAddPacket
#include "NetworkEvents.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "InputEvents.h" //handling input events
#include "InputServiceInterface.h" //for getting mouse info from the input service, prolly not used anymore ?
#include "RenderServiceInterface.h" //for getting rendering services, i.e. raycasts

#include "SceneManager.h"
#include "SceneEvents.h" //sending scene events after (placeable component) manipulation

#include "Avatar/Avatar.h"
#include "EntityComponent/EC_OpenSimPresence.h"
#include "EntityComponent/EC_NetworkPosition.h"
//for CreateEntity. to move to an own file (after the possible prob with having api code in diff files is solved)
//#include "../OgreRenderingModule/EC_OgreMesh.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreMovableTextOverlay.h"
#include "RexNetworkUtils.h"
#include "GenericMessageUtils.h"

#include "RexLogicModule.h" 
//#include "Login/LoginUI.h" //in rexlogic
#include "Environment/Primitive.h"
#include "CameraControllable.h"
//now done via logic cameracontrollable #include "Renderer.h" //for setting camera pitch
//#include "ogrecamera.h"
#include "../OgreRenderingModule/Renderer.h" //for the screenshot api XXX add the path to includes, don't do this.
#include "Avatar/AvatarControllable.h"

#include "Entity.h"
#include "RexPythonQt.h"

//had to move the createcanvas func here due to the staticframework ref prob
#include <PythonQt.h>
#include <QGroupBox> //just for testing addObject
#include <QtUiTools> //for .ui loading in testing
#include <QApplication>
#include "UiWidgetProperties.h"
#include "UiProxyWidget.h"

//the new qt integration, the previous stuff (above) still used for 3d inworld things
#include <UiModule.h>

#include "Vector3Wrapper.h"
#include "QuaternionWrapper.h"

#include "propertyeditor.h"

//now that binding uicanvases (from mediahandler webviews) to 3d objects is here
//#include "EC_UICanvas.h"

#include "QEC_Prim.h"
#include "RexUUID.h"

namespace PythonScript
{
    PythonScriptModule::PythonScriptModule() : ModuleInterfaceImpl(type_static_)
    {
        pythonqt_inited = false;
        inboundCategoryID_ = 0;
        inputeventcategoryid = 0;
        networkstate_category_id = 0;
        framework_category_id = 0;
    }

    PythonScriptModule::~PythonScriptModule()
    {
    }

    // virtual
    void PythonScriptModule::Load()
    {
        using namespace PythonScript;

        LogInfo(Name() + " loaded.");
        //DECLARE_MODULE_EC(EC_OgreEntity);

        AutoRegisterConsoleCommand(Console::CreateCommand(
            "PyExec", "Execute given code in the embedded Python interpreter. Usage: PyExec(mycodestring)", 
            Console::Bind(this, &PythonScriptModule::ConsoleRunString))); 
        /* NOTE: called 'exec' cause is similar to py shell builtin exec() func.
         * Also in the IPython shell 'run' refers to running an external file and not the given string
         */

        AutoRegisterConsoleCommand(Console::CreateCommand(
            "PyLoad", "Execute a python file. PyLoad(mypymodule)", 
            Console::Bind(this, &PythonScriptModule::ConsoleRunFile))); 

        AutoRegisterConsoleCommand(Console::CreateCommand(
            "PyReset", "Resets the Python interpreter - should free all it's memory, and clear all state.", 
            Console::Bind(this, &PythonScriptModule::ConsoleReset))); 

    }

    // virtual
    void PythonScriptModule::Unload()
    {
        LogInfo(Name() + " unloaded.");
    }

    // virtual
    void PythonScriptModule::Initialize()
    {
        PythonScript::pythonscriptmodule_ = this;
        if (!engine_)
        {
            engine_ = PythonScript::PythonEnginePtr(new PythonScript::PythonEngine(framework_));
        }
        engine_->Initialize();
              
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Scripting, engine_);
        //XXX hack to have a ref to framework for api funcs
        PythonScript::staticframework = framework_;
        apiModule = PythonScript::initpymod(); //initializes the rexviewer module to be imported within py

        //init PythonQt, implemented in RexPythonQt.cpp
        if (!pythonqt_inited)
        {
            PythonScript::initRexQtPy(apiModule);
            //PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
            //mainModule.addObject("qtmodule", wrappedModule); 
            pythonqt_inited = true;
            
            //PythonQt::self()->registerCPPClass("Vector3df", "","", PythonQtCreateObject<Vector3Wrapper>);
            //PythonQt::self()->registerCPPClass("Quaternion", "","", PythonQtCreateObject<QuaternionWrapper>);
			//PythonQt::self()->registerClass(&Vector3::staticMetaObject);
        }

        //load the py written module manager using the py c api directly
        pmmModule = PyImport_ImportModule("modulemanager");
        if (pmmModule == NULL) {
            LogError("Failed to import py modulemanager");
            return;
        }
        pmmDict = PyModule_GetDict(pmmModule);
        if (pmmDict == NULL) {
            LogError("Unable to get modulemanager module namespace");
            return;
        }
        pmmClass = PyDict_GetItemString(pmmDict, "ModuleManager");
        if(pmmClass == NULL) {
            LogError("Unable get ModuleManager class from modulemanager namespace");
            return;
        }
        //instanciating the manager moved to PostInitialize, 
        //'cause it does autoload.py where TestComponent expects the event constants to be there already

        //std::string error;
        //modulemanager = engine_->LoadScript("modulemanager", error); //the pymodule loader & event manager
        //modulemanager = modulemanager->GetObject("ModuleManager"); //instanciates
        mouse_left_button_down_ = false;
        mouse_right_button_down_ = false;

        LogInfo(Name() + " initialized succesfully.");
    }

    void PythonScriptModule::PostInitialize()
    {
        em_ = framework_->GetEventManager();        

        // Get Framework category, so we can listen to its event about protocol module ready,
        // then we can subscribe to the other networking categories
        framework_category_id = em_->QueryEventCategory("Framework");
        if (framework_category_id == 0)
            LogWarning("Unable to find event category for incoming Framework events!");

        // Input (OIS)
        inputeventcategoryid = em_->QueryEventCategory("Input");
        if (inputeventcategoryid == 0)
            LogError("Unable to find event category for Input");

        // Scene (SceneManager)
        PythonScript::scene_event_category_ = em_->QueryEventCategory("Scene");
        if (PythonScript::scene_event_category_ == 0)
            LogWarning("Unable to find event category for Scene events!");
        
        /* add events constants - now just the input events */
        //XXX move these to some submodule ('input'? .. better than 'constants'?)
        /*PyModule_AddIntConstant(apiModule, "MOVE_FORWARD_PRESSED", Input::Events::MOVE_FORWARD_PRESSED);
        PyModule_AddIntConstant(apiModule, "MOVE_FORWARD_RELEASED", Input::Events::MOVE_FORWARD_RELEASED);
        LogInfo("Added event constants.");*/

        /* TODO: add other categories and expose the hierarchy as py submodules or something,
        add registrating those (it's not (currently) mandatory),
        to the modules themselves, e.g. InputModule (currently the OIS thing but that is to change) */
        const Foundation::EventManager::EventMap &evmap = em_->GetEventMap();
        Foundation::EventManager::EventMap::const_iterator cat_iter = evmap.find(inputeventcategoryid);
        if (cat_iter != evmap.end())
        {
            std::map<event_id_t, std::string> evs = cat_iter->second;
            for (std::map<event_id_t, std::string>::iterator ev_iter = evs.begin();
                ev_iter != evs.end(); ++ev_iter)
            {
                /*std::stringstream ss;
                ss << ev_iter->first << " (id:" << ev_iter->second << ")";
                LogInfo(ss.str());*/
                PyModule_AddIntConstant(apiModule, ev_iter->second.c_str(), ev_iter->first);
            }
        }
        else
            LogInfo("No registered events in the input category.");

        /*for (Foundation::EventManager::EventMap::const_iterator iter = evmap[inputeventcategoryid].begin();
            iter != evmap[inputeventcategoryid].end(); ++iter)
        {
            std::stringstream ss;
            ss << iter->first << " (id:" << iter->second << ")";
            LogInfo(ss.str());
        }*/
        
        /* TODO perhaps should expose all categories, so any module would get it's events exposed automagically 
        const Foundation::EventManager::EventCategoryMap &categories = em.GetEventCategoryMap();
        for(Foundation::EventManager::EventCategoryMap::const_iterator iter = categories.begin();
            iter != categories.end(); ++iter)
        
            std::stringstream ss;
            ss << iter->first << " (id:" << iter->second << ")";

            treeiter->set_value(0, ss.str());
        } */

        //now that the event constants etc are there, can instanciate the manager which triggers the loading of components
        if (PyCallable_Check(pmmClass)) {
            pmmInstance = PyObject_CallObject(pmmClass, NULL); 
            LogInfo("Instanciated Py ModuleManager.");
        } else {
            LogError("Unable to create instance from class ModuleManager");
        }

    }

    void PythonScriptModule::SubscribeToNetworkEvents()
    {
        // Network In
        if (inboundCategoryID_ == 0)
        {
            inboundCategoryID_ = em_->QueryEventCategory("NetworkIn");
            if (inboundCategoryID_ == 0)
                LogWarning("Unable to find event category for incoming OpenSimNetwork events!");
            else
                LogInfo("Subscribed to [NetworkIn] events");
        }
        // Network State
        networkstate_category_id = em_->QueryEventCategory("NetworkState");
        if (networkstate_category_id == 0)
            LogError("Unable to find event category for Network");
        else
            LogInfo("Subscribed to [NetworkState] events");
    }

    bool PythonScriptModule::HandleEvent(
        event_category_id_t category_id,
        event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {    
        PyObject* value = NULL;
        //input events. 
        //another option for enabling py handlers for these would be to allow
        //implementing input state in py, see the AvatarController and CameraController in rexlogic
        if (category_id == inputeventcategoryid)
        {
            //key inputs, send the event id and key info (code+mod) for the python side
            if (event_id == Input::Events::KEY_PRESSED || event_id == Input::Events::KEY_RELEASED)
            {
                //InputEvents::Key *key = static_cast<InputEvents::Key *> (data);
                Input::Events::Key* key = checked_static_cast<Input::Events::Key *>(data);
                
                const int keycode = key->code_;
                const int mods = key->modifiers_;
                //OIS::KeyCode* keycode = key->code_;
                
                value = PyObject_CallMethod(pmmInstance, "KEY_INPUT_EVENT", "iii", event_id, keycode, mods);
            }
            
            //port to however uimodule does it, as it replaces OIS now
	    else if(event_id == Input::Events::INWORLD_CLICK || 
		    event_id == Input::Events::LEFT_MOUSECLICK_PRESSED || 
		    event_id == Input::Events::LEFT_MOUSECLICK_RELEASED ||
		    event_id == Input::Events::RIGHT_MOUSECLICK_PRESSED || 
		    event_id == Input::Events::RIGHT_MOUSECLICK_RELEASED)
		{
		    Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);
		    
		    value = PyObject_CallMethod(pmmInstance, "MOUSE_INPUT_EVENT", "iiiii", event_id, movement->x_.abs_, movement->y_.abs_, movement->x_.rel_, movement->y_.rel_);
		}
	    
			else if(event_id == Input::Events::MOUSEDRAG) 
			{
                Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);
                PyObject_CallMethod(pmmInstance, "MOUSE_DRAG_INPUT_EVENT", "iiiii", event_id, movement->x_.abs_, movement->y_.abs_, movement->x_.rel_, movement->y_.rel_);   
			}
			/*
			else if(event_id == Input::Events::MOUSEDRAG_STOPPED)
			{
				PyObject_CallMethod(pmmInstance, "MOUSE_DRAG_INPUT_EVENT", "iiiii", event_id, 0, 0, 0, 0);   
			}
			*/
            else//XXX change to if-else...
            {
                value = PyObject_CallMethod(pmmInstance, "INPUT_EVENT", "i", event_id);
            }
        }
        else if (category_id == scene_event_category_)
        {
            /*
             only handles local modifications so far, needs a network refactorin of entity update events
             to get inbound network entity updates workin
            */
            if (event_id == Scene::Events::EVENT_ENTITY_UPDATED) //XXX remove this and handle with the new generic thing below?
            {
                //LogInfo("Entity updated.");
                Scene::Events::SceneEventData* edata = checked_static_cast<Scene::Events::SceneEventData *>(data);
                unsigned int ent_id = edata->localID;
                if (ent_id != 0)
                    value = PyObject_CallMethod(pmmInstance, "ENTITY_UPDATED", "I", ent_id);
            }
			//todo: add EVENT_ENTITY_DELETED so that e.g. editgui can keep on track in collaborative editing when objs it keeps refs disappear

            //for mediaurl handler
            else if (event_id == Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED) 
            {
                Scene::Events::EntityEventData *entity_data = dynamic_cast<Scene::Events::EntityEventData*>(data);
                if (!entity_data)
                    return false;
                    
                Scene::EntityPtr entity = entity_data->entity;
                if (!entity)
                    return false;

                value = PyObject_CallMethod(pmmInstance, "ENTITY_VISUALS_MODIFIED", "I", entity->GetId());
            }

            //how to pass any event data?
            /*else
            {
                // Note: can't assume that all scene events will use this datatype!!!
                Scene::Events::SceneEventData* edata = dynamic_cast<Scene::Events::SceneEventData *>(data);
                if (edata)
                {
                    unsigned int ent_id = edata->localID;	
                    if (ent_id != 0)
                        value = PyObject_CallMethod(pmmInstance, "SCENE_EVENT", "iI", event_id, ent_id);
                }
            }*/            
        }
        else if (category_id == networkstate_category_id) // if (category_id == "NETWORK?") 
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                value = PyObject_CallMethod(pmmInstance, "LOGIN_INFO", "i", event_id);
            }
			else if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
			{
				value = PyObject_CallMethod(pmmInstance, "SERVER_DISCONNECTED", "i", event_id);
			}

        }
        else if (category_id == framework_category_id && event_id == Foundation::NETWORKING_REGISTERED)
        {
            SubscribeToNetworkEvents();
        }
		        
        //was for first receive chat test, when no module provided it, so handles net event directly
        /* got a crash with this now during login, when the viewer was also getting asset data etc.
           disabling the direct reading of network data here now to be on the safe side,
           this has always behaved correctly till now though (since march). --antont june 12th */

		else if (category_id == inboundCategoryID_)
        {
			ProtocolUtilities::NetworkEventInboundData *event_data = static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
			ProtocolUtilities::NetMsgID msgID = event_data->messageID;
			ProtocolUtilities::NetInMessage *msg = event_data->message;
			const ProtocolUtilities::NetMessageInfo *info = event_data->message->GetMessageInfo();
			//std::vector<ProtocolUtilities::NetMessageBlock> vec = info->blocks;

			//Vector3df data = event_data->message->GetData();
            //assert(info);
			const std::string str = info->name;
			unsigned int id = info->id;

            //testing if the unsigned int actually is the same NetMsgID, in this case RexNetMsgAgentAlertMessage == 0xffff0087
			//if (id == 0xff09) //RexNetMsgObjectProperties == 0xff09
			//	LogInfo("golly... it worked");

            if (id == RexNetMsgGenericMessage)
            {
                PyObject *stringlist = PyList_New(0);
                if (!stringlist)
                    return false;

                std::string cxxmsgname = ProtocolUtilities::ParseGenericMessageMethod(*msg);
                StringVector params = ProtocolUtilities::ParseGenericMessageParameters(*msg);

                for (uint i = 0; i < params.size(); ++i)
                {
                    std::string cxxs = params[i];
                    PyObject *pys = PyString_FromStringAndSize(cxxs.c_str(), cxxs.size());
                    if (!pys) 
                    {
                        //Py_DECREF(stringlist);
                        return false;
                    }
                    PyList_Append(stringlist, pys);
                    Py_DECREF(pys);
                }

                value = PyObject_CallMethod(pmmInstance, "GENERIC_MESSAGE", "sO", cxxmsgname.c_str(), stringlist);
            }
            /*else
            {
    			value = PyObject_CallMethod(pmmInstance, "INBOUND_NETWORK", "Is", id, str.c_str());//, msgID, msg);
            }*/
			
            /*
            std::stringstream ss;
            ss << info->name << " received, " << ToString(msg->GetDataSize()) << " bytes.";
            //LogInfo(ss.str());

            switch(msgID)
            {
            case RexNetMsgChatFromSimulator:
                {
                std::stringstream ss;
                size_t bytes_read = 0;

                std::string name = (const char *)msg->ReadBuffer(&bytes_read);
                msg->SkipToFirstVariableByName("Message");
                std::string message = (const char *)msg->ReadBuffer(&bytes_read);
                
                //ss << "[" << GetLocalTimeString() << "] " << name << ": " << message << std::endl;
                //LogInfo(ss.str());
                //WriteToChatWindow(ss.str());
                //can readbuffer ever return null? should be checked if yes. XXX

                PyObject_CallMethod(pmmInstance, "RexNetMsgChatFromSimulator", "ss", name, message);

                break;
                }
            }*/
        }

        if (value)
        {
            if (PyObject_IsTrue(value))
            {
                //LogInfo("X_INPUT_EVENT returned True.");
                return true;  
            } 
            else 
            {
                //LogInfo("X_INPUT_EVENT returned False.");
                return false;
            }
        }
		return false;
    }

    Console::CommandResult PythonScriptModule::ConsoleRunString(const StringVector &params)
    {
        if (params.size() != 1)
        {            
            return Console::ResultFailure("Usage: PyExec(print 1 + 1)");
            //how to handle input like this? PyExec(print '1 + 1 = %d' % (1 + 1))");
            //probably better have separate py shell.
        }

        else
        {
            engine_->RunString(params[0]);
            return Console::ResultSuccess();
        }
    }

    //void PythonScriptModule::x()
    //{
    //    Vector3df v1 = Vector3df();
    //    using Core;
    //    v2 = Vector3df();
    //    
    //    RexLogic::RexLogicModule *rexlogic_;
    //    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

    //    rexlogic_->GetServerConnection()->SendChatFromViewerPacket("x");

    //    rexlogic_->GetServerConnection()->IsConnected();
    //    rexlogic_->GetCameraControllable()->GetPitch();
    //    
    //    Real newyaw = 0.1;
    //    //rexlogic_->GetAvatarControllable()->SetYaw(newyaw);
    //    rexlogic_->SetAvatarYaw(newyaw);
    //    //rexlogic_->GetAvatarControllable()->AddTime(0.1);
    //    //rexlogic_->GetAvatarControllable()->HandleInputEvent(0, NULL);
    //    
    //    //rexlogic_->GetAvatarControllable()->HandleAgentMovementComplete(Vector3(128, 128, 25), Vector3(129, 129, 24));
    //}

    Console::CommandResult PythonScriptModule::ConsoleRunFile(const StringVector &params)
    {        
        if (params.size() != 1)
        {            
            return Console::ResultFailure("Usage: PyLoad(mypymodule) (to run mypymodule.py by importing it)");
        }

        else
        {
            engine_->RunScript(params[0]);
            return Console::ResultSuccess();
        }
    }

    Console::CommandResult PythonScriptModule::ConsoleReset(const StringVector &params)
    {
        //engine_->Reset();
        Uninitialize(); //does also engine_->Uninitialize();
        Initialize();

        return Console::ResultSuccess();
    }    

    // virtual 
    void PythonScriptModule::Uninitialize()
    {        
        framework_->GetServiceManager()->UnregisterService(engine_);

        if (pmmInstance != NULL) //sometimes when devving it can be, when there was a bug - this helps to be able to reload it
            PyObject_CallMethod(pmmInstance, "exit", "");
        /*char** args = new char*[2]; //is this 2 'cause the latter terminates?
        std::string methodname = "exit";
        std::string paramtypes = ""; //"f"
        modulemanager->CallMethod2(methodname, paramtypes);*/

        engine_->Uninitialize();
        LogInfo(Name() + " uninitialized.");

        em_.reset();
        engine_.reset();
    }
    
    // virtual
    void PythonScriptModule::Update(f64 frametime)
    {
        //XXX remove when/as the core has the fps limitter
        //engine_->RunString("import time; time.sleep(0.01);"); //a hack to save cpu now.

        // Somehow this causes extreme lag in consoleless mode         
        if (pmmInstance != NULL)
            PyObject_CallMethod(pmmInstance, "run", "f", frametime);
        
        /*char** args = new char*[2]; //is this 2 'cause the latter terminates?
        std::string methodname = "run";
        std::string paramtypes = "f";
        modulemanager->CallMethod2(methodname, paramtypes, 0.05); //;frametime);
        */        

        /* Mouse input special handling. InputModuleOIS has sending these as events commented out,
           This polling is copy-pasted from the InputHandler in RexLogicModule */
        //boost::shared_ptr<Input::InputServiceInterface> input = framework_->GetService<Input::InputServiceInterface>(Foundation::Service::ST_Input).lock();

        //XXX not ported to UImodule / OIS replacement yet
   //     boost::shared_ptr<Input::InputModuleOIS> input = framework_->GetModuleManager()->GetModule<Input::InputModuleOIS>(Foundation::Module::MT_Input).lock();
   //     if (input)
   //     {
   //         //boost::optional<const Input::Events::Movement&> movement = input->PollSlider(Input::Events::MOUSELOOK);
   //         boost::optional<const Input::Events::Movement&> movement = input->GetMouseMovement();
   //         if (movement)
   //         {
   //             //LogDebug("me sees mouse move too");

   //             //might perhaps wrap that nice pos class later but this is simpler now
   //             //float x_abs = static_cast<float>(movement->x_.abs_);
   //             //float y_abs = static_cast<float>(movement->y_.abs_);
   //             //
   //             //float x_rel = static_cast<float>(movement->x_.rel_);
   //             //float y_rel = static_cast<float>(movement->y_.rel_);

   //             int x_abs = movement->x_.abs_;
   //             int y_abs = movement->y_.abs_;
   //             int x_rel = movement->x_.rel_;
   //             int y_rel = movement->y_.rel_;

			//    //was only sending the mouse_movement event if one of the buttons is pressed, XXX change?
			//    //if (mouse_left_button_down_ || mouse_right_button_down_)
   //             PyObject_CallMethod(pmmInstance, "MOUSE_MOVEMENT", "iiii", x_abs, y_abs, x_rel, y_rel);                
			//}
   //     }

    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace PythonScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(PythonScriptModule)
POCO_END_MANIFEST

#ifdef __cplusplus
extern "C"
#endif

/* API calls exposed to py. 
will probably be wrapping the actual modules in separate files,
but first test now here. also will use boostpy or something, but now first by hand */
PyObject* SendChat(PyObject *self, PyObject *args)
{
    const char* msg;

    if(!PyArg_ParseTuple(args, "s", &msg))
    {
        PyErr_SetString(PyExc_ValueError, "param should be a string.");
        return NULL;
    }

    //Foundation::Framework *framework_ = Foundation::ComponentInterfacePythonScriptModule::GetFramework();
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;//
    //todo weak_pointerize

    //move decl to .h and getting to Initialize (see NetTEstLogicModule::Initialize)
    //if this kind of usage, i.e. getting the logic module for the api, is to remain.
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

    rexlogic_->GetServerConnection()->SendChatFromViewerPacket(msg);
    //rexlogic_->GetServerConnection()->IsConnected();
    //Real newyaw = 0.1;
    //rexlogic_->GetAvatarControllable()->SetYaw(newyaw);
    //rexlogic_->GetCameraControllable()->GetPitch();
    //rexlogic_->GetAvatarControllable()->HandleAgentMovementComplete(Vector3(128, 128, 25), Vector3(129, 129, 24));

    Py_RETURN_TRUE;
}

static PyObject* SetAvatarRotation(PyObject *self, PyObject *args)
{
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    float x, y, z, w;

    if(!PyArg_ParseTuple(args, "ffff", &x, &y, &z, &w))
    {
        PyErr_SetString(PyExc_ValueError, "Value error, need x, y, z, w params");
        return NULL;
    }
    std::cout << "Sending newrot..." << std::endl;
    Quaternion newrot(x, y, z, w); //seriously, is this how constructing a quat works!?
    rexlogic_->SetAvatarRotation(newrot);

    Py_RETURN_NONE;
}

//returns the entity at the position (x, y), if nothing there, returns False    
static PyObject* RayCast(PyObject *self, PyObject *args)
{
    float x, y;
    
    if(!PyArg_ParseTuple(args, "ff", &x, &y)){
        PyErr_SetString(PyExc_ValueError, "Raycasting failed due to ValueError, needs (x, y) values.");
        return NULL;   
    }

    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    boost::shared_ptr<Foundation::RenderServiceInterface> render = framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
	//Scene::Entity *entity = render->Raycast(x, y).entity_;
	Foundation::RaycastResult result = render->Raycast(x, y);
	if (result.entity_)
		return Py_BuildValue("IfffIff", result.entity_->GetId(), result.pos_.x, result.pos_.y, result.pos_.z, result.submesh_, float(result.u_), float(result.v_));
	else
		Py_RETURN_NONE;
	/*
    if (result)
    {
        //Scene::Events::SceneEventData event_data(entity->GetId());
        //framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_GRAB, &event_data);
        return entity_create(entity->GetId());
    }
    else 
        Py_RETURN_NONE;
	*/
}
static PyObject* TakeScreenshot(PyObject *self, PyObject *args)
{
    const char* filePath;
    const char* fileName;

    std::string oFilePath;
    std::string oFileName;

    if(!PyArg_ParseTuple(args, "ss", &filePath, &fileName))
        PyErr_SetString(PyExc_ValueError, "Getting the filepath and filename failed.");

    
    oFilePath = filePath;
    oFileName = fileName;
    
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (renderer){

        //std::cout << "Screenshot in PYSM ... " << std::endl;
        renderer->TakeScreenshot(oFilePath, oFileName);
    }
    else
        std::cout << "Failed ..." << std::endl;

    Py_RETURN_NONE;
}

static PyObject* SwitchCameraState(PyObject *self)
{
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    rexlogic_->SwitchCameraState();
    Py_RETURN_NONE;
}

static PyObject* SendEvent(PyObject *self, PyObject *args)
{
    std::cout << "PySendEvent" << std::endl;
    unsigned int event_id_int;//, ent_id_int = 0;
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    //entity_id_t ent_id;
    event_id_t event_id;

    if(!PyArg_ParseTuple(args, "i", &event_id_int))//, &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an event id failed, param should be an integer.");
        return NULL;   
    }
    
    //ent_id = (entity_id_t) ent_id_int;
    event_id = (event_id_t) event_id_int;
    event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
    if (event_id == Input::Events::SWITCH_CAMERA_STATE) 
    {
        std::cout << "switch camera state gotten!" << std::endl;
        framework_->GetEventManager()->SendEvent(event_category, event_id, NULL);//&event_data);
    } 
    else
        std::cout << "failed..." << std::endl;

    
    Py_RETURN_TRUE;
}

//returns an Entity wrapper, is in actual use
PyObject* GetEntity(PyObject *self, PyObject *args)
{
    unsigned int ent_id_int;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity failed, param should be an integer.");
        return NULL;   
    }

    ent_id = (entity_id_t) ent_id_int;

    Scene::ScenePtr scene = PythonScript::GetScene();

    if (scene == 0)
    {
        PyErr_SetString(PyExc_ValueError, "Scene is none.");
        return NULL;   
    }

    //PythonScript::foo(); 
    /*
    const Scene::EntityPtr entity = scene->GetEntity(ent_id);
    if (entity.get() != 0) //same that scene->HasEntity does, i.e. it first does GetEntity too, so not calling HasEntity here to not do GetEntity twice.
        return entity_create(ent_id, entity);
    //the ptr is stored in a separate map now, so also id needs to passed to entity_create
    */
    //if would just store the id and always re-get ptr, would do this:
    if (scene->HasEntity(ent_id))
        return entity_create(ent_id);

    else
    {
        PyErr_SetString(PyExc_ValueError, "No entity with the ent_id found.");
        return NULL;   
    }
}

PyObject* GetEntityByUUID(PyObject *self, PyObject *args)
{
    char* uuidstr;
    if(!PyArg_ParseTuple(args, "s", &uuidstr))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity by UUID failed, param should be a string.");
        return NULL;
    }

	RexUUID ruuid = RexUUID();
	ruuid.FromString(std::string(uuidstr));

	RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        //PythonScript::self()->LogInfo("Getting prim with UUID:" + ruuid.ToString());
        Scene::EntityPtr entity = rexlogic_->GetPrimEntity(ruuid);
        if (entity)
        {
            return entity_create(entity->GetId());
        }
        else
        {
            //PyErr_SetString(PyExc_RuntimeError, "Entity found by UUID is not in default scene?");
            //return NULL;   
            Py_RETURN_NONE; //not there yet, when still loading?
        }
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError, "RexLogic module not there?");
        return NULL;   
    }
}

//PyObject* GetEntityMatindicesWithTexture(PyObject* self, PyObject* args)
//qt module is gone, this waits for uimodule to bring ec_canvases back
//PyObject* ApplyUICanvasToSubmeshesWithTexture(PyObject* self, PyObject* args)
//{
//	PyObject* pyuicanvas;
//    char* uuidstr;
//
//    if(!PyArg_ParseTuple(args, "Os", &pyuicanvas, &uuidstr))
//    {
//        //PyErr_SetString(PyExc_ValueError, "Setting uicanvas to textures failed,.");
//        return NULL;
//    }
//
//	if (!PyObject_TypeCheck(pyuicanvas, &PythonQtInstanceWrapper_Type))
//	{
//		return NULL;
//	}
//
//	PythonQtInstanceWrapper* wrapped_uicanvas = (PythonQtInstanceWrapper*)pyuicanvas;
//
//	QObject* qobject_ptr = wrapped_uicanvas->_obj;
//    QtUI::UICanvas* canvas_ptr = (QtUI::UICanvas*) qobject_ptr; //XXX can this be checked, in case some other qobject was given in error?
//    
//	RexUUID textureuuid = RexUUID();
//	textureuuid.FromString(std::string(uuidstr));
//
//    // Get QtModule
//    Foundation::ModuleSharedPtr qt_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule("QtModule").lock();
//    QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
//
//    //we have a raw canvas pointer (from PythonQt), but need the shared_ptr in Naali
//    //this is not allowed: boost::shared_ptr<QtUI::UICanvas> canvas = boost::shared_ptr<QtUI::UICanvas>(canvas_ptr);
//    boost::shared_ptr<QtUI::UICanvas> canvas;
//    QList<boost::shared_ptr<QtUI::UICanvas> > canvases;
//    canvases = qt_ui->GetCanvases();
//    QList<boost::shared_ptr<QtUI::UICanvas> >::const_iterator i;
//    for (i = canvases.constBegin(); i != canvases.constEnd(); ++i)
//    {
//        boost::shared_ptr<QtUI::UICanvas> thiscanv = *i;
//        if (thiscanv.get() == canvas_ptr)
//        {
//            canvas = thiscanv;
//            break;
//        }
//    }
//
//    //_ApplyUICanvasToSubmeshesWithTexture(uicanvas_ptr, textureuuid);
//
////    Py_RETURN_NONE;
////}
//
////the impl for above - not py specific, should be in rexlogic or qtmodule or somewhere.
////void _ApplyUICanvasToSubmeshesWithTexture(boost::shared_ptr<QtUI::UICanvas> canvas, RexUUID textureuuid)
////{
//    
//    // Get RexLogic module
//    RexLogic::RexLogicModule *rexlogicmodule_;
//    rexlogicmodule_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
//
//    Scene::ScenePtr scene = PythonScript::GetScene();        
//    if (!scene) { //XXX enable the check || !rexlogicmodule_->GetFramework()->GetComponentManager()->CanCreate(OgreRenderer::EC_OgrePlaceable::NameStatic()))
//        PyErr_SetString(PyExc_RuntimeError, "Default scene is not there in GetEntityMatindicesWithTexture.");
//        return NULL;   
//    }
//
//    //which -- if any -- submeshes / mat indices should get the 3duicanvas applied.
//    std::vector<uint> submeshes_;
//
//    for(Scene::SceneManager::iterator iter = scene->begin();
//        iter != scene->end(); ++iter)
//    {
//        Scene::Entity &entity = **iter;
//        submeshes_.clear();
//
//        Scene::EntityPtr primentity = rexlogicmodule_->GetPrimEntity(entity.GetId());
//        if (!primentity) continue;
//        RexLogic::EC_OpenSimPrim &prim = *checked_static_cast<RexLogic::EC_OpenSimPrim*>(entity.GetComponent(RexLogic::EC_OpenSimPrim::NameStatic()).get());
//        
//        if (prim.DrawType == RexTypes::DRAWTYPE_MESH)
//        {
//            Foundation::ComponentPtr mesh = entity.GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
//            if (!mesh) continue;
//            OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());
//            // If don't have the actual mesh entity yet, no use trying to set texture
//            if (!meshptr->GetEntity()) continue;
//            
//            RexLogic::MaterialMap::const_iterator i = prim.Materials.begin();
//            while (i != prim.Materials.end())
//            {
//                uint idx = i->first;
//                if ((i->second.Type == RexTypes::RexAT_Texture) && (i->second.asset_id.compare(textureuuid.ToString()) == 0))
//                {
//                    // Use a legacy material with the same name as the texture, created automatically by renderer
//                    //meshptr->SetMaterial(idx, res->GetId());
//
//                    submeshes_.push_back(idx);
//
//                    std::stringstream ss;
//                    ss << "Entity with given texture id found: ";
//                    ss << textureuuid.ToString();
//                    ss << " - ";
//                    ss << primentity->GetId();
//                    ss << " / ";
//                    ss << idx;
//                    PythonScript::self()->LogInfo(ss.str());
//                }
//                ++i;
//            }
//
//            if (submeshes_.size() > 0)
//            {
//                QtUI::EC_UICanvas* ec = dynamic_cast<QtUI::EC_UICanvas*>(qt_ui->CreateEC_UICanvasToEntity(&entity, canvas).get());
//                if (ec)
//                    ec->SetSubmeshes(submeshes_);
//            }
//        }
//    }
//
//    Py_RETURN_NONE;
//}

PyObject* GetSubmeshesWithTexture(PyObject* self, PyObject* args)
{
    // Read params from py: entid as uint, textureuuid as string
    unsigned int ent_id_int;
    char* uuidstr;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "Is", &ent_id_int, &uuidstr))
        return NULL;   

    ent_id = (entity_id_t) ent_id_int;
    
	RexUUID textureuuid = RexUUID();
	textureuuid.FromString(std::string(uuidstr));

    // Get RexLogic module
    RexLogic::RexLogicModule *rexlogicmodule_;
    rexlogicmodule_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

    Scene::EntityPtr primentity = rexlogicmodule_->GetPrimEntity(ent_id);
    if (!primentity) Py_RETURN_NONE;
    RexLogic::EC_OpenSimPrim &prim = *checked_static_cast<RexLogic::EC_OpenSimPrim*>(primentity->GetComponent(RexLogic::EC_OpenSimPrim::NameStatic()).get());
    
    if (prim.DrawType == RexTypes::DRAWTYPE_MESH)
    {
        Foundation::ComponentPtr mesh = primentity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (!mesh) Py_RETURN_NONE;
        OgreRenderer::EC_OgreMesh* meshptr = checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh.get());
        // If don't have the actual mesh entity yet, no use trying to set texture
        if (!meshptr->GetEntity()) Py_RETURN_NONE;
        
        RexLogic::MaterialMap::const_iterator i = prim.Materials.begin();
        
        //which -- if any -- submeshes / mat indices use the given textureuuid in this mesh
        std::vector<uint> submeshes_;

        while (i != prim.Materials.end())
        {
            uint idx = i->first;
            if ((i->second.Type == RexTypes::RexAT_Texture) && (i->second.asset_id.compare(textureuuid.ToString()) == 0))
            {
                // Use a legacy material with the same name as the texture, created automatically by renderer
                //meshptr->SetMaterial(idx, res->GetId());

                submeshes_.push_back(idx);

                std::stringstream ss;
                ss << "Subface in the entity with given texture id found: ";
                ss << idx;
                ss << " / ";
                ss << textureuuid.ToString();
                PythonScript::self()->LogInfo(ss.str());
            }
            ++i;
        }

        if (submeshes_.size() > 0)
        {
            PyObject* py_submeshes = PyList_New(submeshes_.size());
            
            for (int i = 0; i < submeshes_.size(); i++) //std::vector<uint>::iterator iter = submeshes_.begin(); iter != submeshes_.end(); ++iter)
            {
                uint submesh = submeshes_[i]; //*iter;
                PyList_SET_ITEM(py_submeshes, i, Py_BuildValue("I", submesh));
            }

            return py_submeshes;            
        }
    }

    Py_RETURN_NONE;
}

//PyObject* ApplyUICanvasToSubmeshes(PyObject* self, PyObject* args)
//{
//    unsigned int ent_id_int;
//    PyObject* py_submeshes;
//	PyObject* pyuicanvas;
//    entity_id_t ent_id;
//
//    // Read params from py: entid as uint, a canvas qobject wrapped by pythonqt
//    if(!PyArg_ParseTuple(args, "IOO", &ent_id_int, &py_submeshes, &pyuicanvas))
//    {
//        //PyErr_SetString(PyExc_ValueError, "Setting uicanvas to textures failed,.");
//        return NULL;
//    }
//
//    ent_id = (entity_id_t) ent_id_int;
//    RexLogic::RexLogicModule *rexlogicmodule_;
//    rexlogicmodule_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
//    Scene::EntityPtr primentity = rexlogicmodule_->GetPrimEntity(ent_id);
//    if (!primentity) Py_RETURN_NONE; //XXX exception would be better?
//
//    if (!PyList_Check(py_submeshes))
//        return NULL;
//
//	if (!PyObject_TypeCheck(pyuicanvas, &PythonQtInstanceWrapper_Type))
//		return NULL;
//
//	PythonQtInstanceWrapper* wrapped_uicanvas = (PythonQtInstanceWrapper*)pyuicanvas;
//
//	QObject* qobject_ptr = wrapped_uicanvas->_obj;
//    QtUI::UICanvas* canvas_ptr = (QtUI::UICanvas*) qobject_ptr; //XXX can this be checked, in case some other qobject was given in error?
//    
//    // Get QtModule
//    Foundation::ModuleSharedPtr qt_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule("QtModule").lock();
//    QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
//
//    //we have a raw canvas pointer (from PythonQt), but need the shared_ptr in Naali
//    //this is not allowed: boost::shared_ptr<QtUI::UICanvas> canvas = boost::shared_ptr<QtUI::UICanvas>(canvas_ptr);
//    boost::shared_ptr<QtUI::UICanvas> canvas;
//    QList<boost::shared_ptr<QtUI::UICanvas> > canvases;
//    canvases = qt_ui->GetCanvases();
//    QList<boost::shared_ptr<QtUI::UICanvas> >::const_iterator i;
//    for (i = canvases.constBegin(); i != canvases.constEnd(); ++i)
//    {
//        boost::shared_ptr<QtUI::UICanvas> thiscanv = *i;
//        if (thiscanv.get() == canvas_ptr)
//        {
//            canvas = thiscanv;
//            break;
//        }
//    }
//
//    //bleh, should really make this a qt slot somewhere and use a QList
//	//that is: all this is just to convert a python list to a std::vector.
//    std::vector<uint> submeshes_;
//    PyObject* py_iter;
//    PyObject* item;
//    py_iter = PyObject_GetIter(py_submeshes);
//    while(item=PyIter_Next(py_iter))
//    {
//        PyObject* py_int;
//        py_int = PyNumber_Int(item);
//        if (!item)
//        {
//            Py_DECREF(py_iter);
//            Py_DECREF(item);
//            PyErr_SetString(PyExc_ValueError, "Submesh indexes for applying uicanvases must be integers.");
//            return NULL;
//        }
//        submeshes_.push_back((uint)PyInt_AsLong(py_int));
//        Py_DECREF(item);
//    }
//    Py_DECREF(py_iter);
//
//    QtUI::EC_UICanvas* ec = dynamic_cast<QtUI::EC_UICanvas*>(qt_ui->CreateEC_UICanvasToEntity(primentity.get(), canvas).get());
//    if (ec)
//        ec->SetSubmeshes(submeshes_);
//
//    Py_RETURN_NONE;
//}


//returns the internal Entity that's now a QObject, 
//with no manual wrapping (just PythonQt exposing qt things)
//experimental now, may replace the PyType in Entity.h used above
//largely copy-paste from above now, is an experiment
/*PyObject* GetQEntity(PyObject *self, PyObject *args)
{
    unsigned int ent_id_int;
    entity_id_t ent_id;
    const Scene::EntityPtr entityptr;
    const Scene::Entity entity;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity failed, param should be an integer.");
        return NULL;   
    }

    ent_id = (entity_id_t) ent_id_int;

    Scene::ScenePtr scene = PythonScript::GetScene();

    if (scene == 0)
    {
        PyErr_SetString(PyExc_ValueError, "Scene is none.");
        return NULL;   
    }

    entityptr = scene->GetEntity(ent_id);
    if (entity.get() != 0) //same that scene->HasEntity does, i.e. it first does GetEntity too, so not calling HasEntity here to not do GetEntity twice.
    {
        return PythonQt::self()->wrapQObject(entity.get());
    }

    else
    {
        PyErr_SetString(PyExc_ValueError, "No entity with the given id found."); //XXX add the id to msg
        return NULL;   
    }
}*/

PyObject* GetQPrim(PyObject* self, PyObject* args)
{
    unsigned int ent_id_int;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        return NULL;
    }

    ent_id = (entity_id_t) ent_id_int;

    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    Scene::EntityPtr primentity = rexlogic_->GetPrimEntity(ent_id);
    if (!primentity) 
    {
	PyErr_SetString(PyExc_ValueError, "The entity id given to getQPrim does not have a prim component.");
	return NULL;
    }
    RexLogic::EC_OpenSimPrim* prim = checked_static_cast<RexLogic::EC_OpenSimPrim*>(primentity->GetComponent(RexLogic::EC_OpenSimPrim::NameStatic()).get());

    return PythonQt::self()->wrapQObject(prim);
}

PyObject* GetQPlaceable(PyObject* self, PyObject* args)
{
    unsigned int ent_id_int;
    entity_id_t ent_id;
    Scene::EntityPtr entity;
    OgreRenderer::EC_OgrePlaceable* placeable = 0;
    Scene::ScenePtr scene = PythonScript::GetScene();

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        return NULL;
    }

    ent_id = (entity_id_t) ent_id_int;
    entity = scene->GetEntity(ent_id);

    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
    
    if (!ogre_component)
    {
	PyErr_SetString(PyExc_AttributeError, "GetQPlaceable: Given entity id does not have a placeable component.");
	return NULL;   
    }

    placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
    return PythonQt::self()->wrapQObject(placeable);
}

PyObject* CreateEntity(PyObject *self, PyObject *value)
{
    Foundation::Framework *framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    RexLogic::RexLogicModule *rexlogic_;

    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    
    std::string meshname;
    const char* c_text;
    float prio = 0;
    //PyObject_Print(value, stdout, 0);
    if(!PyArg_ParseTuple(value, "sf", &c_text, &prio))
    {
        PyErr_SetString(PyExc_ValueError, "mesh name is a string"); //XXX change the exception
        return NULL;
    }

    meshname = std::string(c_text);

    Scene::ScenePtr scene = PythonScript::GetScene();        
    if (!scene){ //XXX enable the check || !rexlogicmodule_->GetFramework()->GetComponentManager()->CanCreate(OgreRenderer::EC_OgrePlaceable::NameStatic()))
        PyErr_SetString(PyExc_RuntimeError, "Default scene is not there in CreateEntity.");
        return NULL;   
    }

    entity_id_t ent_id = scene->GetNextFreeId(); //instead of using the id given
    
    StringVector defaultcomponents;
    defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());
    //defaultcomponents.push_back(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
    defaultcomponents.push_back(OgreRenderer::EC_OgreMesh::NameStatic());
    //defaultcomponents.push_back(OgreRenderer::EC_OgreAnimationController::NameStatic());
        
    Scene::EntityPtr entity = scene->CreateEntity(ent_id, defaultcomponents);

    Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
    if (placeable)
    {
        OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
	if (prio != 0)
	    ogrepos.SetSelectPriority(prio);
	OgreRenderer::EC_OgreMesh &ogremesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
	ogremesh.SetPlaceable(placeable);
	ogremesh.SetMesh(meshname, true);
	
        return entity_create(ent_id); //return the py wrapper for the new entity
    }
    
    PyErr_SetString(PyExc_ValueError, "placeable not found."); //XXX change the exception
    return NULL;   
}

/*
//camera zoom - send an event like logic CameraControllable does:
            CameraZoomEvent event_data;
            //event_data.entity = entity_.lock(); // no entity for camera, :( -cm
            event_data.amount = checked_static_cast<Input::Events::SingleAxisMovement*>(data)->z_.rel_;
            //if (event_data.entity) // only send the event if we have an existing entity, no point otherwise
            framework_->GetEventManager()->SendEvent(action_event_category_, RexTypes::Actions::Zoom, &event_data);
*/

//XXX logic CameraControllable has GetPitch, perhaps should have SetPitch too
PyObject* SetCameraYawPitch(PyObject *self, PyObject *args) 
{
    Real newyaw, newpitch;
    float y, p;
    if(!PyArg_ParseTuple(args, "ff", &y, &p)) {
        PyErr_SetString(PyExc_ValueError, "New camera yaw and pitch expected as float, float.");
        return NULL;
    }
    newyaw = (Real) y;
    newpitch = (Real) p;

    //boost::shared_ptr<OgreRenderer::Renderer> renderer = PythonScript::staticframework->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        //boost::shared_ptr<RexLogic::CameraControllable> cam = rexlogic_->GetCameraControllable();
        //cam->HandleInputEvent(PythonScript::PythonScriptModule::inputeventcategoryid, &x);
        //cam->AddTime((Real) 0.1);
        //cam->SetPitch(p); //have a linking prob with this
        rexlogic_->SetCameraYawPitch(y, p);
    }
    
    //was with renderer, worked but required overriding rexlogic :p
    //{
    //    Ogre::Camera *camera = renderer->GetCurrentCamera();
    //    camera->yaw(Ogre::Radian(newyaw));
    //    camera->pitch(Ogre::Radian(newpitch));
    //}
    else
    {
        PyErr_SetString(PyExc_RuntimeError, "No logic module, no cameracontrollable.");
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject* GetCameraYawPitch(PyObject *self, PyObject *args) 
{
    Real yaw, pitch;

    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        boost::shared_ptr<RexLogic::CameraControllable> cam = rexlogic_->GetCameraControllable();
        pitch = cam->GetPitch();
        yaw = 0; //XXX not implemented yet (?)

        return Py_BuildValue("ff", (float)pitch, float(yaw));
    }
    //else - no logic module. can that ever happen?)
    return NULL; //rises py exception
}
PyObject* PyLogInfo(PyObject *self, PyObject *args) 
{
    const char* message;    
    if(!PyArg_ParseTuple(args, "s", &message))
    {
        PyErr_SetString(PyExc_ValueError, "Needs a string.");
        return NULL;
    }
    PythonScript::self()->LogInfo(message);
    
    Py_RETURN_NONE;
}

PyObject* PyLogDebug(PyObject *self, PyObject *args) 
{
    const char* message;
    if(!PyArg_ParseTuple(args, "s", &message))
    {
        PyErr_SetString(PyExc_ValueError, "Needs a string.");
        return NULL;
    }
    PythonScript::self()->LogDebug(message);
    
    Py_RETURN_NONE;
}
PyObject* SetAvatarYaw(PyObject *self, PyObject *args)
{
    Real newyaw;

    float y;
    if(!PyArg_ParseTuple(args, "f", &y)) {
        PyErr_SetString(PyExc_ValueError, "New avatar yaw expected as float.");
        return NULL;
    }
    newyaw = (Real) y;

    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        rexlogic_->GetServerConnection()->IsConnected();
        //had linking problems with these, hopefully can be solved somehow easily.
        //rexlogic_->GetAvatarControllable()->SetYaw(newyaw);
        //boost::shared_ptr<RexLogic::AvatarControllable> avc = rexlogic_->GetAvatarControllable();
        //avc->SetYaw(newyaw);
        //f64 t = (f64) 0.01;
        //avc->AddTime(t);
        //rexlogic_->GetAvatarControllable()->HandleAgentMovementComplete(Vector3(128, 128, 25), Vector3(129, 129, 24));
        rexlogic_->SetAvatarYaw(newyaw);
    }
    
    else
    {
        PyErr_SetString(PyExc_RuntimeError, "No logic module, no AvatarControllable.");
        return NULL;
    }

    Py_RETURN_NONE;
}

//PyObject* CreateCanvas(PyObject *self, PyObject *args)
//{        
//    if (!PythonScript::self()->GetFramework())//PythonScript::staticframework)
//    {
//        //std::cout << "Oh crap staticframework is not there! (py)" << std::endl;
//        PythonScript::self()->LogInfo("PythonScript's framework is not present!");
//        return NULL;
//    }
//
//    int imode;
//
//    if(!PyArg_ParseTuple(args, "i", &imode))
//    {
//        PyErr_SetString(PyExc_ValueError, "Getting the mode failed, need 0 / 1");
//        return NULL;   
//    }
//    
//    boost::shared_ptr<QtUI::QtModule> qt_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
//    boost::shared_ptr<QtUI::UICanvas> canvas_;
//    
//    if ( qt_module.get() == 0)
//        return NULL;
//    
//    QtUI::UICanvas::DisplayMode rMode = (QtUI::UICanvas::DisplayMode) imode;
//    canvas_ = qt_module->CreateCanvas(rMode).lock();
//
//    QtUI::UICanvas* qcanvas = canvas_.get();
//    
//	PyObject* can = PythonQt::self()->wrapQObject(qcanvas);
//
//    return can;
//}

PyObject* CreateUiWidgetProperty(PyObject *self, PyObject *args)
{        
    if (!PythonScript::self()->GetFramework())//PythonScript::staticframework)
    {
        //std::cout << "Oh crap staticframework is not there! (py)" << std::endl;
        PythonScript::self()->LogInfo("PythonScript's framework is not present!");
        return NULL;
    }
	UiServices::WidgetType type;
    if(!PyArg_ParseTuple(args, "i", &type))
    {
		return NULL;
	}

    UiServices::UiWidgetProperties* prop = new UiServices::UiWidgetProperties("", type);
    return PythonQt::self()->wrapQObject(prop);;
}
PyObject* CreateUiProxyWidget(PyObject* self, PyObject *args)
{
	boost::shared_ptr<UiServices::UiModule> ui_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
	
	PyObject* pywidget;
	PyObject* pyuiprops;

    if(!PyArg_ParseTuple(args, "OO", &pywidget, &pyuiprops))
    {
		return NULL;
	}

	if (!PyObject_TypeCheck(pywidget, &PythonQtInstanceWrapper_Type))
	{
		return NULL;
	}

	if (!PyObject_TypeCheck(pyuiprops, &PythonQtInstanceWrapper_Type))
	{
		return NULL;
	}

	PythonQtInstanceWrapper* wrapped_widget = (PythonQtInstanceWrapper*)pywidget;
	PythonQtInstanceWrapper* wrapped_uiproperty = (PythonQtInstanceWrapper*)pyuiprops;

	QObject* widget_ptr = wrapped_widget->_obj;
	QObject* uiproperty_ptr = wrapped_uiproperty->_obj;

	QWidget* widget = (QWidget*)widget_ptr;
	const UiServices::UiWidgetProperties uiproperty = *(UiServices::UiWidgetProperties*)uiproperty_ptr;
    // If this occurs, we're most probably operating in headless mode.
    if (ui_module.get() == 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "UiModule is missing."); //XXX perhaps should not be an error, 'cause some things should just work in headless without complaining
        return NULL;
    }

	UiServices::UiProxyWidget* uiproxywidget = new UiServices::UiProxyWidget(widget, uiproperty);
	return PythonQt::self()->wrapQObject(uiproxywidget);
}

PyObject* GetPropertyEditor(PyObject *self)
{
	QApplication* qapp = PythonScript::self()->GetFramework()->GetQApplication();
	PropertyEditor::PropertyEditor* pe = new PropertyEditor::PropertyEditor(qapp);
	return PythonQt::self()->wrapQObject(pe); 
}

//PyObject* GetQtModule(PyObject *self)
//{
//    return PythonQt::self()->wrapQObject(PythonScript::GetWrappedQtModule());
//}

PyObject* GetUiSceneManager(PyObject *self)
{
    boost::shared_ptr<UiServices::UiModule> ui_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

    // If this occurs, we're most probably operating in headless mode.
    if (ui_module.get() == 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "UiModule is missing."); //XXX perhaps should not be an error, 'cause some things should just work in headless without complaining
        return NULL;
    }

    return PythonQt::self()->wrapQObject(ui_module->GetSceneManager());
}

PyObject* GetUIView(PyObject *self)
{
    return PythonQt::self()->wrapQObject(PythonScript::self()->GetFramework()->GetUIView());
}

PyObject* GetServerConnection(PyObject *self)
{
	RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
		return PythonQt::self()->wrapQObject(rexlogic_->GetServerConnection().get());
	PyErr_SetString(PyExc_RuntimeError, "RexLogic is missing.");
	return NULL;
}

PyObject* SendObjectAddPacket(PyObject *self, PyObject *args)
{
	RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
		float start_x, start_y, start_z;
		float end_x, end_y, end_z;

		if(!PyArg_ParseTuple(args, "ffffff", &start_x, &start_y, &start_z, &end_x, &end_y, &end_z)) {
			PyErr_SetString(PyExc_ValueError, "Value error, need x1, y1, z1, x2, y2 and z2 params");
            return NULL;   
		}

		rexlogic_->GetServerConnection()->SendObjectAddPacket(Vector3df(start_x, start_y, start_z));
	}
	Py_RETURN_NONE;
}

PyObject* SendRexPrimData(PyObject *self, PyObject *args)
{
    std::cout << "Sending rexprimdata" << std::endl;
    
	RexLogic::RexLogicModule *rexlogic_;
    
    /*rexviewer_EntityObject* py_ent;
    if(!PyArg_ParseTuple(args, "O!", rexviewer_EntityType, &py_ent))
    {
        return NULL;   
    }*/

    unsigned int ent_id_int;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity failed, param should be an integer.");
        return NULL;   
    }

    ent_id = (entity_id_t) ent_id_int;

    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        rexlogic_->SendRexPrimData(ent_id); //py_ent->ent_id);
	}

	Py_RETURN_NONE;
}

PyObject* DeleteObject(PyObject *self, PyObject *args)
{
	RexLogic::RexLogicModule *rexlogic_;

    unsigned int ent_id_int;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity failed, param should be an integer.");
        return NULL;   
    }

    ent_id = (entity_id_t) ent_id_int;

    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        rexlogic_->GetServerConnection()->SendObjectDeletePacket(ent_id);
	}

	Py_RETURN_NONE;
}

PyObject* GetTrashFolderId(PyObject* self, PyObject* args)
{
	RexLogic::RexLogicModule *rexlogic_;
	rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
	ProtocolUtilities::InventoryFolderSkeleton *folder = rexlogic_->GetInventory()->GetFirstChildFolderByName("Trash");
	if (folder)
		return Py_BuildValue("s", folder->id.ToString().c_str());
	return NULL;
}

PyObject* GetUserAvatarId(PyObject* self)
{
	RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        entity_id_t id = rexlogic_->GetUserAvatarId();
        return Py_BuildValue("I", id);
    }

	Py_RETURN_NONE;
}

PyObject* GetCameraUp(PyObject *self) 
{
    Vector3df up;
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        up = rexlogic_->GetCameraUp();
        return Py_BuildValue("fff", up.x, up.y, up.z);
    }
    Py_RETURN_NONE;
}

PyObject* GetCameraRight(PyObject *self) 
{
    Vector3df right;
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        right = rexlogic_->GetCameraRight();
        return Py_BuildValue("fff", right.x, right.y, right.z);
    }
    Py_RETURN_NONE;
}

PyObject* GetCameraFOV(PyObject *self) 
{
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        float fovy = rexlogic_->GetCameraFOV();
        return Py_BuildValue("f", fovy);
    }
    Py_RETURN_NONE;
}

PyObject* GetCameraPosition(PyObject *self) 
{
	Vector3df pos;
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        pos = rexlogic_->GetCameraPosition();
        return Py_BuildValue("fff", pos.x, pos.y, pos.z);
    }
    Py_RETURN_NONE;
}

PyObject* GetScreenSize(PyObject *self) 
{
    RexLogic::RexLogicModule *rexlogic_;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(PythonScript::self()->GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        float width = rexlogic_->GetCameraViewportWidth();
		float height = rexlogic_->GetCameraViewportHeight();
        return Py_BuildValue("ff", width, height);
    }
    Py_RETURN_NONE;
}
		
//slider input
/*    UpdateSliderEvents(input_state_);
    UpdateSliderEvents(Input::State_All);*/

PyObject* NetworkUpdate(PyObject *self, PyObject *args)
{   
	//PythonScript::self()->LogDebug("NetworkUpdate");
    unsigned int ent_id_int;
    entity_id_t ent_id;

    if(!PyArg_ParseTuple(args, "I", &ent_id_int))
    {
        PyErr_SetString(PyExc_ValueError, "Getting an entity failed, param should be an integer.");
        return NULL;   
    }

    ent_id = (entity_id_t) ent_id_int;
    
    Scene::ScenePtr scene = PythonScript::GetScene();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return NULL;
    }

    Scene::EntityPtr entity = scene->GetEntity(ent_id);
    Scene::Events::SceneEventData event_data(ent_id);
    event_data.entity_ptr_list.push_back(entity);
    PythonScript::self()->GetFramework()->GetEventManager()->SendEvent(PythonScript::scene_event_category_, Scene::Events::EVENT_ENTITY_UPDATED, &event_data);
	
    Py_RETURN_NONE;
}

//XXX \todo make Login a QObject and the other login methods slots so they are all exposed
PyObject* StartLoginOpensim(PyObject *self, PyObject *args)
{
    const char* firstAndLast;
    const char* password;
    const char* serverAddressWithPort;

    QString qfirstAndLast;
    QString qpassword;
    QString qserverAddressWithPort;

    Foundation::Framework *framework_;
    RexLogic::RexLogicModule *rexlogic_;
 
    if(!PyArg_ParseTuple(args, "sss", &firstAndLast, &password, &serverAddressWithPort))
    {
        PyErr_SetString(PyExc_ValueError, "Opensim login requires three params: User Name, password, server:port");
        return NULL;
    }
    
    qfirstAndLast = QString(firstAndLast);
    qpassword = QString(password);
    qserverAddressWithPort = QString(serverAddressWithPort);

    framework_ = PythonScript::self()->GetFramework();//PythonScript::staticframework;
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    rexlogic_->StartLoginOpensim(qfirstAndLast, qpassword, qserverAddressWithPort);
    //boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    //if (renderer){

    Py_RETURN_NONE;
}

PyObject *Exit(PyObject *self, PyObject *null)
{
    PythonScript::self()->GetFramework()->Exit();
    Py_RETURN_NONE;
}

PyObject* Logout(PyObject *self)
{
    Foundation::Framework *framework_;
    RexLogic::RexLogicModule *rexlogic_;

    framework_ = PythonScript::self()->GetFramework();
    rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    rexlogic_->LogoutAndDeleteWorld();

    Py_RETURN_NONE;
}

/*
PyObject* PyEventCallback(PyObject *self, PyObject *args){
    std::cout << "PyEventCallback" << std::endl;
    const char* key;
    const char* message;
    if(!PyArg_ParseTuple(args, "ss", &key, &message))
        Py_RETURN_FALSE; //XXX should raise an exception but that'd require refactor in the comms py backend
    std::cout << key << std::endl;
    std::cout << message << std::endl;
    std::string k(key);
    std::string m(message);
    PythonScript::PythonScriptModule::engineAccess->NotifyScriptEvent(k, m);
    Py_RETURN_TRUE;
}
*/
PyObject* RandomTest(PyObject* self, PyObject* args)
{
	/*
	unsigned int starter;
    ProtocolUtilities::NetMsgID id;

    if(!PyArg_ParseTuple(args, "I", &starter))
    {
        PyErr_SetString(PyExc_ValueError, "param should be an integer.");
        return NULL;   
    }

    id = (ProtocolUtilities::NetMsgID) starter;

	if (id == 0xffff0087)
		PythonScript::self()->LogDebug("Test successfull!!");
	*/
	//QMap<QString, int> *map = new QMap<QString, int>;
	QEC_Prim* prim = new QEC_Prim();
	
	prim->map["one"] = 1;
	prim->map["three"] = 3;
	
	prim->setName("This is a Name!");
	RexUUID uuid = RexUUID::CreateRandom();
	//QString qstr(uuid.ToString());
	//QVariant(
	prim->setUUID(QString(uuid.ToString().c_str()));
	QApplication* qapp = PythonScript::self()->GetFramework()->GetQApplication();
	PropertyEditor::PropertyEditor* pe = new PropertyEditor::PropertyEditor(qapp);
	pe->setObject(prim);
	pe->show();
	
	//return PythonQt::self()->wrapQObject(*map);
	Py_RETURN_NONE;
}
// XXX NOTE: there apparently is a way to expose bound c++ methods? 
// http://mail.python.org/pipermail/python-list/2004-September/282436.html
static PyMethodDef EmbMethods[] = {
    {"sendChat", (PyCFunction)SendChat, METH_VARARGS,
    "Send the given text as an in-world chat message."},

    {"randomTest", (PyCFunction)RandomTest, METH_VARARGS,
    "Random test function."},

    {"getEntity", (PyCFunction)GetEntity, METH_VARARGS,
    "Gets the entity with the given ID."},

    {"getQPrim", (PyCFunction)GetQPrim, METH_VARARGS,
    "Gets the prim component as a QObject from an entity with the given ID."},

    {"getQPlaceable", (PyCFunction)GetQPlaceable, METH_VARARGS,
    "Gets the placeable component as a QObject from an entity with the given ID."},

    {"getEntityByUUID", (PyCFunction)GetEntityByUUID, METH_VARARGS,
    "Gets the entity with the given UUID."},

    {"createEntity", (PyCFunction)CreateEntity, METH_VARARGS,
    "Creates a new entity with the given ID, and returns it."},

    {"getCameraYawPitch", (PyCFunction)GetCameraYawPitch, METH_VARARGS,
    "Returns the camera yaw and pitch."},

    {"setCameraYawPitch", (PyCFunction)SetCameraYawPitch, METH_VARARGS,
    "Sets the camera yaw and pitch."},

    {"setAvatarYaw", (PyCFunction)SetAvatarYaw, METH_VARARGS,
    "Changes the avatar yaw with the given amount. Keys left/right are -1/+1."},    

    {"rayCast", (PyCFunction)RayCast, METH_VARARGS,
    "RayCasting from camera to point (x,y)."},

    {"switchCameraState", (PyCFunction)SwitchCameraState, METH_VARARGS,
    "Switching the camera mode from free to thirdperson and back again."},

    {"setAvatarRotation", (PyCFunction)SetAvatarRotation, METH_VARARGS,
    "Rotating the avatar."},

    {"sendEvent", (PyCFunction)SendEvent, METH_VARARGS,
    "Send an event id (WIP other stuff)."},

    {"takeScreenshot", (PyCFunction)TakeScreenshot, METH_VARARGS,
    "Takes a screenshot and saves it to a timestamped file."},

    {"logInfo", (PyCFunction)PyLogInfo, METH_VARARGS,
    "Prints a text using the LogInfo-method."},

	{"logDebug", (PyCFunction)PyLogDebug, METH_VARARGS,
    "Prints a debug text using the LogDebug-method."},

    {"getCameraRight", (PyCFunction)GetCameraRight, METH_VARARGS, 
    "Get the right-vector for the camera."},
    
    {"getCameraUp", (PyCFunction)GetCameraUp, METH_VARARGS, 
    "Get the up-vector for the camera."},
	
	{"getScreenSize", (PyCFunction)GetScreenSize, METH_VARARGS, 
    "Get the size of the screen."},

    {"getCameraFOV", (PyCFunction)GetCameraFOV, METH_VARARGS, 
    "Get the Field of View from the camera."},

    {"getCameraPosition", (PyCFunction)GetCameraPosition, METH_VARARGS, 
    "Get the position of the camera."},

    //from RexPythonQt.cpp now .. except got the fricken staticframework == null prob!

    //{"createCanvas", (PyCFunction)CreateCanvas, METH_VARARGS, 
    //"Create a new Qt canvas within the viewer"},

    //{"closeAndDeleteCanvas", (PyCFunction)CloseAndDeleteCanvas, METH_VARARGS, 
    //"closes and deletes the given canvas"},

    //{"getQtModule", (PyCFunction)GetQtModule, METH_NOARGS, 
    //"gets the qt module"},

    {"getUiSceneManager", (PyCFunction)GetUiSceneManager, METH_NOARGS, 
    "Gets the Naali-Qt UI scene manager"},

    {"getUiView", (PyCFunction)GetUIView, METH_NOARGS, 
    "Gets the Naali-Qt UI main view"},

    {"sendObjectAddPacket", (PyCFunction)SendObjectAddPacket, METH_VARARGS, 
    "Creates a new prim at the given points"},

    {"sendRexPrimData", (PyCFunction)SendRexPrimData, METH_VARARGS,
    "updates prim data to the server - now for applying a mesh to an object"},

	{"getUserAvatarId", (PyCFunction)GetUserAvatarId, METH_VARARGS, 
    "Returns the user's avatar's id."},

    {"networkUpdate", (PyCFunction)NetworkUpdate, METH_VARARGS, 
    "Does a network update for the Scene."},

    {"startLoginOpensim", (PyCFunction)StartLoginOpensim, METH_VARARGS,
    "Starts login using OpenSim authentication: expects User Name, password, server:port"},

    {"logout", (PyCFunction)Logout, METH_NOARGS,
    "Log out from the world. Made for test script to be able to stop."},

    {"exit", (PyCFunction)Exit, METH_NOARGS,
    "Exits viewer. Takes no arguments."},

    {"getServerConnection", (PyCFunction)GetServerConnection, METH_NOARGS,
    "Gets the server connection."},

    {"getPropertyEditor", (PyCFunction)GetPropertyEditor, METH_VARARGS, 
    "get property editor"},

    {"deleteObject", (PyCFunction)DeleteObject, METH_VARARGS, 
    "deletes an object (not used)"},

    {"getTrashFolderId", (PyCFunction)GetTrashFolderId, METH_VARARGS, 
    "gets the trash folder id"},

    {"createUiWidgetProperty", (PyCFunction)CreateUiWidgetProperty, METH_VARARGS, 
    "creates a new UiWidgetProperty"},
	
    {"createUiProxyWidget", (PyCFunction)CreateUiProxyWidget, METH_VARARGS, 
    "creates a new UiProxyWidget"},

//    {"getEntityMatindicesWithTexture", (PyCFunction)GetEntityMatindicesWithTexture, METH_VARARGS, 
//    "Finds all entities with material indices which are using the given texture"},

    //{"applyUICanvasToSubmeshesWithTexture", (PyCFunction)ApplyUICanvasToSubmeshesWithTexture, METH_VARARGS, 
    //"Applies a ui canvas to all the entity submeshes where the given texture is used. Parameters: uicanvas (internal mode required), textureuuid"},

    //{"applyUICanvasToSubmeshes", (PyCFunction)ApplyUICanvasToSubmeshes, METH_VARARGS, 
    //"Applies a ui canvas to the given submeshes of the entity. Parameters: entity id, list of submeshes (material indices), uicanvas (internal mode required)"},
    
    {"getSubmeshesWithTexture", (PyCFunction)GetSubmeshesWithTexture, METH_VARARGS, 
    "Find the submeshes in this entity that use the given texture, if any. Parameters: entity id, texture uuid"},
    
	{NULL, NULL, 0, NULL}
};

static PyObject* PythonScript::initpymod()
{
    PyObject* m;
    
    m = Py_InitModule("rexviewer", EmbMethods);

    //event constants are now put in PostInit so that the other modules have registered theirs already.
    //XXX what about new event types defined in py-written modules?

    entity_init(m); 
    /* this is planned to be vice versa: 
       the implementing modules, like here scene§ for Entity,
       would call something here to get a ref to the module, or something?
    */

    return m;
}

/* this belongs to Entity.cpp but when added to the api from there, the staticframework is always null */
PyObject* PythonScript::entity_getattro(PyObject *self, PyObject *name)
{
    PyObject* tmp;

    if (!(tmp = PyObject_GenericGetAttr((PyObject*)self, name))) {
        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
        {
            //std::cout << "..attribute error" << std::endl;
            return NULL;
        }

        PyErr_Clear();
    }
    else
        return tmp;

    const char* c_name = PyString_AsString(name);
    std::string s_name = std::string(c_name);

    //std::cout << "Entity: getting unknown attribute: " << s_name;
    
    //entity_ptrs map usage
    /* this crashes now in boost, 
       void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
    std::map<entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
    Scene::EntityPtr entity = ep_iter->second;
    fix.. */

    /* re-getting the EntityPtr as it wasn't stored anywhere yet,
       is copy-paste from PythonScriptModule GetEntity 
       but to be removed when that map is used above.*/
    Scene::ScenePtr scene = PythonScript::GetScene();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return NULL;
    }

    rexviewer_EntityObject *eob = (rexviewer_EntityObject *)self;
    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
    RexLogic::EC_OpenSimPrim *prim = 0;
    if (prim_component)
	prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());  
	
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
    OgreRenderer::EC_OgrePlaceable *placeable = 0;
    if (ogre_component)
	placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
    
    if (s_name.compare("id") == 0)
    {
        return Py_BuildValue("I", eob->ent_id); //unsigned int - is verified to be correct, same as c++ shows (at least in GetEntity debug print)
    }
    else if (s_name.compare("prim") == 0)
    {
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }  
        //m->AddU32(prim->LocalId);
        std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
        return PyString_FromString(retstr.c_str());
    }
    else if (s_name.compare("name") == 0)
    {
        //std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }
        return PyString_FromString(prim->ObjectName.c_str());
    }

    else if (s_name.compare("mesh") == 0)
	{
        //std::cout << ".. getting prim in mesh getting" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }  
        return PyString_FromString(prim->MeshID.c_str());	
        
        /* was a test thing, just changes what ogre shows locally
        Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
		Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
		if (placeable)
		{
			OgreRenderer::EC_OgreMesh &ogremesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
			
			std::string text = ogremesh.GetMeshName();
			return PyString_FromString(text.c_str());
		}*/
    }
	else if(s_name.compare("uuid") == 0)
	{
		//std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }
		return PyString_FromString(prim->FullId.ToString().c_str());
	}
	else if(s_name.compare("updateflags") == 0)
	{
		//std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }
		return Py_BuildValue("I", prim->UpdateFlags);
	}
	else if(s_name.compare("editable") == 0)
	{
		// refactor to take into account permissions etc aswell later?
		if(!placeable)
			Py_RETURN_FALSE;
		else
			Py_RETURN_TRUE;
	}

    else if (s_name.compare("pos") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }       
		//std::cout << placeable->GetSelectPriority() << std::endl;
        /* this must probably return a new object, a 'Place' instance, that has these.
           or do we wanna hide the E-C system in the api and have these directly on entity? 
           probably not a good idea to hide the actual system that much. or? */
        Vector3df pos = placeable->GetPosition();
        //RexTypes::Vector3 scale = ogre_pos->GetScale();
        //RexTypes::Vector3 rot = PackQuaternionToFloat3(ogre_pos->GetOrientation());
        /* .. i guess best to wrap the Rex Vector and other types soon,
           the pyrr irrlicht binding project does it for these using swig,
           https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i 
	Now am experimenting with the new QVector3D type, see GetQPlaceable*/
        return Py_BuildValue("fff", pos.x, pos.y, pos.z);
    }

    else if (s_name.compare("scale") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }     
		Vector3df scale = placeable->GetScale();

        return Py_BuildValue("fff", scale.x, scale.y, scale.z);
    }

    else if (s_name.compare("orientation") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }         
        
        Quaternion orient = placeable->GetOrientation();
        return Py_BuildValue("ffff", orient.x, orient.y, orient.z, orient.w);
    }

    else if (s_name.compare("text") == 0)
    {
        const Foundation::ComponentInterfacePtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());

        if (!overlay)
        {
            PyErr_SetString(PyExc_AttributeError, "overlay not found.");
            return NULL;   
        }  
        OgreRenderer::EC_OgreMovableTextOverlay *name_overlay = checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay *>(overlay.get());
        std::string text = name_overlay->GetText();
        return PyString_FromString(text.c_str());
    }
	else if (s_name.compare("boundingbox") == 0)
	{
		if (!placeable)
		{
			PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;  
		}
		Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
		if (meshptr)
		{
			OgreRenderer::EC_OgreCustomObject& cobj = *checked_static_cast<OgreRenderer::EC_OgreCustomObject*>(meshptr.get());
			Vector3df min, max;

			cobj.GetBoundingBox(min, max);

			return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 0);
		}
		else
		{
			meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
			if (meshptr) 
			{
				OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
				Vector3df min, max;

				mesh.GetBoundingBox(min, max);

				return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 1);
			}
		}
		
		PyErr_SetString(PyExc_AttributeError, "getting the bb failed.");
        return NULL;  
	}


    std::cout << "unknown component type."  << std::endl;
    return NULL;
}

int PythonScript::entity_setattro(PyObject *self, PyObject *name, PyObject *value)
{
    /*
    if (!(tmp = PyObject_GenericSetAttr((PyObject*)self, name, value))) {
        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
            return NULL;
        PyErr_Clear();
    }*/

    const char* c_name = PyString_AsString(name);
    std::string s_name = std::string(c_name);

    //std::cout << "Entity: setting unknown attribute: " << s_name;
    rexviewer_EntityObject *eob = (rexviewer_EntityObject *)self;

    //entity_ptrs map usage
    /* this crashes now in boost, 
       void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
    std::map<entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
    Scene::EntityPtr entity = ep_iter->second;
    fix.. */

    /* re-getting the EntityPtr as it wasn't stored anywhere yet,
       is copy-paste from PythonScriptModule GetEntity 
       but to be removed when that map is used above.*/
    Scene::ScenePtr scene = PythonScript::GetScene();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return -1;
    }

    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
	RexLogic::EC_OpenSimPrim *prim = 0;
    if (prim_component)
	   prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());  
	
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
	OgreRenderer::EC_OgrePlaceable *placeable = 0;
    if (ogre_component)
		placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
    
    RexLogic::EC_NetworkPosition* networkpos = dynamic_cast<RexLogic::EC_NetworkPosition*>(entity->GetComponent(RexLogic::EC_NetworkPosition::NameStatic()).get());
    
    /*if (s_name.compare("prim") == 0)
    {
        std::cout << ".. getting prim" << std::endl;
        const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
        if (!prim_component)
            return NULL; //XXX report AttributeError
        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
        //m->AddU32(prim->LocalId);
        std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
        return PyString_FromString(retstr.c_str());
    }*/
	//PyObject_Print(value, stdout, 0);
	//std::cout << "\n" << std::endl;
    //else 
    if (s_name.compare("pos") == 0)
    {
        /* this must probably return a new object, a 'Place' instance, that has these.
           or do we wanna hide the E-C system in the api and have these directly on entity? 
           probably not a good idea to hide the actual system that much. or? */
        float x, y, z;
		x = 0;
		y = 0;
		z = 0;

		//int parsing;
		//parsing = PyArg_ParseTuple(value, "fff", &x, &y, &z);
		/*
		PyObject* pos;
		if(!parsing) {
			parsing = PyArg_ParseTuple(value, "O", &pos);
			if (parsing != 0)
			{
				PyErr_SetString(PyExc_ValueError, "it worked.");
				return -1;
			}
		}
		*/
        if(!PyArg_ParseTuple(value, "fff", &x, &y, &z))
        {	
            //std::cout << "...parse error" << std::endl;
            //PyErr_SetString(PyExc_ValueError, "params should be: (float, float, float).");
            return -1;
        }
		
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;
        }  

        // Set the new values.
        placeable->SetPosition(Vector3df(x, y, z));
        if (networkpos)
        {
            // Override the dead reckoning system
            networkpos->SetPosition(placeable->GetPosition());
        }
            
        //ogre_pos->SetScale(OpenSimToOgreCoordinateAxes(scale));
        //ogre_pos->SetOrientation(OpenSimToOgreQuaternion(quat));
        /* .. i guess best to wrap the Rex Vector and other types soon,
           the pyrr irrlicht binding project does it for these using swig,
           https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */

        /* sending a scene updated event to trigger network synch,
           copy-paste from DebugStats, 
           perhaps there'll be some MoveEntity thing in logic that can reuse for this? */
        return 0; //success.
    }

    else if (s_name.compare("scale") == 0)
    {
        float x, y, z;
		x = 0;
		y = 0;
		z = 0;
        if(!PyArg_ParseTuple(value, "fff", &x, &y, &z))
        {
            //PyErr_SetString(PyExc_ValueError, "params should be: (float, float, float)");
            return -1;   
        }

        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;   
        }  
        // Set the new values.
        placeable->SetScale(Vector3df(x, y, z));
 
        return 0; //success.
    }
    
    else if (s_name.compare("orientation") == 0)
    {
        float x, y, z, w;
        if(!PyArg_ParseTuple(value, "ffff", &x, &y, &z, &w))
        {
            PyErr_SetString(PyExc_ValueError, "params should be (float, float, float, float)"); //XXX change the exception
            return NULL;   
        }
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;   
        }          
        // Set the new values.
        placeable->SetOrientation(Quaternion(x, y, z, w));
        if (networkpos)
        {
            // Override the dead reckoning system
            networkpos->SetOrientation(placeable->GetOrientation());
        }
                    
        return 0; //success.
    }

    else if (s_name.compare("text") == 0)
    {
        if (PyString_Check(value) || PyUnicode_Check(value)) 
        {
            const Foundation::ComponentPtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
            const char* c_text = PyString_AsString(value);
            std::string text = std::string(c_text);
            if (overlay)
            {
                OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
                name_overlay.SetText(text);
                //name_overlay.SetPlaceable(placeable); //is this actually needed for something?
            }
            else //xxx
            {
                PyErr_SetString(PyExc_ValueError, "overlay not found."); //XXX change the exception
                return -1;   
            }
        
        }
        else
        {
            PyErr_SetString(PyExc_ValueError, "text is a string"); //XXX change the exception
            return -1;
        }
        
        //if(!PyArg_ParseTuple(value, "s", c_text))
        //    return NULL; //XXX report ArgumentException error
                


        return 0;
    }
	else if (s_name.compare("mesh") == 0)
	{
	    //std::cout << "Setting mesh" << std::endl;
		if (PyString_Check(value) || PyUnicode_Check(value))
        {
			//NOTE: This is stricly done locally only for now, nothing is sent to the server.
			const char* c_text = PyString_AsString(value);
			std::string text = std::string(c_text);

            //std::cout << ".. getting prim in mesh setting" << std::endl;
			if (!prim)
			{
				PyErr_SetString(PyExc_AttributeError, "prim not found.");
				return -1;   
			}  

            prim->MeshID = text;
            prim->DrawType = RexTypes::DRAWTYPE_MESH;

            return 0;
        }
        
        /* was a test thing, just changes what ogre shows locally

			
            /*Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
			Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
			if (placeable)
			{
				OgreRenderer::EC_OgreMesh &ogremesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
				
				ogremesh.SetMesh(text);

				PythonScript::self()->LogInfo("Entity's mesh changed locally.");
				return NULL;
			}*/

        else
        {
            PyErr_SetString(PyExc_ValueError, "Mesh asset id is expected as a string"); //XXX change the exception
            return -1;
        }
	}
	
    //XXX why does this even exist when uuid is not settable?
	else if(s_name.compare("uuid") == 0)
	{
        PythonScript::self()->LogInfo("UUID cannot be set manually.");
        return 0;   
	}

    //std::cout << "unknown component type."  << std::endl;
	PythonScript::self()->LogDebug("Unknown component type.");
    return -1; //the way for setattr to report a failure
}
