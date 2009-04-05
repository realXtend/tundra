// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonScriptModule.h"
//#include <Poco/ClassLibrary.h>
//#include "Foundation.h"
#include "ServiceManager.h"
#include "ComponentRegistrarInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "PythonEngine.h" //is this needed here?


//testing receiving events, now from the net module 'cause nothing else sends yet
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h" //XXX for login hack


#include <Python/Python.h>

namespace PythonScript
{
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
        // for the hack that login directly from here XXX
		/*
		using namespace OpenSimProtocol;
		netInterface_ = dynamic_cast<OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol));
		*/
		engine_ = PythonScript::PythonEnginePtr(new PythonScript::PythonEngine(framework_));
        engine_->Initialize();
        
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Scripting, engine_.get());

		//XXX hack to have a ref to framework for api funcs
		PythonScript::staticframework = framework_;
		PythonScript::initpymod();

		//Py_InitModule("rexviewer", PythonScript::EmbMethods);
		/*PyObject *rexmodule = PyImport_AddModule("rexviewer");
		PyObject *chatfunc = PyMethod_New(&(PyCFunction)PythonScript::SendChat, NULL, NULL);
		PyModule_AddObject(rexmodule, "sendChat", chatfunc);*/

        LogInfo("Module " + Name() + " initialized.");
		//LogInfo("Py thinks 1 + 1 = " + Py_Eval("1 + 1"));
		//RunString("print 'Py thinks 1 + 1 = %d' % (1 + 1)");
    }

    void PythonScriptModule::PostInitialize()
    {
		/*
		inboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        if (inboundCategoryID_ == 0)
            LogWarning("Unable to find event category for incoming OpenSimNetwork events!");

		pName = PyString_FromString("chathandler");
		//Error checking of pName left out

		/* disabled nettestlogic so can't login via gui so copy-pasted the login procedure here
		LogInfo("Py module tries direct logging in");
		bool success = netInterface_->ConnectToRexServer("Python", "User",
            "test", "localhost", 9000);
            
        if(success)
        {
            myInfo_ = netInterface_->GetClientParameters();
                        
			NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgUseCircuitCode);
			assert(m);
			m->AddU32(myInfo_.circuitCode);
			m->AddUUID(myInfo_.sessionID);
			m->AddUUID(myInfo_.agentID);
			netInterface_->FinishMessageBuilding(m);

	        m = netInterface_->StartMessageBuilding(RexNetMsgCompleteAgentMovement);
			assert(m);
			m->AddUUID(myInfo_.agentID);
			m->AddUUID(myInfo_.sessionID);
			m->AddU32(myInfo_.circuitCode);
			netInterface_->FinishMessageBuilding(m);
            
            LogInfo("Connected to localhost.");
        }
        else
            LogError("Connecting to localhost failed.");
		*/
	}

    /*bool PythonScriptModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
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
				
	            ss << "[" << Core::GetLocalTimeString() << "] " << name << ": " << message << std::endl;
				LogInfo(ss.str());
	            //WriteToChatWindow(ss.str());
				//can readbuffer ever return null? should be checked if yes. XXX

	            pArgs = PyTuple_New(1); //takes a single argument
				//pValue = PyInt_FromLong(1); //..which is now just int 1
				pValue = PyString_FromString(message.c_str());
				//pValue reference stolen here:
				PyTuple_SetItem(pArgs, 0, pValue);

				pValue = PyObject_CallObject(pFunc, pArgs);
				Py_DECREF(pArgs);
				if (pValue != NULL) {
					//printf("Result of call: %ld\n", PyInt_AsLong(pValue));
					LogInfo("Python chathandler executed ok.");
					Py_DECREF(pValue);
				}
				else {
					Py_DECREF(pFunc);
					PyErr_Print();
					fprintf(stderr,"Call failed\n");
				}

	            break;
		        }
			}
		}

		return false;
	} */

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
		engine_->Uninitialize();
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void PythonScriptModule::Update(Core::f64 frametime)
    {
        //renderer_->Update();
		//XXX
		engine_->RunString("import time; time.sleep(0.01);"); //a hack to save cpu now. didn't seem to help .. some other thread runs in a tight loop?
    }

	//stuff for api, copy-paste from nettestlogic, to be moved to logic
	void PythonScriptModule::SendChatFromViewerPacket(const char *msg)
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgChatFromViewer);
		assert(m);
		m->AddUUID(myInfo_.agentID);
		m->AddUUID(myInfo_.sessionID);
		m->AddBuffer(strlen(msg), (uint8_t*)msg);
		m->AddU8(1);
		m->AddS32(0);
		netInterface_->FinishMessageBuilding(m);
	}
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
		return NULL;

	//Foundation::Framework *framework_ = Foundation::ComponentInterfacePythonScriptModule::GetFramework();
	Foundation::Framework *framework_ = PythonScript::staticframework;
	//todo weak_pointerize
    PythonScriptModule *pyModule_ = dynamic_cast<PythonScriptModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_PythonScript));

	pyModule_->SendChatFromViewerPacket(msg);

	//SendChatFromViewerPacket(msg);

	Py_RETURN_TRUE;
}

static PyMethodDef EmbMethods[] = {
	{"sendChat", (PyCFunction)SendChat, METH_VARARGS,
	"Send the given text as a chat message."},
	{NULL, NULL, 0, NULL}
};

static void PythonScript::initpymod()
{
	Py_InitModule("rexviewer", EmbMethods);
}
