// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_TundraLogicModule_h
#define incl_TundraLogicModule_TundraLogicModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

class TundraLogicModule : public Foundation::ModuleInterface
{
public:
    /// Default constructor.
    TundraLogicModule();

    /// Destructor.
    ~TundraLogicModule();

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

    /// Starts a server (console command)
    Console::CommandResult ConsoleStartServer(const StringVector &params);
    
    /// Stops a server (console command)
    Console::CommandResult ConsoleStopServer(const StringVector &params);
    
    /// Stops a server (console command)
    Console::CommandResult ConsoleConnect(const StringVector &params);
    
    /// Stops a server (console command)
    Console::CommandResult ConsoleDisconnect(const StringVector &params);
    
private:
    //! Type name of the module.
    static std::string type_name_static_;
};

#endif
