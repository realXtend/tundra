// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonScriptModule.h"
//#include <Poco/ClassLibrary.h>
//#include "Foundation.h"
#include "ServiceManager.h"
#include "ComponentRegistrarInterface.h"
#include "ConsoleCommandServiceInterface.h"


//testing receiving events, now from the net module 'cause nothing else sends yet
#include "RexProtocolMsgIDs.h"

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
		if (!Py_IsInitialized())
	        Py_Initialize();
		else
			LogWarning("Python already initialized in PythonScriptModule init!");

        LogInfo("Module " + Name() + " initialized.");
		//LogInfo("Py thinks 1 + 1 = " + Py_Eval("1 + 1"));
		//RunString("print 'Py thinks 1 + 1 = %d' % (1 + 1)");
    }

    void PythonScriptModule::PostInitialize()
    {
		inboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        if (inboundCategoryID_ == 0)
            LogWarning("Unable to find event category for incoming OpenSimNetwork events!");

		pName = PyString_FromString("chathandler");
		/* Error checking of pName left out */

		pModule = PyImport_Import(pName);
		Py_DECREF(pName);

		if (pModule != NULL) {
			pFunc = PyObject_GetAttrString(pModule, "onChat");
	        /* pFunc is a new reference */

			if (pFunc && PyCallable_Check(pFunc)) {
				LogInfo("Registered callback onChat from chathandler.py");
			}
			else {
				if (PyErr_Occurred())
					PyErr_Print();
				LogInfo("Cannot find function");
			}
		}
		else 
			LogInfo("chathandler.py not found");
	}

    bool PythonScriptModule::HandleEvent(
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
            //ss << info->name << " received, " << Core::ToString(msg->GetDataSize()) << " bytes.";
			//LogInfo(ss.str());

            switch(msgID)
		    {
		    case RexNetMsgChatFromSimulator:
		        {
	            /*std::stringstream ss;
	            size_t bytes_read;

	            std::string name = (const char *)msg->ReadBuffer(&bytes_read);
	            msg->SkipToFirstVariableByName("Message");
	            std::string message = (const char *)msg->ReadBuffer(&bytes_read);
	            ss << "[" << Core::GetLocalTimeString() << "] " << name << ": " << message << std::endl;

	            WriteToChatWindow(ss.str());*/

	            pArgs = PyTuple_New(1); //takes a single argument
				pValue = PyInt_FromLong(1); //..which is now just int 1
				/* pValue reference stolen here: */
				PyTuple_SetItem(pArgs, 0, pValue);

				pValue = PyObject_CallObject(pFunc, pArgs);
				Py_DECREF(pArgs);
				if (pValue != NULL) {
					printf("Result of call: %ld\n", PyInt_AsLong(pValue));
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
	}

	void PythonScriptModule::RunString(const char* codestr)
	{
		PyRun_SimpleString(codestr);
	}

	void PythonScriptModule::RunFile(const std::string &modulename)
	{
		/* could get a fp* here and pass it to
		int PyRun_SimpleFile(FILE *fp, const char *filename)
		but am unsure whether to use the Poco fs stuff for it an how
		so trying this, why not? we don't need the file on the c++ side?*/
		std::string cmd = "import " + modulename;
		RunString(cmd.c_str());
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
			RunString(params[0].c_str());
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
			RunFile(params[0]);
			return Console::ResultSuccess();
		}
	}

	Console::CommandResult PythonScriptModule::ConsoleReset(const Core::StringVector &params)
	{
		Reset();
		return Console::ResultSuccess();
	}	

    // virtual 
    void PythonScriptModule::Uninitialize()
    {        
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);

		Py_Finalize();

        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void PythonScriptModule::Update()
    {
        //renderer_->Update();
		RunString("import time; time.sleep(0.01)"); //a hack to save cpu now. didn't seem to help .. some other thread runs in a tight loop?
    }

	void PythonScriptModule::Reset()
	{
		/* should probably be module unload & load */
		Py_Finalize();
		Py_Initialize();
		LogInfo("Python interpreter reseted: all memory and state cleared.");
	}
}

using namespace PythonScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(PythonScriptModule)
POCO_END_MANIFEST

