// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NoteCardModule_NoteCardModule_h
#define incl_NoteCardModule_NoteCardModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

namespace NoteCard
{
    class NoteCardManager;

    //! Notecard module
    /*! Displays a manager window which can be used to create/delete
     */
    class NoteCardModule : public Foundation::ModuleInterface
    {
    public:
        //! Constructor.
        NoteCardModule();

        //! Destructor 
        virtual ~NoteCardModule();

        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        
        void SubscribeToNetworkEvents();
        
        Console::CommandResult ShowWindow(const StringVector &params);
        
        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return name_static_; }
        
        MODULE_LOGGING_FUNCTIONS
        
    private:
        //! Notecard manager widget
        NoteCardManager* manager_;
        
        //! Static name of the module
        static std::string name_static_;
        
        //! Event manager pointer.
        Foundation::EventManagerPtr event_manager_;
        
        //! Id for Framework event category
        event_category_id_t framework_event_category_;
        
        //! Id for Scene event category
        event_category_id_t scene_event_category_;
        
        //! Id for NetworkState event category
        event_category_id_t network_state_event_category_;
        
        //! Id for Input event category
        event_category_id_t input_event_category_;
    };
}

#endif

