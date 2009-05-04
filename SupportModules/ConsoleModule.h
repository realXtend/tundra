// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleModule_h
#define incl_ConsoleModule_h

#include "ModuleInterface.h"
#include "ConsoleModuleApi.h"

namespace Foundation
{
    class Framework;
}

/*! \defgroup DebugConsole_group Debug Console Client Interface
    \copydoc Console
*/

//! Provides services related to a debug console.
/*! See \ref DebugConsole "Using the debug console".
*/
namespace Console
{
    //! Debug console module
    /*! See \ref DebugConsole "Using the debug console".
    */
    class CONSOLE_MODULE_API ConsoleModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        ConsoleModule();
        virtual ~ConsoleModule();

        virtual void Load();
        virtual void Unload();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }

        //! Returns default console
        ConsolePtr GetConsole() const { return manager_; }


        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Console;

    private:
        //! debug console manager
        ConsolePtr manager_;
    };
}

#endif
