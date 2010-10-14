//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiExternalModule_UiExternalModule_h
#define incl_UiExternalModule_UiExternalModule_h

#include "IModule.h"

#include "UiExternalModuleApi.h"
#include "ExternalPanelManager.h"
#include "ExternalMenuManager.h"
#include "UiExternalService.h"
#include "ModuleLoggingFunctions.h"
#include "UiExternalServiceInterface.h"
#include "ModuleLoggingFunctions.h"
#include "StaticToolBar.h"

#include "UiServiceInterface.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QMainWindow>

namespace Foundation
{
    class Framework;
	class UiServiceInterface;
}

namespace UiExternalServices
{
	class ExternalPanelManager;
	class ExternalMenuManager;
	class UiExternalService;

	typedef boost::shared_ptr<UiExternalService> UiExternalServicePtr;
    
    //! UiExternalModule provides external user interface services
    /*! For details about External Widget Services read UiExternalService.h
     *  You can add new Widgets, Menus, Actions, etc
     */
    class UIEXTERNAL_MODULE_API UiExternalModule : public QObject, public IModule
    {
        Q_OBJECT

    public:

        UiExternalModule();
        virtual ~UiExternalModule();

        /*************** ModuleInterfaceImpl ***************/
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Returns name of this module.
		static const std::string &NameStatic() { return type_name_static_; };

		//!Get Managers of the module
		ExternalMenuManager* GetExternalMenuManager() { return menu_manager_; };
		ExternalPanelManager* GetExternalPanelManager() { return panel_manager_; };

		//! Logging
        MODULE_LOGGING_FUNCTIONS;

		//! External Ui Service.
		UiExternalServicePtr ui_external_scene_service_;

		//!MainWindow
		QMainWindow* qWin_;

		//!Managers of the Module
		ExternalMenuManager* menu_manager_;
		ExternalPanelManager* panel_manager_;

	private slots:
		//!Slots used to implement static menus
		void SwitchToEtherScene();
		void ExitApp();

    private:
		//! Type name of this module.
        static std::string type_name_static_;
		//! Method to configure static content of the MainWindow
		void createStaticContent();
		//!Static toolbar
		StaticToolBar* staticToolBar_;
		event_category_id_t scene_event_category_;
    };
}

#endif
