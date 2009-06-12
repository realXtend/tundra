// For conditions of distribution and use, see copyright notice in license.txt


#include "OISKeyboard.h"
//#include "OISMouse.h"

#include "StableHeaders.h"
#include "PythonScriptModule.h"
//#include "Foundation.h"
#include "ServiceManager.h"
#include "ComponentRegistrarInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "PythonEngine.h" //is this needed here?

#include "RexLogicModule.h" //now for SendChat, perhaps other stuff for the api will be here too?
#include "OpenSimProtocolModule.h" //for handling net events
#include "RexProtocolMsgIDs.h"
#include "InputEvents.h" //handling input events
#include "InputServiceInterface.h" //for getting mouse info from the input service

#include "SceneManager.h"
#include "SceneEvents.h" //sending scene events after (placeable component) manipulation

#include "Entity.h"

//for CreateEntity. to move to an own file (after the possible prob with having api code in diff files is solved)
//#include "../OgreRenderingModule/EC_OgreMesh.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
//#include "../OgreRenderingModule/EC_OgreMovableTextOverlay.h"
#include "RexNetworkUtils.h" //debugboundingbox in CreateEntity


namespace PythonScript
{
	Foundation::ScriptEventInterface* PythonScriptModule::engineAccess;// for reaching engine from static method

	PythonScriptModule::PythonScriptModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    PythonScriptModule::~PythonScriptModule()
    {
    }

    // virtual
    void PythonScriptModule::Load()
    {
        using namespace PythonScript;

        LogInfo("Module " + Name() + " loaded.");
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
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void PythonScriptModule::Initialize()
    {
		engine_ = PythonScript::PythonEnginePtr(new PythonScript::PythonEngine(framework_));
        engine_->Initialize();
		
		PythonScriptModule::engineAccess = dynamic_cast<Foundation::ScriptEventInterface*>(engine_.get());
        
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Scripting, engine_);

		//XXX hack to have a ref to framework for api funcs
		PythonScript::staticframework = framework_;
		apiModule = PythonScript::initpymod(); //initializes the rexviewer module to be imported within py

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

        LogInfo("Module " + Name() + " initialized succesfully.");
    }

    void PythonScriptModule::PostInitialize()
    {
		Foundation::EventManagerPtr em = framework_->GetEventManager();        

		/* get the category IDs of event types that pass on to the modulemanager */
		//(was) for onChat
		inboundCategoryID_ = em->QueryEventCategory("OpenSimNetworkIn");
        if (inboundCategoryID_ == 0)
            LogWarning("Unable to find event category for incoming OpenSimNetwork events!");

		//for notifying placeable movement
		PythonScript::scene_event_category_ = em->QueryEventCategory("Scene");
	    if (PythonScript::scene_event_category_ == 0)
			LogWarning("Unable to find event category for Scene events!");

		inputeventcategoryid = em->QueryEventCategory("Input");
        if (inputeventcategoryid == 0)
            LogError("Unable to find event category for Input");

		/* add events constants - now just the input events */
		//XXX move these to some submodule ('input'? .. better than 'constants'?)
		/*PyModule_AddIntConstant(apiModule, "MOVE_FORWARD_PRESSED", Input::Events::MOVE_FORWARD_PRESSED);
		PyModule_AddIntConstant(apiModule, "MOVE_FORWARD_RELEASED", Input::Events::MOVE_FORWARD_RELEASED);
		LogInfo("Added event constants.");*/

		/* TODO: add other categories and expose the hierarchy as py submodules or something,
		add registrating those (it's not (currently) mandatory),
		to the modules themselves, e.g. InputModule (currently the OIS thing but that is to change) */
		const Foundation::EventManager::EventMap &evmap = em->GetEventMap();
		Foundation::EventManager::EventMap::const_iterator cat_iter = evmap.find(inputeventcategoryid);
		if (cat_iter != evmap.end())
		{
			std::map<Core::event_id_t, std::string> evs = cat_iter->second;
			for (std::map<Core::event_id_t, std::string>::iterator ev_iter = evs.begin();
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
		{
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

    bool PythonScriptModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
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

				PyObject* value = PyObject_CallMethod(pmmInstance, "KEY_INPUT_EVENT", "iii", event_id, keycode, mods);

				if (PyObject_IsTrue(value))
				{
					//LogInfo("KEY_INPUT_EVENT returned true.");
					return true;  
				} 
				else 
				{
					//LogInfo("KEY_INPUT_EVENT returned false.");
					return false;
				}

			}
			else
			{
				PyObject_CallMethod(pmmInstance, "INPUT_EVENT", "i", event_id);
			}
		}
		
		//was for first receive chat test, when no module provided it, so handles net event directly
		/* got a crash with this now during login, when the viewer was also getting asset data etc.
		   disabling the direct reading of network data here now to be on the safe side,
		   this has always behaved correctly till now though (since march). --antont june 12th 
        if (category_id == inboundCategoryID_)
        {
            OpenSimProtocol::NetworkEventInboundData *event_data = static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            const NetMsgID msgID = event_data->messageID;
            NetInMessage *msg = event_data->message;
            const NetMessageInfo *info = event_data->message->GetMessageInfo();
            assert(info);
            
            std::stringstream ss;
            ss << info->name << " received, " << Core::ToString(msg->GetDataSize()) << " bytes.";
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
				
	            //ss << "[" << Core::GetLocalTimeString() << "] " << name << ": " << message << std::endl;
				//LogInfo(ss.str());
	            //WriteToChatWindow(ss.str());
				//can readbuffer ever return null? should be checked if yes. XXX

				PyObject_CallMethod(pmmInstance, "RexNetMsgChatFromSimulator", "ss", name, message);

	            break;
		        }
			}
		}*/

		return false;
	}

    Console::CommandResult PythonScriptModule::ConsoleRunString(const Core::StringVector &params)
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

    Console::CommandResult PythonScriptModule::ConsoleRunFile(const Core::StringVector &params)
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

	Console::CommandResult PythonScriptModule::ConsoleReset(const Core::StringVector &params)
	{
		engine_->Reset();
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
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void PythonScriptModule::Update(Core::f64 frametime)
    {
		//XXX remove when/as the core has the fps limitter
		engine_->RunString("import time; time.sleep(0.01);"); //a hack to save cpu now.

		if (pmmInstance != NULL)
			PyObject_CallMethod(pmmInstance, "run", "f", frametime);
		/*char** args = new char*[2]; //is this 2 'cause the latter terminates?
		std::string methodname = "run";
		std::string paramtypes = "f";
		modulemanager->CallMethod2(methodname, paramtypes, 0.05); //;frametime);
		*/		

		/* Mouse input special handling. InputModuleOIS has sending these as events commented out,
		   This polling is copy-pasted from the InputHandler in RexLogicModule */
        boost::shared_ptr<Input::InputServiceInterface> input = framework_->GetService<Input::InputServiceInterface>(Foundation::Service::ST_Input).lock();
        if (input)
        {
            boost::optional<const Input::Events::Movement&> movement = input->PollSlider(Input::Events::MOUSELOOK);
            if (movement)
            {
				//LogDebug("me sees mouse move too");

				//might perhaps wrap that nice pos class later but this is simpler now
				float x_abs = static_cast<float>(movement->x_.abs_);
				float y_abs = static_cast<float>(movement->y_.abs_);

				float x_rel = static_cast<float>(movement->x_.rel_);
				float y_rel = static_cast<float>(movement->y_.rel_);

				PyObject_CallMethod(pmmInstance, "MOUSE_INPUT", "ffff", x_abs, y_abs, x_rel, y_rel);

                //dragging_ = true;
                //state->Drag(&*movement);
            } /*else if (dragging_)
            {
                dragging_ = false;
                Input::Events::Movement zero;
                state->Drag(&zero);
            }*/
        }

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
static PyObject* SendChat(PyObject *self, PyObject *args)
{
	const char* msg;

	if(!PyArg_ParseTuple(args, "s", &msg))
		return NULL; //XXX raise ValueError

	//Foundation::Framework *framework_ = Foundation::ComponentInterfacePythonScriptModule::GetFramework();
	Foundation::Framework *framework_ = PythonScript::staticframework;
	//todo weak_pointerize

	//move decl to .h and getting to Initialize (see NetTEstLogicModule::Initialize)
	//if this kind of usage, i.e. getting the logic module for the api, is to remain.
	RexLogic::RexLogicModule *rexlogic_;
	rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

	rexlogic_->GetServerConnection()->SendChatFromViewerPacket(msg);

	Py_RETURN_TRUE;
}

//returns an Entity wrapper, is in actual use
static PyObject* GetEntity(PyObject *self, PyObject *args)
{
	unsigned int ent_id_int;
	Core::entity_id_t ent_id;

	if(!PyArg_ParseTuple(args, "i", &ent_id_int))
		return NULL; //XXX report ArgumentException error

	ent_id = (Core::entity_id_t) ent_id_int;

	Scene::ScenePtr scene = PythonScript::GetScene();

	if (scene == 0)
		return NULL; //XXX return some sensible exception info

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
		return NULL; //XXX TODO: raise ValueError
}

static PyObject* CreateEntity(PyObject *self, PyObject *args)
{
	Foundation::Framework *framework_ = PythonScript::staticframework;
	RexLogic::RexLogicModule *rexlogic_;

	rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

	unsigned int ent_id_int;
	Core::entity_id_t ent_id;

	if(!PyArg_ParseTuple(args, "i", &ent_id_int))
		return NULL; //XXX report ArgumentException error

	ent_id = (Core::entity_id_t) ent_id_int;

	Scene::ScenePtr scene = PythonScript::GetScene();
        
    if (!scene) //XXX enable the check || !rexlogicmodule_->GetFramework()->GetComponentManager()->CanCreate(OgreRenderer::EC_OgrePlaceable::NameStatic()))
		return NULL; //XXX return some sensible exception info
        
    Core::StringVector defaultcomponents;
    defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());
    //defaultcomponents.push_back(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
    //defaultcomponents.push_back(OgreRenderer::EC_OgreMesh::NameStatic());
    //defaultcomponents.push_back(OgreRenderer::EC_OgreAnimationController::NameStatic());
        
    Scene::EntityPtr entity = scene->CreateEntity(ent_id, defaultcomponents);

    Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    if (placeable)
    {
        OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());

        //DebugCreateOgreBoundingBox(rexlogic_,
        //    entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()), "AmbientGreen", Vector3(0.5,0.5,1.5));
            
        /* CreateNameOverlay(ogrepos, entityid);
		        Foundation::ComponentPtr overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
        EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent(EC_OpenSimAvatar::NameStatic()).get());
        if (overlay)
        {
            OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
            name_overlay.SetText(avatar.GetFullName());
            name_overlay.SetParentNode(placeable.GetSceneNode());
        }*/

        //CreateDefaultAvatarMesh(entityid);
        
        return entity_create(ent_id); //return the py wrapper for the new entity
    }

	return NULL; //XXX return some sensible exception info
}

static PyObject* PyEventCallback(PyObject *self, PyObject *args){
	std::cout << "PyEventCallback" << std::endl;
	const char* key;
	const char* message;
	if(!PyArg_ParseTuple(args, "ss", &key, &message))
		Py_RETURN_FALSE;
	std::cout << key << std::endl;
	std::cout << message << std::endl;
	std::string k(key);
	std::string m(message);
	PythonScript::PythonScriptModule::engineAccess->NotifyScriptEvent(k, m);
	Py_RETURN_TRUE;
}

static PyMethodDef EmbMethods[] = {
	{"sendChat", (PyCFunction)SendChat, METH_VARARGS,
	"Send the given text as an in-world chat message."},

	{"getEntity", (PyCFunction)GetEntity, METH_VARARGS,
	"Gets the entity with the given ID."},

	{"createEntity", (PyCFunction)CreateEntity, METH_VARARGS,
	"Creates a new entity with the given ID, and returns it."},

	{"pyEventCallback", (PyCFunction)PyEventCallback, METH_VARARGS,
	"Handling callbacks from py scripts. Calling convension: with 2 strings"},

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
	   the implementing modules, like here scene for Entity,
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
			return NULL;
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
	std::map<Core::entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
	Scene::EntityPtr entity = ep_iter->second;
	fix.. */

	/* re-getting the EntityPtr as it wasn't stored anywhere yet,
	   is copy-paste from PythonScriptModule GetEntity 
	   but to be removed when that map is used above.*/
	Scene::ScenePtr scene = PythonScript::GetScene();
	rexviewer_EntityObject *eob = (rexviewer_EntityObject *)self;
	Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);
	
	if (s_name.compare("prim") == 0)
	{
		std::cout << ".. getting prim" << std::endl;
		const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
		if (!prim_component)
			return NULL; //XXX report AttributeError
		RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
	        
		//m->AddU32(prim->LocalId);
		std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
		return PyString_FromString(retstr.c_str());
	}

	else if (s_name.compare("place") == 0)
	{
        const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
		if (!ogre_component)
            return NULL; //XXX report AttributeError        
		OgreRenderer::EC_OgrePlaceable *placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
		
		/* this must probably return a new object, a 'Place' instance, that has these.
		   or do we wanna hide the E-C system in the api and have these directly on entity? 
		   probably not a good idea to hide the actual system that much. or? */
		RexTypes::Vector3 pos = placeable->GetPosition();
		//RexTypes::Vector3 scale = ogre_pos->GetScale();
		//RexTypes::Vector3 rot = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
		/* .. i guess best to wrap the Rex Vector and other types soon,
		   the pyrr irrlicht binding project does it for these using swig,
		   https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */
		return Py_BuildValue("fff", pos.x, pos.y, pos.z);
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
	std::map<Core::entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
	Scene::EntityPtr entity = ep_iter->second;
	fix.. */

	/* re-getting the EntityPtr as it wasn't stored anywhere yet,
	   is copy-paste from PythonScriptModule GetEntity 
	   but to be removed when that map is used above.*/
	Scene::ScenePtr scene = PythonScript::GetScene();
	Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);
	
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

	//else 
	if (s_name.compare("place") == 0)
	{
        const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
		if (!ogre_component)
            return NULL; //XXX report AttributeError        
		OgreRenderer::EC_OgrePlaceable *placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
		
		/* this must probably return a new object, a 'Place' instance, that has these.
		   or do we wanna hide the E-C system in the api and have these directly on entity? 
		   probably not a good idea to hide the actual system that much. or? */
		float x, y, z;
		if(!PyArg_ParseTuple(value, "fff", &x, &y, &z))
			return NULL; //XXX report ArgumentException error
	    
		// Set the new values.
		placeable->SetPosition(Vector3(x, y, z));
	    //ogre_pos->SetScale(Core::OpenSimToOgreCoordinateAxes(scale));
		//ogre_pos->SetOrientation(Core::OpenSimToOgreQuaternion(quat));
		/* .. i guess best to wrap the Rex Vector and other types soon,
		   the pyrr irrlicht binding project does it for these using swig,
		   https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */

		/* sending a scene updated event to trigger network synch,
		   copy-paste from DebugStats, 
		   perhaps there'll be some MoveEntity thing in logic that can reuse for this? */
	    Scene::Events::SceneEventData event_data(eob->ent_id);
		event_data.entity_ptr_list.push_back(entity);
		PythonScript::staticframework->GetEventManager()->SendEvent(PythonScript::scene_event_category_, Scene::Events::EVENT_ENTITY_UPDATED, &event_data);

		return 0; //success.
	}

	std::cout << "unknown component type."  << std::endl;
	return -1; //the way for setattr to report a failure
}

