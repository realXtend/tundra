// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiModule_h
#define incl_UiModule_UiModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include "UiModuleApi.h"
#include "UiModuleFwd.h"
#include "UiDefines.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QStringList>

class KeyEvent;
class InputContext;

namespace OgreRenderer
{
    class QOgreUIView;
}

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace UiServices
{
    class UiSettingsService;
    class UiSceneService;
    typedef boost::shared_ptr<UiSettingsService> UiSettingsPtr;
    typedef boost::shared_ptr<UiSceneService> UiSceneServicePtr;

    //! UiModule provides user interface services
    /*! For details about Inworld Widget Services read UiWidgetServices.h
     *  For details about Notification Services read UiNotificationServices.h
     *  Include above headers into your .cpp and UiServicesFwd.h to your .h files for easy access
     *  For the UI services provided for other, see @see UiSceneService
     */
    class UI_MODULE_API UiModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

    public:
        UiModule();
        ~UiModule();

        /*************** ModuleInterfaceImpl ***************/
        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        /*************** UiModule Services ***************/

        //! InworldSceneController will give you a QObject derived class that will give you all
        //! the UI related services like adding your own QWidgets into the 2D scene 
        //! \return InworldSceneController The scene manager with scene services
        InworldSceneController *GetInworldSceneController() const { return inworld_scene_controller_; }

        NotificationManager *GetNotificationManager() const { return inworld_notification_manager_; }

        CoreUi::UiStateMachine *GetUiStateMachine() const { return ui_state_machine_; }

        QObject *GetEtherLoginNotifier() const;

        QPair<QString, QString> GetScreenshotPaths();

        //! Logging
        MODULE_LOGGING_FUNCTIONS;

        //! Returns name of this module.
        static const std::string &NameStatic() { return type_name_static_; }

    private slots:
        void OnKeyPressed(KeyEvent &key);

    private:
        //! Notify all ui module components of connected/disconnected state
        void PublishConnectionState(UiDefines::ConnectionState connection_state);

        //! Get all the category id:s of categories in eventQueryCategories
        void SubscribeToEventCategories();

        //! Type name of this module.
        static std::string type_name_static_;

        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! Pointer to the QOgre UiView
        QGraphicsView *ui_view_;

        //! UiConsoleManager pointer
        CoreUi::UiConsoleManager* ui_console_manager_;

        //! UiStateMachine pointer
        CoreUi::UiStateMachine *ui_state_machine_;

        //! Service getter, provides functions to access Naali services
        CoreUi::ServiceGetter *service_getter_;

        //! InworldSceneController pointer
        InworldSceneController *inworld_scene_controller_;

        //! NotificationManager pointer
        NotificationManager *inworld_notification_manager_;

        //! Ether Logic
        Ether::Logic::EtherLogic *ether_logic_;

        //! Current World Stream pointer
        boost::shared_ptr<ProtocolUtilities::WorldStream> current_world_stream_;

        //! Ui settings service 
        UiSettingsPtr ui_settings_service_;

        //! Ui service.
        UiSceneServicePtr ui_scene_service_;

        boost::shared_ptr<InputContext> input;
    };
}

#endif // incl_UiModule_UiModule_h
