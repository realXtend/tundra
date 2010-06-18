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
    class Mesh;

    //! Interface for modules
    class LegacyAvatarModule: public Foundation::ModuleInterface
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
        static const std::string &NameStatic() { return type_name_static_; }

        //! callback for console command
        Console::CommandResult ConsoleStats(const StringVector &params);

    private:
        void operator=(const LegacyAvatarModule &);
        LegacyAvatarModule(const LegacyAvatarModule &);

        //! Type name of the module.
        static std::string type_name_static_;

        static Mesh *mesh_eye;
        static Mesh *mesh_eyelashes;
        static Mesh *mesh_hair;
        static Mesh *mesh_head;
        static Mesh *mesh_lower_body;
        static Mesh *mesh_skirt;
        static Mesh *mesh_upper_body;
    };
}
#endif
