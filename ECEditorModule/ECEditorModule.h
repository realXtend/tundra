// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorModule_h
#define incl_ECEditorModule_ECEditorModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QPointer>

namespace ECEditor
{
    class ECEditorWindow;
    class EcXmlEditorWidget;

    //! EC Editor module
    /*! \defgroup ECEditorModuleClient ECEditorModule Client interface.
     *  EC Editor implements a way of adding arbitrary EC's to world entities, using (so far) xml-formatted data typed in RexFreeData
     */
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

        //! Added for testing EC_DynamicComponent.
        /*! @param params Params should be following:
         *  0 = entity id.
         *  1 = operation (add or rem)
         *  2 = component type.(ec. EC_DynamicComponent)
         *  3 = attribute name.
         *  4 = attribute type. !Only rem dont use in rem operation.
         *  5 = attribute value. !Only rem dont use in rem operation.
         */
        Console::CommandResult EditDynamicComponent(const StringVector &params);

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return name_static_; }

        //! Logging functions.
        MODULE_LOGGING_FUNCTIONS

    public slots:
        void AddEditorWindowToUI();

        //! Creates EC attribute XML editor widget for entity.
        //! \param entity Entity pointer.
        void CreateXmlEditor(Scene::EntityPtr entity);

        //! Creates EC attribute XML editor widget for entity.
        //! \param entities List of entity pointers.
        void CreateXmlEditor(const QList<Scene::EntityPtr> &entities);

        //! Creates EC attribute XML editor widget for component.
        //! \param component Component pointer.
        void CreateXmlEditor(Foundation::ComponentPtr component);

        //! Creates EC attribute XML editor widget for component.
        //! \param components List of component pointers.
        void CreateXmlEditor(const QList<Foundation::ComponentPtr> &components);

    private:
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

        //! EC editor window
        ECEditorWindow* editor_window_;

        //! EC XML editor window
        QPointer<EcXmlEditorWidget> xmlEditor_;
    };
}

#endif

