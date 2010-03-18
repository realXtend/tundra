// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiModule_h
#define incl_UiModule_UiModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "UiModuleApi.h"

#include <QPair>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QGraphicsView>

namespace RexLogic
{
    class OpenSimLoginHandler;
}

namespace Ether
{
    namespace Logic
    {
        class EtherLogic;
    }
}

namespace CoreUi
{
    class UiConsoleManager;
}

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
    class UiProxyWidget;
    class UiWidgetProperties;
    class UiStateMachine;
    class InworldSceneController;
    class NotificationManager;

    enum ConnectionState { Connected, Disconnected, Failed };

    /** UiModule hadles the shown ui scene, creates 
      * core modules ui and provides ui services to modules 
      */

    class UI_MODULE_API UiModule : public Foundation::ModuleInterfaceImpl
    {

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

        QObject *GetEtherLoginNotifier();

        QPair<QString, QString> GetScreenshotPaths();

        /*************** Logging ***************/

        MODULE_LOGGING_FUNCTIONS;
        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        //! Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_UiServices;

    private:
        //! Notify all ui module components of connected/disconnected state
        void PublishConnectionState(ConnectionState connection_state);

        //! Get all the category id:s of categories in eventQueryCategories
        void SubscribeToEventCategories();

        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! Pointer to the QOgre UiView
        QGraphicsView *ui_view_;

        //! UiConsoleManager pointer
        CoreUi::UiConsoleManager* ui_console_manager_;

        //! InworldSceneController pointer
        InworldSceneController *inworld_scene_controller_;

        //! NotificationManager pointer
        NotificationManager *inworld_notification_manager_;

        UiStateMachine *ui_state_machine_;

        //! Ether Logic
        Ether::Logic::EtherLogic *ether_logic_;

        //! Current World Stream pointer
        boost::shared_ptr<ProtocolUtilities::WorldStream> current_world_stream_;

    };
}

#endif // incl_UiModule_UiModule_h
