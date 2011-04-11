//$ HEADER_MOD_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.h
 *  @brief  Implements UiServiceInterface and provides means of adding widgets to the 
 *          in-world scene and managing different UI scenes.
 *          Basically this class is just a wrapper around InworldSceneController
 *          and UiStateMachine.
 */

#ifndef incl_UiModule_UiSceneService_h
#define incl_UiModule_UiSceneService_h 

#include "UiServiceInterface.h"
#include "Outworld/ExternalPanelManager.h"
#include "Outworld/ExternalMenuManager.h"
#include "Outworld/ExternalToolBarManager.h"
#include "AssetAPI.h"
#include "QtUiAsset.h"
//$ BEGIN_MOD $
#include <QMap>
#include <QList>
#include <QWidget>
#include <QDockWidget>
//$ END_MOD $

namespace UiServices
{
    class UiModule;
//$ BEGIN_MOD $
	//typedef QPair<UiProxyWidget*,QDockWidget*> proxyDock;
	typedef QPair<QString, QString> menusPair;
//$ END_MOD $

    /** Implements UiServiceInterface and provides means of adding widgets to the 
     *  in-world scene and managing different UI scenes.
     *  Basically this class is just a wrapper around InworldSceneController
     *  and UiStateMachine.
     */
    class UiSceneService : public UiServiceInterface
    {
        Q_OBJECT

    public:
        /** Constuctor.
         *  @param owner Owner module.
         */
        UiSceneService(UiModule *owner);

        /// Destructor.
        ~UiSceneService();

    public slots:

        /// UiServiceInterface override.
        UiProxyWidget *AddWidgetToScene(QWidget *widget, bool dockable = true, bool outside = false, Qt::WindowFlags flags = Qt::Dialog);

        /// UiServiceInterface override.
        bool AddWidgetToScene(UiProxyWidget *widget);

		bool AddProxyWidgetToScene(UiProxyWidget *proxy);

		/// UiServiceInterface override.
		bool AddInternalWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence);

        /// UiServiceInterface override.
        void AddWidgetToMenu(QWidget *widget, const QString &name, const QString &menu, const QString &icon);

        /// UiServiceInterface override.
        void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu, const QString &icon);

        /// UiServiceInterface override.
        void AddWidgetToMenu(QWidget *widget);

		/// UiServiceInterface override.
		bool AddExternalMenu(QMenu *new_menu, const QString &menu, const QString &icon = 0);

        /// UiServiceInterface override.
        void RemoveWidgetFromMenu(QWidget *widget);

        /// UiServiceInterface override.
        void RemoveWidgetFromMenu(QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void RemoveWidgetFromScene(QWidget *widget);

        /// UiServiceInterface override.
        void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void ShowWidget(QWidget *widget) const;

        void ShowWidget(const QString &name) const;

        /// UiServiceInterface override.
        void HideWidget(QWidget *widget) const;

        void HideWidget(const QString &name) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QWidget *widget) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

        /// UiServiceInterface override.
		QList<QString> GetAllWidgetsNames() const;

        /// UiServiceInterface override.
		QWidget* GetWidget(const QString &name) const;

        /// UiServiceInterface override.
		void SendToCreateDynamicWidget(const QString &name,const QString &module,const QVariantList properties);

        /// UiServiceInterface override.
        bool AddSettingsWidget(QWidget *widget, const QString &name) const;

        /// UiServiceInterface override.
        QGraphicsScene *GetScene(const QString &name) const;

		/// UiServiceInterface override.
		QGraphicsScene *GetMainScene() const ;

		/// UiServiceInterface override.
        void RegisterMainScene(const QString &name, QGraphicsScene *scene);

        /// UiServiceInterface override.
        void RegisterScene(const QString &name, QGraphicsScene *scene);

        /// UiServiceInterface override.
        bool UnregisterScene(const QString &name);

        /// UiServiceInterface override.
        bool SwitchToScene(const QString &name);

		/// UiServiceInterface override.
        bool SwitchToMainScene();

        /// UiServiceInterface override.
        void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget);

		/// UiServiceInterface override.
		void ShowNotification(int hide_in_msec, const QString &message);

        /// UiServiceInterface override.
        QWidget *LoadFromFile(const QString &file_path,  bool add_to_scene = true, QWidget *parent = 0);

		//! Save the position of widgets in a configuration file
		void SaveViewConfiguration();

//$ BEGIN_MOD $
		/// UiServiceInterface override.
		bool AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon = 0);
		/// UiServiceInterface override.
        void BringWidgetToFront(QString widget);
		/// UiServiceInterface override.
		//void TransferWidgetOut(QString widgetToChange, bool out);

		//TOOLBARS

		/// UiServiceInterface override.
		bool AddExternalToolbar(QToolBar *toolbar, const QString &name);

		/// UiServiceInterface override.
		bool RemoveExternalToolbar(QString name);

		/// UiServiceInterface override.
		bool ShowExternalToolbar(QString name);

		/// UiServiceInterface override.
		bool HideExternalToolbar(QString name);

		/// UiServiceInterface override.
		bool EnableExternalToolbar(QString name);

		/// UiServiceInterface override.
		bool DisableExternalToolbar(QString name);

		/// UiServiceInterface override.
		QToolBar* GetExternalToolbar(QString name);

		//TO MANAGE MENU SETTINGS
		/*! Method to create the settings panel and add it to scene
		*
		*/
		void CreateSettingsPanel();

		/*! Method to check if one menu is inside the scene or outside
		* \param name Name of the widget to be checked
		*/
		bool IsMenuInside(QString name);

		/*! Method to check if one menu is moveable or not
		* \param name Name of the widget to be checked
		*/
		bool IsMenuMoveable(QString name);

		/*! Method to give the settings panel the list of the panels available
			We only give the list when we switch from Ether scene to a new one.
		* \param old_name Name of the old scene
		* \param new_name Name of the new scene
		*
		* \note: this slot is conected to a signal
		*/

//$ END_MOD $
    private slots:
		/// UiServiceInterface override.
        void TranferWidgets();

    private:
        /// Owner UI module.
        UiModule *owner_;
//$ BEGIN_MOD $
		//! List of movable widget
		QMap<QString, menusPair> panels_menus_list_;

		//Main scene pointer
		QGraphicsScene *main_scene_;


		//New lists
		QMap<QString, QDockWidget*> external_dockeable_widgets_;
		QMap<QString, UiProxyWidget*> internal_widgets_;
		QMap<QString, QWidget*> external_nondockeable_widgets_;
		QMap<QString, QWidget*> dynamic_widgets_;
//$ END_MOD $
    };
}

#endif
