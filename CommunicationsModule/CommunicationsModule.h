// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationsModule_h
#define incl_CommunicationsModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include <QObject>

namespace Communications
{
    class ServiceInterface;
    typedef boost::shared_ptr<ServiceInterface> CommunicationsServicePtr; 
}

namespace CommunicationsService
{
    /**
     *  Communications Module
     *
     *  - Implementes and register interfaces/CommunicationsServiceInterface.
     *  - You must not use this module directly. All functionality must be used throug service interface
     *
     *  @todo Console commands: 'communications' .. Information about available services
     *  @todo Move Communications UI implementation here ???
     */
    class CommunicationsModule : public QObject, public IModule
    {
    public:
        CommunicationsModule();
        virtual ~CommunicationsModule();

        // Module API
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return type_name_static_; }

    private:
        void InitializeConsoleCommands();
        Console::CommandResult OnConsoleCommandCommunications(const StringVector &params);
        static std::string type_name_static_;
        Communications::CommunicationsServicePtr communications_service_;
    };

} // CommunicationsService

#endif // incl_CommunicationsModule_h
