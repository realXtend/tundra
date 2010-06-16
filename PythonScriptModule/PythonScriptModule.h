// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptingModule_h
#define incl_PythonScriptingModule_h

#ifdef PYTHON_FORCE_RELEASE_VERSION
  #ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
  #else
    #include <Python.h>
  #endif 
#else
    #include <Python.h>
#endif

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
#include "../OgreRenderingModule/Renderer.h" //for the screenshot api XXX add the path to includes, don't do this.
#include "SceneManager.h"

#include <PythonQt.h>
#include <QList>

class EC_OpenSimPrim;

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class RexLogicModule;
    class CameraControllable;
    class AvatarControllable;
}

namespace ProtocolUtilities
{
    class InventorySkeleton;
}

typedef boost::shared_ptr<ProtocolUtilities::InventorySkeleton> InventoryPtr;

namespace PythonScript
{
    class PythonEngine;
    typedef boost::shared_ptr<PythonEngine> PythonEnginePtr;

    //! A scripting module using Python
    class MODULE_API PythonScriptModule : public QObject, public Foundation::ModuleInterfaceImpl
    {
        Q_OBJECT

    public slots: //things for the py side to call.
        OgreRenderer::Renderer* GetRenderer();
        Scene::SceneManager* GetScene(QString name);
        void RunJavascriptString(QString codestr, QVariantMap context = QVariantMap());

    public:
        PythonScriptModule();
        virtual ~PythonScriptModule();

        //the module interface
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        //handling events
        virtual bool HandleEvent(
            event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! callback for console command        
        Console::CommandResult ConsoleRunString(const StringVector &params);
        Console::CommandResult ConsoleRunFile(const StringVector &params);
        Console::CommandResult ConsoleReset(const StringVector &params);

        // Subscribing to network categories
        void SubscribeToNetworkEvents();
        
        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        static void Add3DCanvasComponents(Scene::Entity *entity, QWidget *widget, QList<uint> submeshes, int refresh_rate);

        //Foundation::Framework* GetFramework() { return frameworkptr;  };//this still returns null or 0... WHY?
        //static Foundation::ScriptEventInterface* engineAccess;

        //api code is outside the module now, but reuses these .. err, but can't see 'cause dont have a ref to the instance?
        // Category id for incoming messages.
        event_category_id_t inboundCategoryID_;
        event_category_id_t inputeventcategoryid;
        event_category_id_t networkstate_category_id;
        event_category_id_t framework_category_id;
        event_category_id_t scene_event_category_ ;

        /// Returns the currently initialized PythonScriptModule.
        static PythonScriptModule *GetInstance();

        Scene::ScenePtr GetScenePtr() { return framework_->GetDefaultWorldScene(); }
        PyObject* WrapQObject(QObject* qobj) { return PythonQt::self()->priv()->wrapQObject(qobj); }

        PyObject* entity_create(entity_id_t ent_id); //, Scene::EntityPtr entity);

        PyTypeObject *GetRexPyTypeObject();

        // Inventory skeleton retrieved during login process
        InventoryPtr inventory;

    private:
        //void SendObjectAddPacket(float start_x, start_y, start_z, float end_x, end_y, end_z);
        //! Type name of the module.
        static std::string type_name_static_;

        PythonEnginePtr engine_;
        bool pythonqt_inited;

        //basic feats
        void RunString(const char* codestr);
        void RunFile(const std::string &modulename);

        //void Reset();

        //a testing place
        void x();
        
        PyObject *apiModule; //the module made here that exposes the c++ side / api, 'rexviewer'

        // the hook to the python-written module manager that passes events on
        
        PyObject *pmmModule, *pmmDict, *pmmClass, *pmmInstance;
        PyObject *pmmArgs, *pmmValue;

        //Foundation::ScriptObject* modulemanager;
        
        // can't get passing __VA_ARGS__ to pass my args 
        //   in PythonScriptObject::CallMethod2
        //   so reverting to use the Py C API directly, not using the ScriptObject now
        //   for the modulemanager 
        
        bool mouse_left_button_down_;
        bool mouse_right_button_down_;

        // EventManager to member variable to be accessed from SubscribeNetworkEvents()
        Foundation::EventManagerPtr em_;
    };

    static PythonScriptModule *self() { return PythonScriptModule::GetInstance(); }
}

#endif
