// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_h
#define incl_WorldBuildingModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "WorldBuildingModuleApi.h"
#include "InputServiceInterface.h"

#include <QObject>
#include <QStringList>
#include <QMap>

namespace WorldBuilding
{
    class BuildSceneManager;
    typedef boost::shared_ptr<BuildSceneManager> BuildServicePtr;

    class WB_MODULE_API WorldBuildingModule : public QObject, public Foundation::ModuleInterface
    {

    Q_OBJECT
    
    MODULE_LOGGING_FUNCTIONS
    static const std::string &NameStatic();

    public:
        WorldBuildingModule();
        virtual ~WorldBuildingModule();

        void Initialize();
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    public slots:
        //! Return the python handler
        QObject *GetPythonHandler();
    
    private slots:
        //! Populate service_category_identifiers_
        void SubscribeToEventCategories();
        //! Handle our key context input
        void KeyPressed(KeyEvent &key);       

    private:
        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! World building input context
        InputContextPtr input_context_;

        //! WorldBuilding service
        BuildServicePtr build_scene_manager_;
    };
}
#endif