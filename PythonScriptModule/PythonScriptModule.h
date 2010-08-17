// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_PythonScriptModule_h
#define incl_PythonScriptModule_PythonScriptModule_h

#include "Core.h"
#include "Foundation.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QVariantMap>

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

class EC_OpenSimPrim;

class InputContext;
typedef boost::shared_ptr<InputContext> InputContextPtr;
class KeyEvent;
class MouseEvent;

namespace OgreRenderer
{
    class Renderer;
}

namespace Scene
{
    class SceneManager;
}

namespace Foundation
{
    class Framework;
}

namespace ProtocolUtilities
{
    class InventorySkeleton;
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}
namespace Player
{
    class PlayerServiceInterface;
}

typedef boost::shared_ptr<ProtocolUtilities::InventorySkeleton> InventoryPtr;

namespace PythonScript
{
    class PythonEngine;
    typedef boost::shared_ptr<PythonEngine> PythonEnginePtr;

    //! A scripting module using Python
    class MODULE_API PythonScriptModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

    public slots: //things for the py side to call.
        OgreRenderer::Renderer* GetRenderer() const;
        Scene::SceneManager* GetScene(const QString &name) const;
        void RunJavascriptString(const QString &codestr, const QVariantMap &context = QVariantMap());
        InputContext* GetInputContext() const { return input.get(); }
        Player::PlayerServiceInterface* GetPlayerService() const;

    public:
        PythonScriptModule();
        virtual ~PythonScriptModule();

        // Module interface overrides
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! callback for console command
        Console::CommandResult ConsoleRunString(const StringVector &params);
        Console::CommandResult ConsoleRunFile(const StringVector &params);
        Console::CommandResult ConsoleReset(const StringVector &params);

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        static void Add3DCanvasComponents(Scene::Entity *entity, QWidget *widget, const QList<uint> &submeshes, int refresh_rate);

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

        Scene::ScenePtr GetScenePtr() const { return framework_->GetDefaultWorldScene(); }
        PyObject* WrapQObject(QObject* qobj) const;

        PyObject* entity_create(entity_id_t ent_id); //, Scene::EntityPtr entity);

//        PyTypeObject *GetRexPyTypeObject();

        // Inventory skeleton retrieved during login process
        InventoryPtr inventory;

        /// World stream pointer.
        ProtocolUtilities::WorldStreamPtr worldstream;
    private slots:
        void HandleMouseEvent(MouseEvent *mouse);

    private:
        //void SendObjectAddPacket(float start_x, start_y, start_z, float end_x, end_y, end_z);

        //! Type name of the module.
        static std::string type_name_static_;

        //! Static instance of ourselves.
        static PythonScriptModule *pythonScriptModuleInstance_;

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

        /// The default input context for python code to access. This context operates below
        /// the Qt windowing priority.
        InputContextPtr input;
    };

    static PythonScriptModule *self() { return PythonScriptModule::GetInstance(); }
}

#endif
