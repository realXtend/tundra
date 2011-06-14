//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiModule_h
#define incl_UiModule_UiModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include "UiModuleApi.h"
#include "UiModuleFwd.h"
#include "UiTypes.h"
#include "UiWidget.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QStringList>
//$ BEGIN_MOD $
#include <QMainWindow>
#include <QString>
#include <QMenuBar>
#include <QAction>
//$ END_MOD $

class KeyEvent;
class InputContext;

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace CoreUi
{
    class ChangeThemeWidget;
}

namespace UiServices
{
	class ExternalPanelManager;
	class ViewManager;

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
	
        //! \return InworldSceneController The scene manager with scene services
        InworldSceneController *GetInworldSceneController() const { return inworld_scene_controller_; }
        QMap<UiWidget*, QDockWidget*> GetExternalWidgets() const { return external_widgets_; }


		//!Get Managers of the module
		ExternalPanelManager *GetExternalPanelManager() const { return external_panel_manager_; }
        CoreUi::SettingsWidget *GetSettingsPanel() const { return settings_widget_;}
		bool HasBeenPostinitializaded() const { return win_restored_; }
        bool HasBeenUninitializaded() const { return win_uninitialized_;}

        //! Logging
        MODULE_LOGGING_FUNCTIONS;

        //! Returns name of this module.
        static const std::string &NameStatic() { return type_name_static_; }
        CoreUi::ChangeThemeWidget *changetheme_widget_;

    public slots:
        
        void AddWidgetToWindow(UiWidget *widget);
        void RemoveWidgetFromScene(UiWidget *widget);
        void AddAnchoredWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence);
        void RemoveAnchoredWidgetFromScene(QWidget *widget);
        QWidget *GetThemeSettingsWidget(); 

    private slots:
        void OnKeyPressed(KeyEvent *key);
		
		//To restore it after postinitialize all modules
		void RestoreMainWindow();

    private:
        
        //! Get all the category id:s of categories in eventQueryCategories
        void SubscribeToEventCategories();

        //! Type name of this module.
        static std::string type_name_static_;

		//! External Managers of the Module
		ExternalPanelManager *external_panel_manager_;
        CoreUi::SettingsWidget *settings_widget_;

        //! Current query categories
        QStringList event_query_categories_;

        //! Current subscribed category events
        QMap<QString, event_category_id_t> service_category_identifiers_;

        //! Pointer to the QOgre UiView
        QGraphicsView *ui_view_;

        //! InworldSceneController pointer
        InworldSceneController *inworld_scene_controller_;

        //! MainWindow
		QMainWindow* qWin_;

		//! Manager of views
		ViewManager* viewManager_;

        QMap<UiWidget*, QDockWidget*> external_widgets_;

		bool win_restored_;
        bool win_uninitialized_;

    };

}

#endif // incl_UiModule_UiModule_h
