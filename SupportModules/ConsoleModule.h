// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SupportModules_ConsoleModule_h
#define incl_SupportModules_ConsoleModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleModuleApi.h"

/*! \defgroup DebugConsole_group Debug Console Client Interface
    \copydoc Console
*/

//! Provides services related to a debug console.
/*! See \ref DebugConsole "Using the debug console".
*/
namespace Console
{
    class UiConsoleManager;

    //! Debug console module
    /*! See \ref DebugConsole "Using the debug console".
    */
    class CONSOLE_MODULE_API ConsoleModule : public IModule
    {
    public:
        ConsoleModule();
        virtual ~ConsoleModule();

        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Returns default console
        ConsolePtr GetConsole() const { return manager_; }

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

    private:
        //! Type name of the module.
        static std::string type_name_static_;

        //! debug console manager
        ConsolePtr manager_;

        //! Console event category id.
        event_category_id_t consoleEventCategory_;

        //! Input event category id.
        event_category_id_t inputEventCategory_;

        //! Console UI manager
        UiConsoleManager *ui_console_manager_;
    };
}

#endif
