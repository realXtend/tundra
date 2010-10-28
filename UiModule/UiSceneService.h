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
//$ BEGIN_MOD $
#include "UiExternalServiceInterface.h"
#include <QMap>
#include <QList>
//$ END_MOD $

namespace UiServices
{
    class UiModule;
//$ BEGIN_MOD $
	typedef QPair<UiProxyWidget*,QDockWidget*> proxyDock;
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
        UiProxyWidget *AddWidgetToScene(QWidget *widget, bool moveable = true, bool outside = false, Qt::WindowFlags flags = Qt::Dialog);

        /// UiServiceInterface override.
        bool AddWidgetToScene(UiProxyWidget *widget);

        /// UiServiceInterface override.
        void AddWidgetToMenu(QWidget *widget, const QString &name, const QString &menu, const QString &icon);

        /// UiServiceInterface override.
        void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu, const QString &icon);

        /// UiServiceInterface override.
        void AddWidgetToMenu(QWidget *widget);

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

        /// UiServiceInterface override.
        void HideWidget(QWidget *widget) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QWidget *widget) const;

        /// UiServiceInterface override.
        void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

        /// UiServiceInterface override.
        bool AddSettingsWidget(QWidget *widget, const QString &name) const;

        /// UiServiceInterface override.
        QGraphicsScene *GetScene(const QString &name) const;

        /// UiServiceInterface override.
        void RegisterScene(const QString &name, QGraphicsScene *scene);

        /// UiServiceInterface override.
        bool UnregisterScene(const QString &name);

        /// UiServiceInterface override.
        bool SwitchToScene(const QString &name);

        /// UiServiceInterface override.
        void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget);

        /// UiServiceInterface override.
        void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget);

        /// UiServiceInterface override.
        QWidget *LoadFromFile(const QString &file_path,  bool add_to_scene = true, QWidget *parent = 0);
//$ BEGIN_MOD $
		bool TransferWidgetInOut(QString widgetToChange);
		/// UiServiceInterface override.
        void BringWidgetToFront(QString widget);
		/// UiServiceInterface override.
		void TransferWidgetOut(QString widgetToChange, bool out);
//$ END_MOD $
    private slots:
		/// UiServiceInterface override.
        void TranferWidgets();
//$ BEGIN_MOD $
		/*! Transfer to the scene the widgets that WorldBuild uses when the current scene is WorldBuilding.
		 *	This slot is connected with the SceneChanged signal
		 *
		 *	old_name name of the old current scene
		 *	new_name name of the new current scene
		*/
		void HandleTransferToBuild(const QString& old_name, const QString& new_name);
//$ END_MOD $
    private:
        /// Owner UI module.
        UiModule *owner_;
//$ BEGIN_MOD $
		//! UiExternal Service
		Foundation::UiExternalServiceInterface *uiExternal;
		//! List of movable widget
		QList<QString> *moveable_widgets_;
		//! QMap of widgets pairs
		QMap<QString, proxyDock> proxy_dock_list;
		//! QMap of menu pairs 
		QMap<QString, menusPair> panels_menus_list_;
//$ END_MOD $
    };
}

#endif
