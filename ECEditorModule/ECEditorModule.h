// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorModule_h
#define incl_ECEditorModule_ECEditorModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

namespace ECEditor
{
    //! EC Editor module
    /*! EC Editor implements a way of adding arbitrary EC's to world entities, using (so far) xml-formatted data typed in RexFreeData
     */
    class ECEditorWindow;
    
    class ECEditorModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        //! Constructor.
        ECEditorModule();

        //! Destructor 
        virtual ~ECEditorModule();

        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        
        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return name_static_; }
        
        MODULE_LOGGING_FUNCTIONS
        
    private:
        //! Event manager pointer.
        Foundation::EventManagerPtr event_manager_;
        
        //! EC editor window
        ECEditorWindow* editor_window_;
        
        //! Static name of the module
        static std::string name_static_;
    };
}

#endif

