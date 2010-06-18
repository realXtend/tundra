// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldInputModule
#define incl_WorldInputModule

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "InputServiceInterface.h"

#include "WorldInputModuleApi.h"

namespace Input
{
    class WorldInputLogic;
    typedef boost::shared_ptr <WorldInputLogic> StateMachinePtr;

    /*! Input module for handling inplicit user interface with "The World", 
     * that is anything that is that isn't an explicity Qt-based graphical 
     * user interface.
     */
    /*! Low-level key codes are not sent as events. Instead it is expect that 
     * users will either poll the machine, or install custom bindings which 
     * fire from within certain states when certain inputs are observed.  
     *
     * See \ref Input_page "Handling input" for more information.  
     */
    class WorldInputModule : public Foundation::ModuleInterface
    {
        MODULE_LOGGING_FUNCTIONS

    public:
        WorldInputModule();
        virtual ~WorldInputModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();

        virtual void Update(f64 frametime);

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        //! returns true if key with specified keycode is currently held down. Internal use only!
        //WORLDINPUT_MODULE_API bool IsKeyDown(OIS::KeyCode keycode) const {}

        //! returns true if button with specified code is currently held down. Internal use only!
        //WORLDINPUT_MODULE_API bool IsButtonDown(OIS::MouseButtonID code) const {}

        //! Polls the current mouse state for both absolute and relative movement
        //WORLDINPUT_MODULE_API const Events::Movement &GetMouseMovement() const {}

    private:
        static std::string type_name_static_;

        //! Input handling state machine
        StateMachinePtr state_machine_;
    };
}
#endif
