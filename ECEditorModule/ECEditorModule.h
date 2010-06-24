// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorModule_h
#define incl_ECEditorModule_ECEditorModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>

namespace ECEditor
{
    //! EC Editor module
    /*! \defgroup ECEditorModuleClient ECEditorModule Client interface.
     *  EC Editor implements a way of adding arbitrary EC's to world entities, using (so far) xml-formatted data typed in RexFreeData
     */
    class ECEditorWindow;

    class ECEditorModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

    public:
        //! Constructor.
        ECEditorModule();

        //! Destructor 
        virtual ~ECEditorModule();

        //! ModuleInterfaceImpl overrides.
        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Show EC editor window.
        Console::CommandResult ShowWindow(const StringVector &params);

        //! For test use, remove this after dynamic component has fully tested.
        //Console::CommandResult AddDynamicComponent(const StringVector &params);

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return name_static_; }

        //! Logging functions.
        MODULE_LOGGING_FUNCTIONS

    public slots:
        void AddEditorWindowToUI();

        //! Creates EC attribute XML editor widget for entity.
        //! \param entity Entity pointer.
        void CreateXmlEditor(Scene::EntityPtr entity);

        //! Creates EC attribute XML editor widget for component.
        //! \param component Component pointer.
        void CreateXmlEditor(Foundation::ComponentPtr component);

    private:
        //! EC editor window
        ECEditorWindow* editor_window_;

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

