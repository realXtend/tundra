// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraServerModule_TundraServerModule_h
#define incl_TundraServerModule_TundraServerModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

class TundraServerModule : public Foundation::ModuleInterface
{
public:
    /// Default constructor.
    TundraServerModule();

    /// Destructor.
    ~TundraServerModule();

    /// ModuleInterface override.
    void PreInitialize();

    /// ModuleInterface override.
    void Initialize();

    /// ModuleInterface override.
    void PostInitialize();

    /// ModuleInterface override.
    void Uninitialize();

    /// ModuleInterface override.
    void Update(f64 frametime);

    /// ModuleInterface override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

private:
    //! Type name of the module.
    static std::string type_name_static_;
};

#endif
