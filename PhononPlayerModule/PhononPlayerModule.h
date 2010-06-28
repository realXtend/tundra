// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PhononPlayerModule_h
#define incl_PhononPlayerModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include <QObject>

namespace Player
{
    class PlayerServiceInterface;
    typedef boost::shared_ptr<PlayerServiceInterface> PlayerServicePtr; 
}

namespace PlayerService
{
    /**
     *  Offers PlayerService using Phonon backend.
     */
    class PhononPlayerModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT
    public:
        PhononPlayerModule();
        virtual ~PhononPlayerModule();

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
        static std::string type_name_static_;
        
        Player::PlayerServicePtr player_service_;
    };

} // PlayerService

#endif // incl_PhononPlayerModule_h
