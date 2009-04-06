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

        virtual void RunScript(const std::string& scriptname);
		virtual void RunString(const std::string& codestr);

		void Reset();

    private:
		/*
        boost::mutex renderer_;
        */

        //! scripting module we belong to - needed for logging?
        //PythonScriptModule* module_;
        
        //! framework we belong to
        Foundation::Framework* framework_;        
    };
}

#endif
