// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_PythonEngine_h
#define incl_PythonScriptModule_PythonEngine_h

#include "ScriptServiceInterface.h"

namespace Foundation
{
    class Framework;
}

/*
namespace Python
{
    class Root;
    class SceneManager;
    class Camera;
    class RenderWindow;
}
*/

namespace PythonScript
{
    //typedef boost::shared_ptr<PythonEngine> PythonEnginePtr;
    typedef boost::shared_ptr<std::vector<void(*)(char*)> > StdFunctionVectorPtr;

    class PythonScriptModule;
    
    //! Python code runner
    //! created by PythonScriptModule.
    class PythonEngine : public Foundation::ScriptServiceInterface
    {
    public:
        PythonEngine(Foundation::Framework* framework);
        virtual ~PythonEngine();

        //! initializes the interpreter / engine
        void Initialize();
        void Uninitialize();

        /*deprecated - ScriptEventInterface
        virtual void SetCallback(void(*f)(char*), std::string key);
        virtual void NotifyScriptEvent(const std::string& key, const std::string& message);
        */

        virtual void RunScript(const QString &scriptname);
        //virtual void RunString(QString codestr, QMap<QString, QObject*> context = QMap<QString, QObject*>());
        virtual void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

        // generic script wrapper interface
        /*
        virtual Foundation::ScriptObject* LoadScript(const std::string& scriptname, std::string& error);
        virtual Foundation::ScriptObject* GetObject(const Foundation::ScriptObject& script, 
                                                    const std::string& objectname, 
                                                    std::string& error);
        */

        //void Reset();


    private:
        //! scripting module we belong to - needed for logging?
        //PythonScriptModule* module_;
        
        //! framework we belong to
        Foundation::Framework* framework_;        
    };

}

#endif
