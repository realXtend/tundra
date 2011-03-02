//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiModule_h
#define incl_UiModule_UiModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include "UiModuleApi.h"
#include "UiModuleFwd.h"
#include "UiTypes.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QStringList>
//$ BEGIN_MOD $
#include <QMainWindow>
#include <QString>
#include <QMenuBar>
#include <QAction>

//#include "MainWindow.h"
//$ END_MOD $

class KeyEvent;
class InputContext;

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace UiServices
{
    class UiSettingsService;
    class UiSceneService;
    class MessageNotification;
	class ExternalPanelManager;
	class ExternalMenuManager;
	class ExternalToolBarManager;
	class StaticToolBar;

    typedef boost::shared_ptr<UiSettingsService> UiSettingsPtr;
    typedef boost::shared_ptr<UiSceneService> UiSceneServicePtr;

    //! UiModule provides user interface services
    /*! For details about Inworld Widget Services read UiWidgetServices.h
     *  For details about Notification Services read UiNotificationServices.h
     *  Include above headers into your .cpp and UiServicesFwd.h to your .h files for easy access
     *  For the UI services provided for other, see @see UiSceneService
     */
    class UI_MODULE_API UiModule : public QObject, public IModule
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
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        /*************** UiModule Services ***************/

        //! InworldSceneController will give you a QObject derived class that will give you all
        //! the UI related services like adding your own QWidgets into the 2D scene 
        //! \return InworldSceneController The scene manager with scene services
        InworldSceneController *GetInworldSceneController() const { return inworld_scene_controller_; }
        NotificationManager *GetNotificationManager() const { return inworld_notification_manager_; }
        CoreUi::UiStateMachine *GetUiStateMachine() const { return ui_state_machine_; }

		//!Get Managers of the module
		ExternalMenuManager *GetExternalMenuManager() const { return external_menu_manager_; }
		ExternalPanelManager *GetExternalPanelManager() const { return external_panel_manager_; }
		ExternalToolBarManager *GetExternalToolBarManager() const { return external_toolbar_manager_;}
		bool HasBeenPostinitializaded() const { return postInitialize_; }

        //cjb Ether::Logic::EtherLoginNotifier *GetEtherLoginNotifier() const;

        //! Logging
        MODULE_LOGGING_FUNCTIONS;

        //! Returns name of this module.
        static const std::string &NameStatic() { return type_name_static_; }

		//$ BEGIN_MOD $
		//$ MOD_DESCRIPTION Inside this method is where we create the QMainWindow of the main Aplication $
		//QMainWindow *qtWin_; 
		//$END_MOD$

    private slots:
        void OnKeyPressed(KeyEvent *key);

        //cjb void OnSceneChanged(const QString &old_name, const QString &new_name);

        //! Takes UI screenshots of world/avatar with rendering service
       //cjb void TakeEtherScreenshots();

    private:
        //! Notify all ui module components of connected/disconnected state
        //! \param message Optional message, e.g. error message.
        void PublishConnectionState(ConnectionState connection_state, const QString &message = "");

        //! Get all the category id:s of categories in eventQueryCategories
        void SubscribeToEventCategories();

        //! Type name of this module.
        static std::string type_name_static_;

		//! External Managers of the Module
		ExternalMenuManager *external_menu_manager_;
		ExternalPanelManager *external_panel_manager_;
		ExternalToolBarManager *external_toolbar_manager_;

        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! Pointer to the QOgre UiView
        QGraphicsView *ui_view_;

        //! UiStateMachine pointer
        CoreUi::UiStateMachine *ui_state_machine_;

        //! InworldSceneController pointer
        InworldSceneController *inworld_scene_controller_;

        //! NotificationManager pointer
        NotificationManager *inworld_notification_manager_;

        //! MainWindow
		QMainWindow* qWin_;

		//! Static toolbar
		StaticToolBar* staticToolBar_;

        //! Current World Stream pointer
        boost::shared_ptr<ProtocolUtilities::WorldStream> current_world_stream_;

        //! Ui service.
        UiSceneServicePtr ui_scene_service_;

        //! Input context for Ether
        boost::shared_ptr<InputContext> input;

		bool postInitialize_;

        //! Welcome message to be sent when inworld scene is enabled
        //! Do NOT delete this on deconstructor or anywhere else for that matter!

		//$ BEGIN_MOD $
		//$ MOD_DESCRIPTION Inside this method is where we create the QMainWindow of the main Aplication $
		//void CreateAndConfigureMainWin();
		//$ END_MOD $
    };
}

#endif // incl_UiModule_UiModule_h
