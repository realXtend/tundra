// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_LegacyAvatarModule_h
#define incl_LegacyAvatarModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

namespace Foundation
{
    class Framework;
}

namespace LegacyAvatar
{
    //! Interface for modules
    class LegacyAvatarModule: public Foundation::ModuleInterfaceImpl
    {
    public:
        LegacyAvatarModule();
        virtual ~LegacyAvatarModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        
        virtual bool HandleEvent(
            event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);
            
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        //! callback for console command
        Console::CommandResult ConsoleStats(const StringVector &params);

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_LegacyAvatar;

    private:
        void operator=(const LegacyAvatarModule &);
        LegacyAvatarModule(const LegacyAvatarModule &);
               
        // Pointer to rexlogic interface
    };
}
#endif
