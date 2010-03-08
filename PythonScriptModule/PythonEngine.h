// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScript_Script_h
#define incl_PythonScript_Script_h

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
    class PythonEngine : public Foundation::ScriptServiceInterface //, public Foundation::ScriptEventInterface
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

        virtual void RunScript(const std::string& scriptname);
        virtual void RunString(const std::string& codestr);

        // generic script wrapper interface
        /*
        virtual Foundation::ScriptObject* LoadScript(const std::string& scriptname, std::string& error);
        virtual Foundation::ScriptObject* GetObject(const Foundation::ScriptObject& script, 
                                                    const std::string& objectname, 
                                                    std::string& error);
        */

        //void Reset();


    private:
        /*
        boost::mutex renderer_;
        */

        //! scripting module we belong to - needed for logging?
        //PythonScriptModule* module_;
        
        //! framework we belong to
        Foundation::Framework* framework_;        

        // map of methods to call
        //std::map<std::string, void(*)(char*)> methods_;
        //std::map<std::string, std::vector<void(*)(char*)>> methods_;
        //std::map<std::string, std::vector<void(*)(char*)>> methods_;
        //std::map<std::string, StdFunctionVectorPtr> methods_;
    };

}

#endif
